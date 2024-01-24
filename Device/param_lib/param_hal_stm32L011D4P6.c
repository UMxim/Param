#include "param_hal_stm32L011D4P6.h"

static void (*uart_rx_clbk)(uint8_t, uint8_t);
static void (*timer_clbk)(uint32_t);

void Param_HAL_Init(void)
{
	uart_rx_clbk = 0;
}

void Param_HAL_Transmit(uint8_t *data, uint32_t size)
{
}

void Param_HAL_RegisterCallback_Uart_Rx(void (*fn)(uint8_t, uint8_t))
{
	uart_rx_clbk = fn;
}

void Param_HAL_RegisterCallback_Timer(void (*fn)(uint32_t))
{
	timer_clbk = fn;
}


uint32_t *Param_HAL_GetFlashDataAddr(void)
{
}
