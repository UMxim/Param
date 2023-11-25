#include "param.h"
#include "param_cfg.h"

#define PARAM_LIST_NUM ( sizeof(paramsC) / sizeof(params_t) )
#define STAT_LIST_NUM ( sizeof(statisticC) / sizeof(params_t) )
#define BUFF_MAX_SIZE (1 + 1 + 255 + 1 ) // T L V X
#define CHECKSUMM_INIT  0x96C30FA5UL
#define US_IN_S		1000000UL
#define SAFE_TIMER_INTERVAL 300000UL // 300ms

void Param_RecieveByte_Callback(uint8_t byte, uint8_t isReset);
void Param_Timer_Callback(uint32_t period);

static const params_t paramsC[] =	{
	PARAM_PARAM_ARR	}; // параметры
	
static const params_t statisticC[] = {
	PARAM_STAT_ARR }; // статистика

uint32_t params[PARAM_LIST_NUM]; // параметры
	
uint32_t statistic[STAT_LIST_NUM]; // статистика
	
#if (PARAM_IS_STATIC_BUFF)
static uint8_t buff[BUFF_MAX_SIZE];
#else 
static uint8_t *buff;
#endif

static uint32_t timer_us = 0;
static uint32_t timer_s = 0;

static uint8_t isInit = 0;

// Возврат буфера
static void GetBuff_(uint32_t size)
{
#if (PARAM_IS_STATIC_BUFF == 0 )
if (size > BUFF_MAX_SIZE) {
	return NULL;
}
buff = malloc(size);
#endif
return;
}

static void FreeBuff(void *buff)
{
#if (PARAM_IS_STATIC_BUFF == 0)
	free(buff);
#endif
}

static void Update_Stat_(void)
{
	
}
static void SetError(uint8_t errN)
{
	 
}

static void SendBuff_(void)
{

}


static void ParseBuff_(void)	
{
	// check crc
}


void Param_Init(void *dump, int dump_size)
{	
	Param_HAL_Init();
	Param_HAL_RegisterCallback_Uart_Rx(Param_RecieveByte_Callback);
	Param_HAL_RegisterCallback_Timer(Param_Timer_Callback);
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
		params[i] = (cs) ? paramsC[i].value : data[i];		
	}		
	
	for (int i=0; i<STAT_LIST_NUM; i++)
	{
		statistic[i] = (cs) ? statisticC[i].value : data[i+PARAM_LIST_NUM];
	}
	isInit = 1;	
}

// Вызываем в прерывании при получении байта
void Param_RecieveByte_Callback(uint8_t byte, uint8_t isReset)
{	
	/*static uint8_t firstByte;		
	if ( !isInit ) ; else return;	
	
	switch (counter)	{
		case 0:			
			firstByte = byte;
			counter++;
			state = STATE_RX;
			safeTimer = timer_us;
			break;
		case 1:			
			GetBuff_(1 + 1 + byte + 2);
			if (!buff) {
				state = STATE_NO_INIT; // вот и всё
				return;
			}
			buff[0] = firstByte;
			buff[1] = byte;
			counter++;
			break;		
		default:
			buff[counter]= byte;
			counter++;			
			if (counter == (2 + buff[0] + 1)) {	// все получено, обрабатываем				
				ParseBuff_();	
				SendBuff_();	
				counter = 0;
				state = STATE_IDLE;
			}
			break;
	}	*/
}

// Вызывать в прерывании по отправке байта
uint8_t Param_TransmitByte_callback(void)
{
	if (!isInit) return 0;
	
}

// Вызывать в прерывании таймера. Квант времени описать в дефайне 
void Param_Timer_Callback(uint32_t period)
{	/*
	static uint32_t nextStatUpdate = STAT_UPDATE_FREQ_S;
	if (state == STATE_NO_INIT) return;
	timer_us += PARAM_TIMER_PERIOD_MKS;
	uint32_t fullSec = US_IN_S * timer_s;
	if ( (timer_us - fullSec) >= US_IN_S ) {
		timer_s++;
	}
	if (timer_s >= nextStatUpdate) {
		nextStatUpdate+=STAT_UPDATE_FREQ_S;
		Update_Stat_();
	}
	if ( (timer_us - safeTimer) > SAFE_TIMER_INTERVAL )...................*/
}