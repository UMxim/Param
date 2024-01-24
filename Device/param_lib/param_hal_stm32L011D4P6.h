#ifndef PARAM_HAL_STM32L011D4P6_H
#define PARAM_HAL_STM32L011D4P6_H

#include <stdint.h>

#define PARAM_FLASH_OFFSET 		0x12345678 // адрес нулевого смещения
#define PARAM_UART				UART1

void Param_HAL_Init(void);
void Param_HAL_Transmit(uint8_t *data, uint32_t size);
void Param_HAL_RegisterCallback_Uart_Rx(void (*fn)(uint8_t, uint8_t));
void Param_HAL_RegisterCallback_Timer(void (*fn)(uint32_t));

uint32_t *Param_HAL_GetFlashDataAddr(void);

#endif //PARAM_HAL_STM32L011D4P6_H
