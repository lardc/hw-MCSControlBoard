// -----------------------------------------
// Global definitions
// ----------------------------------------

#ifndef __GLOBAL_H
#define __GLOBAL_H


// Include
#include "stdinc.h"


// Constants
//
// Password to unlock non-volatile area for write
#define ENABLE_LOCKING				FALSE
#define UNLOCK_PWD_1				1
#define UNLOCK_PWD_2				1
#define UNLOCK_PWD_3				1
#define UNLOCK_PWD_4				1

#define DT_EPROM_ADDRESS			0
#define	SCCI_TIMEOUT_TICKS  		1000
#define SC_FILTER_T					100

#define EP_COUNT_16					1
#define EP_COUNT_32					1
#define VALUES_x_SIZE				500
#define VALUES_XLOG_x_SIZE			1000

#define	PNEUMATIC_READ_PAUSE		500			// in ms
#define PNEUMATIC_CTRL_PAUSE		1000		// in ms
#define PNEUMATIC_POWER_TIMEOUT		2000		// in ms
#define HOMING_PAUSE				500			// in ms
#define TRM_READ_PAUSE				1000		// in ms

// Stepper motors system config
#define SM_FULL_ROUND_STEPS			1000ul		// steps/round
#define SM_MOVING_RER_ROUND			5000ul		// in um/round
#define SM_MAX_POSITION				180000ul	// in um

#endif // __GLOBAL_H
