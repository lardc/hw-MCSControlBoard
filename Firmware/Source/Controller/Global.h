// -----------------------------------------
// Global definitions
// ----------------------------------------

#ifndef __GLOBAL_H
#define __GLOBAL_H

// Include
#include "stdinc.h"

// Constants
#define	SCCI_TIMEOUT_TICKS  			1000
#define SC_FILTER_T						100

#define EP_COUNT						0
#define EP_WRITE_COUNT					0
#define FEP_COUNT						1

#define VALUES_x_SIZE					500
#define VALUES_EXT_INFO_SIZE			300

#define TIME_LED_BLINK					500
#define PRESSURE_SAMPLE_PERIOD			500
#define AVG_SAMPLES_DEF					4

#define ADC_REF_VOLTAGE					3300.0f
#define ADC_RESOLUTION					4095

#define	PNEUMATIC_READ_PAUSE			500
#define PNEUMATIC_CTRL_PAUSE			1000
#define PNEUMATIC_POWER_TIMEOUT			2000
#define HOMING_PAUSE					500
#define TRM_READ_PAUSE					1000
#define SPI_WAIT_TIMEOUT				2000	// мс (тики CONTROL_TimeCounter)
#define ADAPTER_HOLD_PRESSURE_TIMEOUT	2000	// мс

// Stepper motors system config
#define SM_FULL_ROUND_STEPS				1000ul		// steps/round
#define SM_MOVING_RER_ROUND				5000ul		// in um/round
#define SM_MAX_POSITION					180000ul	// in um

#endif // __GLOBAL_H
