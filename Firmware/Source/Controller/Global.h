// -----------------------------------------
// Global definitions
// ----------------------------------------

#ifndef __GLOBAL_H
#define __GLOBAL_H

// Include
#include "stdinc.h"

// Constants
#define DT_EPROM_ADDRESS			0
#define	SCCI_TIMEOUT_TICKS  		1000
#define SC_FILTER_T					100

#define EP_COUNT_16					1
#define EP_WRITE_COUNT				0
#define FEP_COUNT					0

#define VALUES_x_SIZE				500
#define VALUES_XLOG_x_SIZE			1000

#define TIME_LED_BLINK				500
#define PRESSURE_SAMPLE_PERIOD		500
#define AVG_SAMPLES_DEF				4

#define ADC_REF_VOLTAGE				3300.0f
#define ADC_RESOLUTION				4095

#define	PNEUMATIC_READ_PAUSE		500
#define PNEUMATIC_CTRL_PAUSE		1000
#define PNEUMATIC_POWER_TIMEOUT		2000
#define HOMING_PAUSE				500
#define TRM_READ_PAUSE				1000

// Stepper motors system config
#define SM_FULL_ROUND_STEPS			1000ul		// steps/round
#define SM_MOVING_RER_ROUND			5000ul		// in um/round
#define SM_MAX_POSITION				180000ul	// in um

#endif // __GLOBAL_H
