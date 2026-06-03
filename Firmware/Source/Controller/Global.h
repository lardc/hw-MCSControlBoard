#ifndef __GLOBAL_H
#define __GLOBAL_H

// Definitions
// 
#define	SCCI_TIMEOUT_TICKS						1000	// Таймаут интерфейса SCCI (в мс)
#define EP_WRITE_COUNT							0		// Количество массивов для записи
#define EP_COUNT								0		// Количество массивов для чтения
#define FEP_COUNT								0		// Количество массивов для чтения типа float
#define VALUES_x_SIZE							0		// Размер массивов
#define ENABLE_LOCKING							FALSE	// Защита NV регистров паролем

// Временные параметры
#define TIME_LED_BLINK							500		// Мигание светодиодом (в мс)
#define TIME_FAULT_LED_BLINK					250		// Мигание светодиодом в состоянии Fault
#define PNEUMO_DELAY							500		// Задержка при работе с пневматикой (в мс)
#define PRESSURE_SAMPLE_PERIOD					500		// Интервал опроса датчика давления

// Параметры измерения
#define ADC_REF_VOLTAGE							3300.0f	// Опорное напряжение (в В)
#define ADC_RESOLUTION							4095	// Разрешение АЦП

#endif //  __GLOBAL_H
