#include "param_hal_stm32L011D4P6.h"

static void (*uart_rx_clbk_)(uint8_t, uint8_t);
static void (*timer_clbk_)(uint32_t);

void Param_HAL_Init(void (*uart_callback)(uint8_t, uint8_t), void (*timer_callback)(uint32_t))
{
	uart_rx_clbk_ = uart_callback;
	timer_clbk_ = timer_callback;
}

void Param_HAL_Transmit(uint8_t *data, uint32_t size)
{
}

uint32_t *Param_HAL_GetFlashDataAddr(void)
{
}

uint32_t Param_HAL_WriteFlashData(void *data, uint32_t size)
{

}
