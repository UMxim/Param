#ifndef PARAM_H_
#define PARAM_H_

#include <stdint.h>

typedef struct {
	uint32_t value;
	const char *text;
}params_t;

extern uint32_t params[]; // параметры
extern uint32_t statistic[]; // статистика

// Инициализация. Вызвать после инициализации железа но до других модулей (Заполняет параметры сохраненными значениями!!!)
void Param_Init(void);

// А её нет! Все происходит по прерываниям! Даже запись во флеш (все равно затупим)
//void Param_Cycle(void);


#endif // PARAM_H_
