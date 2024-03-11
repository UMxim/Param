#include "param_main.h"
#include "param_cfg.h"
#include <string.h>

#define PARAM_LIST_NUM 		( sizeof(paramsDef) / sizeof(params_t) )
#define STAT_LIST_NUM 		( sizeof(statisticDef) / sizeof(params_t) )
#define BUFF_MAX_SIZE 		(4 + 8 + PARAM_MAX_STR) // Len Type CS NU Data. Data расчитана для макимального ответа
#define CHECKSUMM_INIT  	0x96C30FA5UL
#define FIND_CONSTANT		0xDEADBEEF
#define HEADER_LEN			4 // размер заголовка пакета

typedef struct
{
	uint8_t dataLen;
	uint8_t type;
	uint8_t checksumm;
	uint8_t reserved;
	uint8_t dataFB; // data First Byte - для взятия адреса начала данных
} packet_header_t;

static const struct
{
	uint32_t findConst;	// константа для поиска параметров в прошивке внешней программой
	uint8_t type;
	uint8_t hw_ver;
	uint8_t sw_ver;
	uint8_t na;
	uint8_t name[];
} info = {
			FIND_CONSTANT,
			HW_TYPE,
			HW_VER,
			SW_VER,
			0,
			DEVICE_STR
		  };

void Param_RecieveByte_Callback(uint8_t byte);

static const params_t paramsDef[] =
    {
	PARAM_PARAM_ARR
    }; // параметры по умолчанию на флеше

static const params_t statisticDef[] =
    {
	PARAM_STAT_ARR
    }; // статистика по умолчанию на флеше

uint32_t params[PARAM_LIST_NUM]; // параметры текущие

uint32_t statistic[STAT_LIST_NUM]; // статистика текущая

static uint8_t buff[BUFF_MAX_SIZE] __attribute__ ((aligned (4)));

static packet_header_t * const header = (packet_header_t *)buff;
static uint8_t * const data = &header->dataFB;

static volatile uint64_t timer_ns = 0;

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
	uint16_t len = HEADER_LEN + header->dataLen;
	uint8_t cs = 0;
	for (int i=0; i<len; i++)
		cs^=buff[i];
	return cs;
}

static inline void _Get_info()
{
	header->type = TYPE_GET_INFO;
	header->dataLen = 8 + PARAM_MAX_STR;
	memcpy(data, &info, header->dataLen); // первые значения. Для дисплея берем только 16 букв
}

static inline error_t Get_statistic()
{
	uint8_t n = data[0];
	if (n >= STAT_LIST_NUM - 1)
		return ERROR_DATA;

	header->type = TYPE_GET_STATISTIC;
	memcpy(data, &statistic[n], 4);
	memcpy(data+sizeof(statistic[0]), statisticDef[n].text, PARAM_MAX_STR);
	header->dataLen = 4 + PARAM_MAX_STR;
	return ERROR_OK;
}

static inline void Reset_statistic()
{

}

static inline void Get_param()
{

}

static inline void Write_param()
{

}

static inline void Get_dump()
{

}

static inline void Reset()
{

}

static inline void Write_FW()
{

}

static void SendError(error_t errN)
{
	buff[0] = 1; // len type err cs
	buff[1] = TYPE_ERROR;
	buff[2] = errN;
	_CalcCS();
	Param_HAL_Transmit(buff, 4);
}

static void _ParseBuff(void)
{
	// check
	uint8_t cs = _CalcCS();
	if (cs)
		return SendError(ERROR_CHECKSUMM);

	error_t ans = ERROR_OK;
	switch (header->type)
	{

	case TYPE_GET_INFO | 0x80:
		_Get_info();
		break;

	case TYPE_GET_STATISTIC | 0x80:
		ans = Get_statistic(1);
		break;
	case TYPE_RESET_STATISTIC | 0x80:
		break;
	case TYPE_GET_PARAM | 0x80:
		break;
	case TYPE_WRITE_PARAM | 0x80:
		break;
	case TYPE_GET_DUMP | 0x80:
		break;
	case TYPE_RESET | 0x80:
		break;
	case TYPE_WRITE_FW | 0x80:
		break;

	default:
		ans = ERROR_TYPE;
		}

	if (ans != ERROR_OK)
		SendError(ans);
	header->checksumm = 0;
	header->checksumm = _CalcCS();

	Param_HAL_Transmit(buff, 2 + buff[0] + 1);
}


void Param_Init(void *dump, int dump_size)
{
	Param_HAL_Init(Param_RecieveByte_Callback, Param_Timer_Callback);
	// read params
	uint16_t dataSize_Word = PARAM_LIST_NUM + STAT_LIST_NUM + 1;
	uint32_t *dataFlash = buff;//Param_HAL_GetFlashDataAddr();

	uint32_t cs = CHECKSUMM_INIT;
	for (int i=0; i<=dataSize_Word; i++)
	{
		cs ^= dataFlash[i];
	}

	for (int i=0; i<PARAM_LIST_NUM; i++)
	{
		params[i] = (cs) ? paramsDef[i].value : dataFlash[i];
	}

	for (int i=0; i<STAT_LIST_NUM; i++)
	{
		statistic[i] = (cs) ? statisticDef[i].value : dataFlash[i+PARAM_LIST_NUM];
	}
}

// Вызываем в прерывании при получении байта
void Param_RecieveByte_Callback(uint8_t byte)
{
	static uint16_t pos = 0;	// позиция в массиве для текущего байта

	buff[pos] = byte;
	pos++;

	/////
	buff[0] = 0;
	buff[1] = 0x80;
	buff[2] = 0x80;
	buff[3] = 0;
	pos = 4;
	/////


	if (pos == HEADER_LEN + header->dataLen)
	{
		pos = 0;
		_ParseBuff();
	}

	return;
}


// Вызывать в прерывании таймера. Квант времени описать в дефайне
void Param_Timer_Callback(void)
{
	uint64_t beforeTimer = timer_ns;
	timer_ns += PARAM_EXT_TIMER_PERIOD_NS ? PARAM_EXT_TIMER_PERIOD_NS : PARAM_INTERNAL_TIMER_PERIOD_NS;
	if ( (beforeTimer>>30) != (timer_ns>>30) )	// Раз в ~1 сек вызываем цикловую функцию
	{
		//_Param_Cycle();
	}
}
