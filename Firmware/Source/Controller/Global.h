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
#define FEP_COUNT						3

// Размеры массивов EP
#define VALUES_x_SIZE					1000
#define VALUES_EXT_INFO_SIZE			300

// Системные временные параметры
#define TIME_LED_BLINK					500		// мс
#define CT_SAVE_TIMEOUT					1800000 // в мс

// АЦП
#define ADC_REF_VOLTAGE					3300.0f	// мВ
#define ADC_RESOLUTION					4095
// PA0, АЦП1 канал 1: делитель 47k/(47k+10k); восстановление напряжения на датчике
#define ADC_PRESSURE_INPUT_GAIN					1.213f	// 1 / (47k / (47k + 10k))

// Таймауты циклов
#define SC_FILTER_T						100		// 
#define PNEUMATIC_READ_PAUSE			500		//
#define HOMING_PAUSE					500		//
#define HOMING_REVERSE_PAUSE			100		//
#define HOMING_TIMEOUT					45000	//
#define MOVEMENT_TIMEOUT				20000	//
#define TRM_READ_PAUSE					1000	//
#define SPI_WAIT_TIMEOUT				2000	//
#define ADAPTER_HOLD_PRESSURE_TIMEOUT	2000	//

// Шаговый привод
#define SM_FULL_ROUND_STEPS				1000ul	// steps/round
#define SM_MOVING_RER_ROUND				5000ul	// um/round

#endif // __GLOBAL_H
