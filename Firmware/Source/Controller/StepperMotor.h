// ----------------------------------------
// SM driver module
// ----------------------------------------

#ifndef __STEPPER_MOTOR_H
#define __STEPPER_MOTOR_H

// Include
#include "ZbBoard.h"
#include "LowLevel.h"

// Types
typedef struct __SM_Params
{
	Int16U NewPosition;
	Int16U SlowDownDistance;
	Int16U MaxSpeed;
	Int16U SlowSpeed;
	Int16U MinSpeed;
} SM_Params, *pSM_Params;

// Functions
//
// Main logic ISR call
void SM_TimerHandler();
// Connect alter handler for timer processing
void SM_ConnectAlterHandler(void *Handler);
// Steps Enable
void SM_Enable(Boolean State);
// Fill config per MCS TT: max=REG_POS_SPEED_MAX(12), slow=REG_POS_SPEED_MIN(10),
// slowDown=REG_SLOW_DOWN_DIST(13), min=REG_CLAMP_SPEED_MIN(16)
void SM_Config(pSM_Params Params, Int16U PositionMm, Boolean UseSlowdown);
// New position in mm, speed in mm/s
void SM_GoToPosition(pSM_Params Params);
Boolean SM_IsPositioningDone();
// Homing
void SM_Homing(Int16U HomingSpeed);
Boolean SM_IsHomingDone();
Boolean SM_IsSafetyEvent();
void SM_ResetZeroPoint();
void SM_RequestStop();

#endif // __STEPPER_MOTOR_H
