#ifndef __CONTROLLER_H
#define __CONTROLLER_H

// Include
//
#include "stdinc.h"
#include "Global.h"

// Types
//
typedef enum __DeviceState
{
	DS_None					= 0,
	DS_Fault				= 1,
	DS_Disabled 			= 2,
	DS_Ready 				= 3,
	DS_Halt					= 4,
	DS_Clamping				= 7,
	DS_ClampingDone			= 8,
	DS_ClampingRelease 		= 10
} DeviceState;

typedef enum __DeviceSubState
{
	SS_None					= 0,
	SS_BlockAdapters,
	SS_BlockDelay,
	SS_ClampDelay,
	SS_StartRelease,
	SS_ReleaseDelay,
	SS_UnblockDelay
} DeviceSubState;

// Variables
//
extern volatile Int64U CONTROL_TimeCounter;
extern volatile DeviceState CONTROL_State;
extern volatile DeviceSubState CONTROL_SubState;
extern Int64U CONTROL_LEDTimeout;

// Functions
//
void CONTROL_Init();
void CONTROL_Idle();
void CONTROL_DelayMs(uint32_t Delay);
void CONTROL_SwitchToFault(Int16U Reason);
void CONTROL_SetDeviceState(DeviceState NewState, DeviceSubState NewSubState);
void CONTROL_Halt();

#endif // __CONTROLLER_H
