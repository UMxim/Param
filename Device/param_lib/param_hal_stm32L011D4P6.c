#include "param_hal_stm32L011D4P6.h"
#include "param_cfg.h"

#define UPDATE_MASK(reg, mask, val) reg = ( (reg) & ~(mask) ) | (val)

static void(*_callback_rx)(uint8_t);

static uint8_t *txBuff;
static uint16_t txLen;
volatile static uint8_t isTransmitt = 0; // Передача в процессе.

void USART2_IRQHandler(void)
{
	if (USART2->ISR | USART_ISR_TC) // Да, надо обрабатывать не по complite, а по опустошению буфера. Но по правильному глючит
	{
		if (txLen == 0)
		{
			USART2->CR1 = USART_CR1_RXNEIE | USART_CR1_RE | USART_CR1_UE;	// txcmplt , enableRx, enableUART
			isTransmitt = 0;
		}
		txLen --;
		USART2->TDR = (uint8_t)*(txBuff++);
	}
	//else if (USART2->ISR | USART_ISR_TC)
	/*{
		USART2->CR1 = 0;//USART_CR1_RXNEIE | USART_CR1_RE | USART_CR1_UE;
	}*/
	else if (USART2->ISR | USART_ISR_RXNE)
	{
		uint8_t byte = USART2->RDR;
		_callback_rx(byte);// read
	}
}

void Param_HAL_Transmit(uint8_t *data, uint16_t size)
{
	isTransmitt = 1;
	USART2->TDR = (uint8_t)*(data++);
	USART2->CR1 = USART_CR1_TCIE | USART_CR1_TE | USART_CR1_UE;	// txempty, enableTx, enableUART
	txBuff = data;
	txLen = size - 1;
}

uint32_t *Param_HAL_GetFlashDataAddr(void)
{
	return (uint32_t*)DATA_EEPROM_BASE;
}

void Param_HAL_WriteFlashData(uint32_t *data, uint32_t size_word)
{
	//const uint32_t PEKEY1 = 0x89ABCDEF;
	//const uint32_t PEKEY2 = 0x02030405;
	if ((FLASH->PECR & FLASH_PECR_PELOCK) != 0)
	{
	FLASH->PEKEYR = 0x89ABCDEF;
	FLASH->PEKEYR = 0x02030405;
	}

	uint32_t *eeprom_ptr = (uint32_t*)DATA_EEPROM_BASE;
	while(size_word--)
	{
		while (FLASH->SR & FLASH_SR_BSY) ;
		if (*eeprom_ptr != *data)
			*eeprom_ptr = *data;
		eeprom_ptr++;
		data++;
	}
}

// Boot PA9 PA10
void Param_HAL_Init(void(*callback_rx)(uint8_t))
{
	RCC->APB1ENR |= RCC_APB1ENR_USART2EN;
	RCC->IOPENR |= RCC_IOPENR_GPIOAEN;
	USART2->CR3 = USART_CR3_HDSEL;
	USART2->BRR = PARAM_SYS_CLK_FREQ_HZ / PARAM_UART_BR;
	USART2->CR1 |= USART_CR1_RXNEIE | USART_CR1_RE | USART_CR1_UE;
	// GPIO
	GPIOA->OTYPER |= GPIO_OTYPER_OT_9; // OpenDrain 1
	UPDATE_MASK(GPIOA->PUPDR, GPIO_PUPDR_PUPD9_Msk, GPIO_PUPDR_PUPD9_0);		//2
	UPDATE_MASK(GPIOA->AFR[1], GPIO_AFRH_AFSEL9_Msk, 4 << GPIO_AFRH_AFSEL9_Pos);
	UPDATE_MASK(GPIOA->MODER, GPIO_MODER_MODE9_Msk, GPIO_MODER_MODE9_1);
	NVIC->ISER[0] = 1<<USART2_IRQn;
	__enable_irq();
	_callback_rx = callback_rx;
}

void Param_HAL_Reset()
{
	NVIC_SystemReset();
}

void Param_HAL_FW_Update(uint8_t* buff)
{

	FLASH->PEKEYR =  0x89ABCDEF;
	FLASH->PEKEYR =  0x02030405;
	FLASH->PRGKEYR = 0x8C9DAEBF;
	FLASH->PRGKEYR = 0x13141516;

	while(1)
	{
		uint8_t pos = 1;
		// rx
		/*BOOT_UART->CR1 = USART_CR1_RE | USART_CR1_UE;
		while(!(BOOT_UART->ISR & USART_ISR_RXNE_Msk));
		buff[0] = BOOT_UART->RDR;

		for(int i=1; i<buff[0]+3; i++)
		{
			while(!(BOOT_UART->ISR & USART_ISR_RXNE_Msk));
			buff[pos++] = BOOT_UART->RDR;
		}
		// work

		// enable Tx
		BOOT_UART->CR1 = USART_CR1_TE | USART_CR1_UE;
		while (size--)
		{
			BOOT_UART->TDR = *(data++);
			while(!(BOOT_UART->ISR & USART_ISR_TXE_Msk));
		}

		while(!(BOOT_UART->ISR & USART_ISR_TC_Msk));

		//UART->CR1 = USART_CR1_UE;*/
	}

}


uint8_t GetTxFlag(void)
{
	return isTransmitt;
}
