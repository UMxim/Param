#ifndef PARAM_H_
#define PARAM_H_

#include <stdint.h>
#include <stddef.h>
#include "param_cfg.h"

typedef struct {
	uint32_t value;
	const char *text;
}params_t;

extern uint32_t params[]; // параметры
extern uint32_t statistic[]; // статистика

// Инициализация. Вызвать после инициализации железа но до других модулей (Заполняет параметры сохраненными значениями!!!)
void Param_Init(void *dump, int dump_size);

// А её нет! Все происходит по прерываниям! Даже запись во флеш (все равно затупим)
//void Param_Cycle(void);

// Принудительно сбросить значения на флеш
void Param_Flush(void);


void Param_Timer_Callback(uint32_t period_ns);
#endif // PARAM_H_
