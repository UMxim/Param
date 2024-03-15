#include "param_hal_stm32L011D4P6.h"
#include "stm32l011xx.h"
#include "param_cfg.h"

#define UPDATE_MASK(reg, mask, val) reg = ( (reg) & ~(mask) ) | (val)

static void (*clbk_)(uint8_t, uint8_t);

static uint8_t *txBuff;
static uint16_t txLen;

void USART2_IRQHandler(void)
{
	if (USART2->ISR | USART_ISR_TXE)
	{
		if (txLen --)
		{
			USART2->TDR = *(txBuff++);
		}
		else
		{
			UPDATE_MASK(USART2->CR1, USART_CR1_TXEIE_Msk, 0);
		}

	}
	else if (USART2->ISR | USART_ISR_TC)
	{

		clbk_(2, 1);// tx ok
	}
	else if (USART2->ISR | USART_ISR_RXNE)
	{
		// read
	}
}








static inline uint16_t ReceiveData(uint8_t *data)
{
	/*BOOT_UART->CR1 = USART_CR1_RE | USART_CR1_UE;
	while(!(BOOT_UART->ISR & USART_ISR_RXNE_Msk));
	*data = BOOT_UART->RDR;

	uint16_t size = (*data == 0xFF) ? (1<<_BOOT_PAGE_SIZE_POW) + 2 : *data;
	data++;
	size += 3;	// sizeByte typeByte .. CheckSummByte

	for(int i=1; i<size; i++)
	{
		while(!(BOOT_UART->ISR & USART_ISR_RXNE_Msk));
		*(data++) = BOOT_UART->RDR;
	}

	//UART->CR1 = USART_CR1_UE;
	return size;*/
}

static void FlashWriteWord(uint32_t addr, uint32_t data, uint32_t isErase) // 0 или ERASE
{
	FLASH->PECR = isErase;
	*(uint32_t *)addr = data;
	while ( (FLASH->SR & FLASH_SR_BSY) );
}
// Boot PA9 PA10

void Param_HAL_Init(void (*callback)(uint8_t, uint8_t))
{
	RCC->APB1ENR |= RCC_APB1ENR_USART2EN;
	RCC->IOPENR |= RCC_IOPENR_GPIOAEN;
	USART2->CR3 = USART_CR3_HDSEL;
	USART2->BRR = (SYS_CLK_FREQ_HZ)/PARAM_UART_BR;
	USART2->CR1 |= USART_CR1_RXNEIE | USART_CR1_UE;
	// GPIO
	GPIOA->OTYPER |= GPIO_OTYPER_OT_9; // OpenDrain 1
	UPDATE_MASK(GPIOA->PUPDR, GPIO_PUPDR_PUPD9_Msk, GPIO_PUPDR_PUPD9_0);		//2
	UPDATE_MASK(GPIOA->AFR[1], GPIO_AFRH_AFSEL9_Msk, 4 << GPIO_AFRH_AFSEL9_Pos);
	UPDATE_MASK(GPIOA->MODER, GPIO_MODER_MODE9_Msk, GPIO_MODER_MODE9_1);

	clbk_ = callback;
}

void Param_HAL_Transmit(uint8_t *data, uint16_t size)
{
	USART2->CR1 = USART_CR1_TCIE | USART_CR1_TXEIE | USART_CR1_TE | USART_CR1_UE;	// txcmplt , txempty, enableTx, enableUART
	USART2->TDR = *(data++);
	txBuff = data;
	txLen = size - 1;
}

uint32_t *Param_HAL_GetFlashDataAddr(void)
{
	return (uint32_t*)DATA_EEPROM_BASE;
}

uint32_t Param_HAL_WriteFlashData(void *data, uint32_t size)
{
	return 0;
}

void Param_HAL_Reset()
{
	NVIC_SystemReset();
}

void Param_HAL_SetBootCfg()
{

}
