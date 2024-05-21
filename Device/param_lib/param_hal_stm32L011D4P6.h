#ifndef PARAM_HAL_STM32L011D4P6_H
#define PARAM_HAL_STM32L011D4P6_H

#include <stdint.h>
#include "stm32l011xx.h"

#define PARAM_UART_BR					57600		// скорость UART
#define PARAM_SYS_CLK_FREQ_HZ			32000000	// частота ядра
#define PARAM_SECTOR_NUM				4			// количество секторов на флеш
#define PARAM_SECTOR_SIZE				4096		// размер сектора в байтах

void Param_HAL_Init(void(*callback_rx)(uint8_t));

void Param_HAL_Transmit(uint8_t *data, uint16_t size);

uint32_t *Param_HAL_GetFlashDataAddr(void);
void Param_HAL_WriteFlashData(uint32_t *data, uint32_t size_word);

void Param_HAL_Reset();
void Param_HAL_FW_Update();

uint8_t GetTxFlag(void);
#endif //PARAM_HAL_STM32L011D4P6_H
