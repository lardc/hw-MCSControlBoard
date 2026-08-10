// -----------------------------------------
// Global definitions
// ----------------------------------------

#ifndef __GLOBAL_H
#define __GLOBAL_H

#include "stdinc.h"

// Параметры интерфейсов
#define SCCI_TIMEOUT_TICKS				1000	// мс

#define EP_COUNT						0
#define EP_WRITE_COUNT					0
#define FEP_COUNT						1

// Размеры массивов EP
#define VALUES_x_SIZE					500
#define VALUES_EXT_INFO_SIZE			300

// Системные временные параметры
#define TIME_LED_BLINK					500		// мс
#define PRESSURE_SAMPLE_PERIOD			500		// мс
#define AVG_SAMPLES_DEF					4

// АЦП
#define ADC_REF_VOLTAGE					3300.0f	// мВ
#define ADC_RESOLUTION					4095

// Таймауты циклов
#define SC_FILTER_T						100		// мс
#define PNEUMATIC_READ_PAUSE			500		// мс
#define PNEUMATIC_CTRL_PAUSE			1000	// мс
#define PNEUMATIC_POWER_TIMEOUT			2000	// мс
#define HOMING_PAUSE					500		// мс
#define HOMING_REVERSE_PAUSE			100		// мс
#define TRM_READ_PAUSE					1000	// мс
#define SPI_WAIT_TIMEOUT				2000	// мс
#define ADAPTER_HOLD_PRESSURE_TIMEOUT	2000	// мс

// Шаговый привод
#define SM_FULL_ROUND_STEPS				1000ul	// steps/round
#define SM_MOVING_RER_ROUND				5000ul	// um/round
#define SM_MAX_POSITION					180000ul	// um

#endif // __GLOBAL_H
