#include "param.h"
#include "param_cfg.h"

#define PARAM_LIST_NUM ( sizeof(params) / sizeof(params_t) )
#define STAT_LIST_NUM ( sizeof(statistic) / sizeof(params_t) )
#define STAT_OFFSET (PARAM_FLASH_OFFSET +  sizeof(params_t) + 4 ) // CheckSumm  
#define BUFF_MAX_SIZE (1 + 1 + 255 + 1 ) // T L V X
#define CHECKSUMM_INIT  0x0x96C30FA5
#define US_IN_S		1000000UL
#define SAFE_TIMER_INTERVAL 300000UL // 300ms

(align 4) params_t params[] =	{
	PARAM_PARAM_ARR	}; // параметры
	
(align 4) params_t statistic[] = {
	PARAM_STAT_ARR }; // статистика

#if (PARAM_IS_STATIC_BUFF)
static uint8_t buff[BUFF_MAX_SIZE];
#else 
static uint8_t *buff;
#endif

static uint32_t timer_us = 0;
static uint32_t timer_s = 0;

static uint8_t state = STATE_NO_INIT;

static void Fill_Arr_(uint32_t flashOffset, uint16_t N_elem, params_t *dest)
{
	uint32_t *data = (uint32_t *)flashOffset;		
	uint32_t size = N_elem * sizeof(uint32_t) + 2;
	uint32_t cs = CHECKSUMM_INIT;
	for (int i=0; i<=N_elem; i++)
		cs ^= data[i];
	if ( cs != 0 )	{
		return;
	}	
	// read data
	for (int i=0; i< N_elem; i++) {
		dest->value = *data;
		data++;
	}		
}

// Возврат буфера
static void GetBuff_(uint32_t size)
{
#if (PARAM_IS_STATIC_BUFF == 0 
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
	state_ = STATE_TX;
}


static void ParseBuff_(void)	
{
	// check crc
	if (Param_GetCRC16(buff, buff[1]) != 0 )	{
		
	}
}


void Param_Init(void)
{
	// read params	
	Fill_Arr_(PARAM_FLASH_OFFSET, PARAM_LIST_NUM, &params);	
	// read statistic
	Fill_Arr_(STAT_OFFSET, STAT_LIST_NUM, &statistic);	
	state_ = STATE_IDLE;	
}

// Вызываем в прерывании при получении байта
void Param_RecieveByte_callback(byte)
{	
	static uint8_t firstByte;		
	if ( (state == STATE_IDLE) || (state == STATE_RX) ) ; else return;	
	
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
	}	
}

// Вызывать в прерывании по отправке байта
uint8_t Param_TransmitByte_callback(void)
{
	if (state == STATE_NO_INIT) return;
	
}

// Вызывать в прерывании таймера. Квант времени описать в дефайне 
void Param_Timer_callback(void)
{	
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
	if ( (timer_us - safeTimer) > SAFE_TIMER_INTERVAL )...................
}