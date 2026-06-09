// -----------------------------------------
// Logic controller
// ----------------------------------------

#ifndef __CONTROLLER_H
#define __CONTROLLER_H

#include "OWENProtocol.h"
#include "stdinc.h"
#include "Global.h"
#include "DeviceObjectDictionary.h"
#include "Constraints.h"
#include "StepperMotor.h"

// Types
//
typedef enum __DeviceState
{
	DS_None	= 0,
	DS_Fault = 1,
	DS_Disabled = 2,
	DS_Ready = 3,
	DS_Halt = 4,
	DS_Homing = 5,
	// 6 — reserved (legacy DS_Position)
	DS_Clamping = 7,
	DS_ClampingDone = 8,
	DS_SelfTest = 9,
	DS_ClampingRelease = 10,
	DS_AdapterHold = 11,
	DS_AdapterRelease = 12
} DeviceState;

typedef enum __DeviceSubState
{
	DSS_None = 0,

	DSS_HomingSearchSensor = 10,
	DSS_HomingPause = 11,
	DSS_HomingMakeOffset = 12,

	DSS_ClampingOperating = 31,
	DSS_ClampingReleaseOperating = 40,

	DSS_AdapterHold_CheckPressure = 50,
	DSS_AdapterHold_ConnectAdapter = 51,
	DSS_AdapterHold_ConnectBus = 52,
	DSS_AdapterHold_ReadId = 53,
	DSS_AdapterHold_Done = 54,

	DSS_AdapterRelease_Bus = 60,
	DSS_AdapterRelease_Adapter = 61,
	DSS_AdapterRelease_HeatingOff = 62,
	DSS_AdapterRelease_Done = 63,

	DSS_Heating_Start = 70,
	DSS_Heating_Operating = 71,
	DSS_Heating_Done = 72
} DeviceSubState;

// Variables
extern volatile Int64U CONTROL_TimeCounter;
extern volatile DeviceState CONTROL_State;
extern volatile DeviceSubState CONTROL_SubState;
extern volatile Int32U HomingDuration;
extern volatile Int32U ClampingDuration;
extern volatile Int32U ReleaseDuration;
extern volatile Boolean RequestSaveToFlash;
extern volatile Int16U CONTROL_BootLoaderRequest;

// Functions
void CONTROL_Init();
void CONTROL_Idle();
void CONTROL_UpdatePressureOK();
void CONTROL_SetDeviceState(DeviceState NewState, DeviceSubState NewSubState);
void CONTROL_SwitchToFault(Int16U Reason);
void CONTROL_Halt();

#endif // __CONTROLLER_H
