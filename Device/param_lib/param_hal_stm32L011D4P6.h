#ifndef PARAM_HAL_STM32L011D4P6_H
#define PARAM_HAL_STM32L011D4P6_H

#include <stdint.h>

#define PARAM_UART_BR					57600
#define SYS_CLK_FREQ_HZ					32000000

void Param_HAL_Init(void(*callback_rx)(uint8_t));

void Param_HAL_Transmit(uint8_t *data, uint16_t size);

uint32_t *Param_HAL_GetFlashDataAddr(void);
void Param_HAL_WriteFlashData(uint32_t *data, uint32_t size_word);

void Param_HAL_Reset();
void Param_HAL_SetBootCfg();

#endif //PARAM_HAL_STM32L011D4P6_H
