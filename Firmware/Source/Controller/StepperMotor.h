// ----------------------------------------
// SM driver module
// ----------------------------------------

#ifndef __STEPPER_MOTOR_H
#define __STEPPER_MOTOR_H

// Include
#include "ZbBoard.h"
#include "LowLevel.h"

// Types
typedef struct __SM_Config
{
	Int16U NewPosition;
	Int16U SlowDownDistance;
	Int16U MaxSpeed;
	Int16U SlowSpeed;
	Int16U MinSpeed;
} SM_Config, *pSM_Config;

// Functions
//
// Main logic ISR call
void SM_TimerHandler();
// Connect alter handler for timer processing
void SM_ConnectAlterHandler(void *Handler);
// Steps Enable
void SM_Enable(Boolean State);
// New position in mm, speed in mm/s
void SM_GoToPosition(pSM_Config Config);
Boolean SM_IsPositioningDone();
// Homing
void SM_Homing(Int16U HomingSpeed);
Boolean SM_IsHomingDone();
Boolean SM_IsSafetyEvent();
void SM_ResetZeroPoint();
void SM_RequestStop();

#endif // __STEPPER_MOTOR_H
