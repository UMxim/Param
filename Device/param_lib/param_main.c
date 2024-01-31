#include "param_main.h"
#include "param_cfg.h"

#define PARAM_LIST_NUM ( sizeof(paramsDef) / sizeof(params_t) )
#define STAT_LIST_NUM ( sizeof(statisticDef) / sizeof(params_t) )
#define BUFF_MAX_SIZE (1 + 1 + 0xFF + 1) // 0x102 258  T L V X
#define CHECKSUMM_INIT  0x96C30FA5UL
#define TIME_1_BYTE (1200000000 / 5760) // Время получения одного байта : 1E9 ns / байт в секунде + 20%

#define BUFF_DATA_LEN	buff[0]
#define BUFF_LEN 		(2 + BUFF_DATA_LEN + 1)
#define BUFF_TYPE 		buff[1]
#define BUFF_DATA		(buff + 2)

void Param_RecieveByte_Callback(uint8_t byte, uint8_t isReset);
void Param_Timer_Callback(uint32_t period);

static const params_t paramsDef[] =
    {
	PARAM_PARAM_ARR
    }; // параметры

static const params_t statisticDef[] =
    {
	PARAM_STAT_ARR
    }; // статистика

uint32_t params[PARAM_LIST_NUM]; // параметры

uint32_t statistic[STAT_LIST_NUM]; // статистика

// для выравнивания поля data на 4
static uint32_t buff32[BUFF_MAX_SIZE/4 + 1];
static uint8_t *buff = (uint8_t *)buff32 + 2; // поле DATA попадет на выровненную по 4 часть

static volatile uint64_t timer_ns = 0;
static volatile uint64_t rx_timer;

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

static void CalcCS()
{
	uint16_t len = 2 + BUFF_DATA_LEN;
	uint8_t cs = 0;
	for (int i=0; i<len; i++)
		cs^=buff[i];
	buff[len] = cs;
}

static uint8_t AddStr(const char *str, uint16_t pos_in_data)
{
	while(str)
		if (pos_in_data <= 0xFF)
			BUFF_DATA[pos_in_data++] = *str++;
	return pos_in_data - 1;
}

static inline void Get_info()
{
	BUFF_TYPE = TYPE_GET_INFO;
	BUFF_DATA[0] = HW_TYPE;
	BUFF_DATA[1] = HW_VER;
	BUFF_DATA[2] = SW_VER;
	BUFF_DATA[3] = 0;

	BUFF_DATA_LEN = AddStr(DEVICE_STR, 4);
}

static inline error_t Get_statistic(uint8_t N)
{
	if (N >= STAT_LIST_NUM - 1)
		return ERROR_DATA;

	BUFF_TYPE = TYPE_GET_STATISTIC;
	BUFF_DATA[0] =

	BUFF_DATA_LEN =

	statistic[STAT_LIST_NUM];
//	Value	4	Значение параметра статистики. В формате Big Endian
	//String	XX	Строка с описанием параметра статистики.

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
	CalcCS();
	Param_HAL_Transmit(buff, 4);
}

static void ParseBuff_(void)
{
	Param_RecieveByte_Callback(0, 1);	// reset Rx

	// check
	uint16_t lenRx = 2 + BUFF_LEN + 1;
	uint8_t cs = 0;
	for (int i=0; i<lenRx; i++)
		cs^= buff[i];

	if (cs)
		return SendError(ERROR_CHECKSUMM);

	if ( (buff[1] & 0x80) == 0)
		return SendError(ERROR_TYPE);

	error_t ans = ERROR_OK;
	switch (buff[1] & 0x7F)
	{
	case TYPE_GET_INFO:
		Get_info();
		break;
	case TYPE_GET_STATISTIC:
		ans = Get_statistic(1);
		break;
	case TYPE_RESET_STATISTIC:
		break;
	case TYPE_GET_PARAM:
		break;
	case TYPE_WRITE_PARAM:
		break;
	case TYPE_GET_DUMP:
		break;
	case TYPE_RESET:
		break;
	case TYPE_WRITE_FW:
		break;
		}

	if (ans != ERROR_OK)
		SendError(ans);

	CalcCS();

	Param_HAL_Transmit(buff, 2 + buff[0] + 1);
}


void Param_Init(void *dump, int dump_size)
{
	Param_HAL_Init(Param_RecieveByte_Callback, Param_Timer_Callback);
	// read params
	uint16_t dataSize_Word = PARAM_LIST_NUM + STAT_LIST_NUM + 1;
	uint32_t *data = Param_HAL_GetFlashDataAddr();

	uint32_t cs = CHECKSUMM_INIT;
	for (int i=0; i<=dataSize_Word; i++)
	{
		cs ^= data[i];
	}

	for (int i=0; i<PARAM_LIST_NUM; i++)
	{
		params[i] = (cs) ? paramsDef[i].value : data[i];
	}

	for (int i=0; i<STAT_LIST_NUM; i++)
	{
		statistic[i] = (cs) ? statisticDef[i].value : data[i+PARAM_LIST_NUM];
	}
}

// Вызываем в прерывании при получении байта
void Param_RecieveByte_Callback(uint8_t byte, uint8_t isReset)
{
	static uint16_t pos = 0;	// позиция в массиве для текущего байта

	if (isReset)
	{
		pos = 0;
		return;
	}
	// инициируем таймер обработки
	rx_timer = ( (pos == 0) ? timer_ns  : rx_timer ) + TIME_1_BYTE;

	buff[pos] = byte;
	pos++;
	// проверяем выход за пределы
	if (pos == BUFF_MAX_SIZE)
		pos--;

	return;
}


// Вызывать в прерывании таймера. Квант времени описать в дефайне
void Param_Timer_Callback(uint32_t period)
{
	// проверка таймера получения
	if (rx_timer < timer_ns)
		ParseBuff_();		// обрабатываем принятую последовательность

	timer_ns += PARAM_EXT_TIMER_PERIOD_NS != 0 ? PARAM_EXT_TIMER_PERIOD_NS : PARAM_INTERNAL_TIMER_PERIOD_NS; // 1ms по умолчанию

}
