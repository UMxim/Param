#ifndef PARAM_HAL_STM32L011D4P6_H
#define PARAM_HAL_STM32L011D4P6_H

#include <stdint.h>

#define PARAM_FLASH_OFFSET 				0x12345678 // адрес нулевого смещения
#define PARAM_UART						UART1
#define PARAM_UART_BR					57600
#define SYS_CLK_FREQ_HZ					32000000

void Param_HAL_Init(void (*callback)(uint8_t, uint8_t));

void Param_HAL_Transmit(uint8_t *data, uint16_t size);

uint32_t *Param_HAL_GetFlashDataAddr(void);
uint32_t Param_HAL_WriteFlashData(void *data, uint32_t size);

void Param_HAL_Reset();
void Param_HAL_SetBootCfg();

#endif //PARAM_HAL_STM32L011D4P6_H
