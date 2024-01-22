#include "stm32l011xx.h"
#include "boot_config.h"

#define ANSWER_WAIT_MS 		512U // не более 8000 для этого МК
#define SYS_CLK_FREQ_KHZ 	2000U

// служебные
#define UPDATE_MASK(reg, mask, val) reg = ( (reg) & ~(mask) ) | (val)

#define ERASE 				(FLASH_PECR_ERASE | FLASH_PECR_PROG)

__attribute__((section(".consts"), used)) const struct // Думал сделать доступ из основной проги, но решил все вынести в .h файл и подключать и тут и там. А это пусть останется
{	
	uint8_t len;
	uint8_t type;
	uint8_t bootVer;
	uint8_t hwType;
	uint8_t hwVer;
	uint8_t sectorSize;
	uint8_t sectorNum_hi;
	uint8_t sectorNum_lo;
	uint8_t cs;
}	vers = 
{
	0x06, 	// len
	0x07, 	// type
	_BOOT_BOOT_VER, 
	_BOOT_HW_TYPE, 
	_BOOT_HW_VER, 
	_BOOT_PAGE_SIZE_POW, 
	_BOOT_PAGES_FOR_WRITE >> 8,
	_BOOT_PAGES_FOR_WRITE,
	0x06 ^ 0x07 ^ _BOOT_BOOT_VER ^ _BOOT_HW_TYPE ^ _BOOT_HW_VER ^ _BOOT_PAGE_SIZE_POW ^ (_BOOT_PAGES_FOR_WRITE >> 8) ^ _BOOT_PAGES_FOR_WRITE	// xor
};

static void SendData(const uint8_t* data, uint32_t size)
{
	// enable Tx
	BOOT_UART->CR1 = USART_CR1_TE | USART_CR1_UE;
	while (size--)
	{
		BOOT_UART->TDR = *(data++);		
		while(!(BOOT_UART->ISR & USART_ISR_TXE_Msk));
	}
	
	while(!(BOOT_UART->ISR & USART_ISR_TC_Msk));
		
	//UART->CR1 = USART_CR1_UE;
}

static inline uint16_t ReceiveData(uint8_t *data)
{	
	BOOT_UART->CR1 = USART_CR1_RE | USART_CR1_UE;
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
	return size;
}

static void FlashWriteWord(uint32_t addr, uint32_t data, uint32_t isErase) // 0 или ERASE
{		
	FLASH->PECR = isErase;
	*(uint32_t *)addr = data;
	while ( (FLASH->SR & FLASH_SR_BSY) );
}

static void Go_To_User_App(void)
{
    uint32_t app_jump_address;
	
    typedef void(*pFunction)(void);//объявляем пользовательский тип
    pFunction Jump_To_Application;//и создаём переменную этого типа
		
		SCB->VTOR = _BOOT_FW_PAGE_START;
	
    app_jump_address = *( uint32_t*) (_BOOT_FW_PAGE_START + 4);    //извлекаем адрес перехода из вектора Reset
    Jump_To_Application = (pFunction)app_jump_address;            //приводим его к пользовательскому типу
    __set_MSP(*(__IO uint32_t*) _BOOT_FW_PAGE_START);          //устанавливаем SP приложения                                           
    Jump_To_Application();		                        //запускаем приложение	
}

int main(void)
{	
	
// BR = 9600 !!!!!	
	// SysTick
	SysTick->LOAD = ANSWER_WAIT_MS * SYS_CLK_FREQ_KHZ;
	SysTick->VAL = 0;
	SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_ENABLE_Msk;
	// UART
	RCC->APB1ENR = RCC_APB1ENR_LPUART1EN | RCC_APB1ENR_USART2EN; 
	RCC->IOPENR = RCC_IOPENR_GPIOAEN;
	BOOT_UART->CR3 = USART_CR3_HDSEL;
	BOOT_UART->BRR = BOOT_UART_BRR;
	// GPIO	
	BOOT_UART_PORT->OTYPER |= BOOT_GPIO_OPEN_DRAIN; // OpenDrain 1
	UPDATE_MASK(BOOT_UART_PORT->PUPDR, BOOT_GPIO_PULL_UP_MSK, BOOT_GPIO_PULL_UP_VAL);		//2
	UPDATE_MASK(BOOT_UART_PORT->AFR[BOOT_GPIO_AFR_NUM], BOOT_GPIO_AFR_MSK, BOOT_GPIO_AFR << BOOT_GPIO_AFR_POS);
	UPDATE_MASK(BOOT_UART_PORT->MODER, BOOT_GPIO_MODER_MSK, BOOT_GPIO_MODER_VAL);
	
	//
	uint32_t buff32[(1<<_BOOT_PAGE_SIZE_POW) + 5]; // потому-что памяти завались....
	uint8_t *buff = (uint8_t *)buff32;
	
	SendData((const void *)&vers, sizeof(vers));
	
	BOOT_UART->CR1 = USART_CR1_RE | USART_CR1_UE;
	
	while(!(BOOT_UART->ISR & USART_ISR_RXNE_Msk))
	{
		if (SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk)
			Go_To_User_App();
	}
	
	FLASH->PEKEYR =  0x89ABCDEF;
	FLASH->PEKEYR =  0x02030405;
	FLASH->PRGKEYR = 0x8C9DAEBF;
	FLASH->PRGKEYR = 0x13141516;
	
	uint16_t size;
	uint8_t xor; 
	int i;
	static const uint8_t errCS[] =   {1, 0x7F, 1, 1^0x7F^1 };
	static const uint8_t errType[] = {1, 0x7F, 2, 1^0x7F^2 };
	while (1)
	{
		size = ReceiveData(buff);
		xor = 0;
		for (i=0; i<size; i++)
			xor ^= buff[i];
		if (xor)
		{					
			SendData(errCS, sizeof(errCS));
			continue;
		}
		
		if (buff[1] == 0x87)
			NVIC_SystemReset();
	
		if (buff[1] != 0x88)
		{			
			SendData(errType, sizeof(errType));
			continue;
		}
	
		uint32_t addr = _BOOT_FW_PAGE_START + (((buff[2]<<8) + buff[3]) << _BOOT_PAGE_SIZE_POW);
		FlashWriteWord(addr, 0, ERASE);
		uint32_t word;
		for(i=0; i < (1<<_BOOT_PAGE_SIZE_POW); i+=4)
		{			
			word = /*__REV*/(*(uint32_t*)&buff[4+i]);
			FlashWriteWord(addr, word, 0);
			addr += 4;
		}	
		*(uint32_t*)buff = (0x01 << 0) + (0x08 << 8) + ((0x01^0x08)<<16);// 0x01 0x08, 0x08, 0xXX			
		SendData(buff, 3);
		
	}	
}

