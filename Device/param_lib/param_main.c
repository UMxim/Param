#include "param_main.h"
#include "param_cfg.h"
#include <string.h>

#define PARAM_LIST_NUM 		( sizeof(paramsDef) / sizeof(params_t) )
#define STAT_LIST_NUM 		( sizeof(statisticDef) / sizeof(params_t) )
#define BUFF_MAX_SIZE 		(sizeof(packet_header_t) + 8 + PARAM_MAX_STR) // Len Type CS NU Data. Data расчитана для макимального ответа
#define CHECKSUMM_INIT  	0x96C30FA5UL
#define FIND_CONSTANT		0xDEADBEEF

typedef struct
{
	uint8_t dataLen;
	uint8_t type;
	uint8_t checksumm;
	uint8_t reserved;
} packet_header_t;

static const struct
{
	uint32_t findConst;	// константа для поиска параметров в прошивке внешней программой
	uint8_t type;
	uint8_t hw_ver;
	uint8_t sw_ver;
	uint8_t max_data_size;
	uint8_t name[];
} info = {
			FIND_CONSTANT,
			HW_TYPE,
			HW_VER,
			SW_VER,
			BUFF_MAX_SIZE - sizeof(packet_header_t),
			DEVICE_STR
		  };

static struct
{
	uint8_t *ptr;
	uint16_t len;
} dump;

static const params_t paramsDef[] =
    {
	PARAM_PARAM_ARR
    }; // параметры по умолчанию на флеше

static const params_t statisticDef[] =
    {
	PARAM_STAT_ARR
    }; // статистика по умолчанию на флеше

static struct
{
	uint32_t params[PARAM_LIST_NUM];
	uint32_t statistic[STAT_LIST_NUM];
	uint32_t cs;
} ps;

//uint32_t param_statistic_cs_arr[PARAM_LIST_NUM + STAT_LIST_NUM + 1]; // чтобы вместе лежали, как на флеше

//uint32_t *const params = param_statistic_cs_arr; // параметры текущие
//uint32_t *const statistic = param_statistic_cs_arr + PARAM_LIST_NUM; // статистика текущая
//uint32_t * const cs = param_statistic_cs_arr + PARAM_LIST_NUM + STAT_LIST_NUM;


static uint8_t buff[BUFF_MAX_SIZE] __attribute__ ((aligned (4)));

static packet_header_t * const header = (packet_header_t *)buff;
static uint8_t * const data = buff + sizeof(packet_header_t);

static volatile struct
{
	uint32_t ns;
	uint32_t s;
	int32_t reset; // обратный таймер ресета в ns
} timer = {0, 0, INT32_MAX};

typedef enum
{
	ERROR_OK = 			0,
	ERROR_CHECKSUMM =	1,
	ERROR_TYPE =		2,
	ERROR_DATA =		3,
	ERROR_LEN =			4,
	ERROR_HW =			5
} error_t;

typedef enum
{
	TYPE_GET_INFO	= 		0,
	TYPE_GET_STATISTIC = 	1,
	TYPE_RESET_STATISTIC =	2,
	TYPE_GET_PARAM = 		3,
	TYPE_WRITE_PARAM =		4,
	TYPE_GET_DUMP =			6,
	TYPE_RESET =			7,
	TYPE_WRITE_FW = 		8,
	TYPE_ERROR = 			0x7F
} type_t;

static uint8_t _CalcCS()
{
	uint16_t len = sizeof(packet_header_t) + header->dataLen;
	uint8_t cs = 0;
	for (int i=0; i<len; i++)
		cs^=buff[i];
	return cs;
}

static uint16_t _CopyStr(const char *str, uint8_t data_pos) // указатель на исходную строку, позиция в буфере данных для записи
{
	uint16_t len = 0;
	while (str)
	{
		data[data_pos++] = *(str++);
		len++;
		if (len == PARAM_MAX_STR) break;
	}
	return len;
}

// Сбрасываем на флеш при необходимости
void _Flush()
{
	uint32_t *flashData = Param_HAL_GetFlashDataAddr();
	int res = memcmp(flashData, &ps, sizeof(ps) );
	if (!res) return; // не изменилось
	ps.cs = CHECKSUMM_INIT;
	for (int i=0; i<=PARAM_LIST_NUM + STAT_LIST_NUM; i++)
	{
		ps.cs ^= ((uint32_t*)&ps)[i];
	}
	Param_HAL_WriteFlashData((uint32_t*)&ps, PARAM_LIST_NUM + STAT_LIST_NUM + 1);
}

static inline void _Get_info()
{
	header->type = TYPE_GET_INFO;
	memcpy(data, &info, 8); // первые значения.
	header->dataLen = 8 + _CopyStr(info.name, 8);
}

static inline error_t _Get_statistic()
{
	uint8_t n = data[0];
	if (n >= STAT_LIST_NUM)
		return ERROR_DATA;

	header->type = TYPE_GET_STATISTIC;
	memcpy(data, ps.statistic + n, sizeof(ps.statistic[0]));
	//memcpy(data+sizeof(ps.statistic[0]), statisticDef[n].text, PARAM_MAX_STR);
	header->dataLen = sizeof(ps.statistic[0]) + _CopyStr(statisticDef[n].text, 4) ; // 4 + str
	return ERROR_OK;
}

static inline void _Reset_statistic()
{
	for (int i=0; i<STAT_LIST_NUM; i++)
	{
		ps.statistic[i] = statisticDef[i].value;
	}
}

