#ifndef PARAM_H_
#define PARAM_H_

#include <stdint.h>
#include <stddef.h>

typedef struct {
	uint32_t value;
	const char *text;
}params_t;

extern uint32_t *const params; // параметры
extern uint32_t *const statistic; // статистика

// Инициализация. Вызвать после инициализации железа но до других модулей (Заполняет параметры сохраненными значениями!!!)
void Param_Init(void *dump, uint16_t dump_len, int dump_size);

// А её нет! Все происходит по прерываниям! Даже запись во флеш (все равно затупим)
//void Param_Cycle(void);

// Принудительно сбросить значения на флеш
void Param_Flush(void);

void Param_RxCallback(uint8_t byte);
void Param_Timer_Callback(void);

#endif // PARAM_H_
