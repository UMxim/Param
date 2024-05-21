#ifndef PARAM_CFG_H_
#define PARAM_CFG_H_
/*
Модуль для хранения настроек и статистики, а также изменение параметров по команде с UART
Для работы необходимо:

2. Задать PARAM_FLASH_OFFSET - начало сохраненных параметров во флеш или EEPROM
3. Задать PARAM_TIMER_PERIOD_MKS. Необходимо по таймеру вызывать Param_Timer_callback. Этим дефайном задается перииод вызова.
4. Заполнить массивы params и statistic по образцу
4а.Чтобы не использовать обезличенные переменные вроде param[0].value можно задать им клички типа #define STAT_UPDATE_FREQ_S
5. Написать интерфейсные функции Param_SendByte Param_GetCRC16 Param_WriteParam Param_Reset для работы с текущим железом
6. Описать ф-ию Param_GetDump(), которая вызывается при команде получения дампа
7. В обработчике прерываний получения/отправки байта по UART вызывать ф-ии Param_RecieveByte_callback Param_TransmitByte_callback
8. В обработчике прерываний таймера вызвать Param_Timer_callback

Порядок работы модуля
1. Проверка сохраненных данных на FLASH. При корректности - заполнение переменных param и statistic
2. С периодичностью STAT_UPDATE_FREQ_S проверяется изменение статистики. Если да - то сохранение на FLASH
3. При поступлении команды по UART выполнить действия и отдать ответ

*/

#define PARAM_EXT_TIMER_PERIOD_NS		1000000	// Вызов пользователем Param_Timer_callback c периодичностью XX

// Заполнить переменные параметров по образцу:
#define PARAM_PARAM_ARR	{24*60*60, "Stat_upd, sec"}   // STAT_UPDATE_FREQ_S по умолчанию - сутки

// Заполнить переменные статистики
#define PARAM_STAT_ARR	{0, "on counter"} // количество включений
					//	,{val, "other stat"},

// заполнить псевдонимы для переменных параметров. по образцу:
#define STAT_UPDATE_FREQ_S 		param[0]	// это нужный параметр - частота проверки изменения статистики и запись на флеш при необходимости
#define STAT_ON_CNT 			statistic[0]

// Выбор используемого МК
#include "param_hal_stm32L011D4P6.h"

// Заполнить тип устройства и версии!!!
#define HW_TYPE	0x01
#define HW_VER  0x01
#define SW_VER  0x01
#define DEVICE_STR "TEST_DEVICE"

#define PARAM_BUFF_SIZE 64 // Размер выделяемого буфера
#define PARAM_FOO_UPDATE_SIZE 512 // Сложный параметр - размер функции обновления для копирования в память
#endif // PARAM_CFG_H_