static inline error_t _Get_param()
{
	uint8_t n = data[0];
	if (n >= PARAM_LIST_NUM)
		return ERROR_DATA;

	header->type = TYPE_GET_PARAM;
	memcpy(data, &ps.params[n], sizeof(ps.params[0]));	//4
	memcpy(data + sizeof(ps.params[0]), &paramsDef[n], sizeof(paramsDef[0])); //4
	//memcpy(data + 2 * sizeof(ps.params[0]), paramsDef[n].text, PARAM_MAX_STR);
	header->dataLen = 2 * sizeof(ps.params[0]) + _CopyStr(paramsDef[n].text, 8); // 8 + 16
	return ERROR_OK;
}

static inline error_t _Write_param()
{
	uint8_t n = data[0];
	if (n >= PARAM_LIST_NUM)
		return ERROR_DATA;

	uint32_t newParam;
	memcpy(&newParam, data + 1, 4);
	ps.params[n] = newParam;

	header->type = TYPE_WRITE_PARAM;
	header->dataLen = 0;

	return ERROR_OK;
}

static inline void _Get_dump()
{
	uint16_t offset = data[0] + (data[1]<<8);
	header->type = TYPE_GET_DUMP;
	if (offset >= dump.len || !dump.ptr)
	{
		header->dataLen = 0;
		return;
	}
	header->dataLen = ((dump.len - offset) > info.max_data_size) ? info.max_data_size : (dump.len - offset);
	memcpy(data, dump.ptr + offset, header->dataLen);
}

static inline void _Reset()
{
	_Flush();
	header->type = TYPE_RESET;
	header->dataLen = 0;
	timer.reset = 500000000; // 500ms
}

static inline void _Write_FW()
{
	_Flush();
	header->type = TYPE_WRITE_FW;
	header->dataLen = 0;
	Param_HAL_SetBootCfg(); // настраиваются только биты boot / uart не изменяется
	timer.reset = 500000000; // 500ms
}

static void SendError(error_t errN)
{
	header->dataLen = 1; // len type err cs
	header->type = TYPE_ERROR;
	*data = errN;
	header->checksumm = 0;
	header->checksumm = _CalcCS();
	//Param_HAL_Transmit(buff, 4);
}

static void _ParseBuff(void)
{
	// check
	uint8_t cs = _CalcCS();
	if (cs)
	{
		SendError(ERROR_CHECKSUMM);
		goto transmit;
	}

	error_t ans = ERROR_OK;
	switch (header->type)
	{

	case TYPE_GET_INFO | 0x80:
		_Get_info();
		break;
	case TYPE_GET_STATISTIC | 0x80:
		ans = _Get_statistic();
		break;
	case TYPE_RESET_STATISTIC | 0x80:
		_Reset_statistic();
		break;
	case TYPE_GET_PARAM | 0x80:
		ans = _Get_param();
		break;
	case TYPE_WRITE_PARAM | 0x80:
		ans = _Write_param();
		break;
	case TYPE_GET_DUMP | 0x80:
		_Get_dump();
		break;
	case TYPE_RESET | 0x80:
		_Reset();
		break;
	case TYPE_WRITE_FW | 0x80:
		_Write_FW();
		break;
	default:
		ans = ERROR_TYPE;
		}

	if (ans != ERROR_OK)
	{
		SendError(ans);
		goto transmit;
	}

	header->checksumm = 0;
	header->checksumm = _CalcCS();

	transmit:
	Param_HAL_Transmit(buff, sizeof(packet_header_t) + header->dataLen);
}


void Param_Init(void * dump_in, uint16_t dump_len, int dump_size)
{
	Param_HAL_Init(Param_RxCallback);
	// read params
	uint32_t *dataFlash = Param_HAL_GetFlashDataAddr();
	dump.ptr = dump_in;
	dump.len = dump_len;
	uint32_t cs = CHECKSUMM_INIT;
	for (int i=0; i<=sizeof(ps); i++)
	{
		cs ^= dataFlash[i];
	}

	for (int i=0; i<PARAM_LIST_NUM; i++)
	{
		ps.params[i] = (cs) ? paramsDef[i].value : dataFlash[i];
	}

	for (int i=0; i<STAT_LIST_NUM; i++)
	{
		ps.statistic[i] = (cs) ? statisticDef[i].value : dataFlash[PARAM_LIST_NUM + i];
	}
}

void _Param_Cycle()
{

}

// Вызываем в прерывании при получении байта
void Param_RxCallback(uint8_t byte)
{
	static uint16_t pos = 0;	// позиция в массиве для текущего байта
	buff[pos] = byte;
	pos++;

	if (pos == sizeof(packet_header_t) + header->dataLen)
	{
		pos = 0;
		_ParseBuff();
	}

	return;
}

// Вызывать в прерывании таймера. Квант времени описать в дефайне
void Param_Timer_Callback(void)
{
	timer.ns += PARAM_EXT_TIMER_PERIOD_NS;
	if (timer.reset != INT32_MAX)
	{
		timer.reset -= PARAM_EXT_TIMER_PERIOD_NS;
		if (timer.reset < 0)
			Param_HAL_Reset();
	}

	if ( timer.ns >= 1000000000 )
	{
		timer.ns -= 1000000000;
		timer.s ++;
		_Param_Cycle();
	}
}

