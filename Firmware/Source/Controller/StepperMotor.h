// ----------------------------------------
// SM driver module
// ----------------------------------------

#ifndef __STEPPER_MOTOR_H
#define __STEPPER_MOTOR_H

// Include
#include "LowLevel.h"

// Types
typedef struct __SM_Params
{
	Int16U NewPosition;
	Int16U MaxSpeed;
	Int16U MinSpeed;
} SM_Params, *pSM_Params;

// Functions
//
// Main logic ISR call
void SM_TimerHandler();
// Connect alter handler for timer processing
void SM_ConnectAlterHandler(void *Handler);
// Config Stepper Motor
void SM_Config(pSM_Params Params, Int16U PositionMm);
// New position in mm, speed in mm/s
void SM_GoToPosition(pSM_Params Params);
Boolean SM_IsBusy();
Boolean SM_IsPositioningDone();
// Homing
void SM_Homing();
Boolean SM_IsHomingDone();
void SM_ResetZeroPoint();
void SM_RequestStop();

#endif // __STEPPER_MOTOR_H
