#ifndef PARAM_HAL_STM32L011D4P6_H
#define PARAM_HAL_STM32L011D4P6_H

#include <stdint.h>

#define PARAM_FLASH_OFFSET 				0x12345678 // адрес нулевого смещения
#define PARAM_UART						UART1
#define PARAM_INTERNAL_TIMER_PERIOD_NS	1000000 // 1ms

void Param_HAL_Init(void (*uart_callback)(uint8_t), void (*timer_callback)(void));
void Param_HAL_Transmit(uint8_t *data, uint32_t size);

uint32_t *Param_HAL_GetFlashDataAddr(void);
uint32_t Param_HAL_WriteFlashData(void *data, uint32_t size);

#endif //PARAM_HAL_STM32L011D4P6_H
