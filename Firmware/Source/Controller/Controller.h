#ifndef __CONTROLLER_H
#define __CONTROLLER_H

#include "OWENProtocol.h"
#include "stdinc.h"
#include "Global.h"
#include "DeviceObjectDictionary.h"
#include "Constraints.h"

// Types
//
typedef enum __DeviceState
{
	DS_None	= 0,
	DS_Fault = 1,

	DS_Ready = 3,
	DS_Halt = 4,
	DS_Homing = 5,

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
	DSS_HomingPauseBeforeOffset = 11,
	DSS_HomingMakeOffset = 12,
	DSS_HomingSearchSensorWait = 13,

	DSS_ClampingOperating = 31,
	DSS_ClampingReleaseOperating = 40,

	DSS_AdapterHold_CheckPressure = 50,
	DSS_AdapterHold_CheckPressureWait = 51,
	DSS_AdapterHold_ConnectAdapter = 52,
	DSS_AdapterHold_ConnectAdapterWait = 53,
	DSS_AdapterHold_ConnectBus = 54,
	DSS_AdapterHold_ConnectBusWait = 55,
	DSS_AdapterHold_ReadId = 56,
	DSS_AdapterHold_Done = 57,

	DSS_AdapterRelease_Bus = 60,
	DSS_AdapterRelease_BusWait = 61,
	DSS_AdapterRelease_Adapter = 62,
	DSS_AdapterRelease_AdapterWait = 63,
	DSS_AdapterRelease_HeatingOff = 64,
	DSS_AdapterRelease_Done = 65
} DeviceSubState;

// Variables
extern volatile Int64U CONTROL_TimeCounter;
extern volatile DeviceState CONTROL_State;
extern volatile DeviceSubState CONTROL_SubState;
extern volatile Int32U HomingDuration;
extern volatile Int32U ClampingDuration;
extern volatile Int32U ReleaseDuration;
extern volatile Boolean RequestSaveToFlash;
extern volatile Int16U CONTROL_ExtInfoCounter;
extern volatile Int16U CONTROL_ValuesCounter;
extern Boolean HeatingActive;

extern float CONTROL_MotorMovement[VALUES_x_SIZE];
extern float CONTROL_MotorSpeed[VALUES_x_SIZE];
extern volatile float CONTROL_ExtInfoData[VALUES_EXT_INFO_SIZE];

// Functions
void CONTROL_Init();
void CONTROL_Idle();
void CONTROL_UpdatePressureOK();
void CONTROL_SetDeviceState(DeviceState NewState, DeviceSubState NewSubState);
void CONTROL_SwitchToFault(Int16U Reason);
void CONTROL_FinishedWithProblem(Int16U Problem);
Int16U CONTROL_ProblemFromDs2431();
void CONTROL_Halt();

#endif // __CONTROLLER_H
