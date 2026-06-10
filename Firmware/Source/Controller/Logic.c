// ----------------------------------------
// MCS operational logic (FSM)
// ----------------------------------------

#include "Logic.h"
#include "Controller.h"
#include "Global.h"
#include "SysConfig.h"
#include "DataTable.h"
#include "DeviceObjectDictionary.h"
#include "Constraints.h"
#include "LowLevel.h"
#include "Measurement.h"
#include "SelfTest.h"
#include "StepperMotor.h"
#include "DS18B20.h"
#include "TRM101.h"

static Int16U LOGIC_ClampHeightMm = 0;
static Int64U LOGIC_WaitDeadline = 0;
static Int64U LOGIC_StateTimeout = 0;
static DeviceState LOGIC_LatchState = DS_None;
static DeviceSubState LOGIC_LatchSubState = DSS_None;

static void LOGIC_PrepareHoming();
static void LOGIC_StartSpiWait();
static Boolean LOGIC_WaitSpiInBit(Int8U Bit);
static Boolean LOGIC_OnSubStateEntry(DeviceState State, DeviceSubState SubState);
static Boolean LOGIC_ReadAdapterId();
static Boolean LOGIC_ValidateAdapter();
static void LOGIC_ProcessSelfTest();
static void LOGIC_MonitorCycleFaults();
static void LOGIC_PrepareClamping(Boolean Clamp);
static Int16U LOGIC_GetClampHeightMm();

static void LOGIC_PrepareClamping(Boolean Clamp)
{
	SM_Params Params;

	if(Clamp)
		SM_Config(&Params, LOGIC_GetClampHeightMm(), TRUE);
	else
		SM_Config(&Params, 0, FALSE);

	SM_GoToPosition(&Params);
}
// ----------------------------------------

static void LOGIC_PrepareHoming()
{
	SM_Params Params;

	Params.NewPosition = DataTable[REG_HOMING_OFFSET];
	Params.SlowDownDistance = 0;
	Params.MaxSpeed = DataTable[REG_HOMING_SPEED];
	Params.SlowSpeed = DataTable[REG_HOMING_SPEED];
	Params.MinSpeed = DataTable[REG_HOMING_SPEED];

	SM_GoToPosition(&Params);
}
// ----------------------------------------

static Int16U LOGIC_GetClampHeightMm()
{
	if(LOGIC_ClampHeightMm != 0)
		return LOGIC_ClampHeightMm;

	return DataTable[REG_ADAPTER_CLAMP_HEIGHT];
}
// ----------------------------------------

static Boolean LOGIC_OnSubStateEntry(DeviceState State, DeviceSubState SubState)
{
	if(LOGIC_LatchState != State || LOGIC_LatchSubState != SubState)
	{
		LOGIC_LatchState = State;
		LOGIC_LatchSubState = SubState;
		return TRUE;
	}

	return FALSE;
}
// ----------------------------------------

static void LOGIC_StartSpiWait()
{
	LOGIC_WaitDeadline = CONTROL_TimeCounter + SPI_WAIT_TIMEOUT;
}
// ----------------------------------------

static Boolean LOGIC_WaitSpiInBit(Int8U Bit)
{
	if(LL_SPI_GetInBit(Bit))
		return TRUE;

	if(CONTROL_TimeCounter > LOGIC_WaitDeadline)
		CONTROL_SwitchToFault(FAULT_SPI_TIMEOUT);

	return FALSE;
}
// ----------------------------------------

static Boolean LOGIC_ReadAdapterId()
{
	AdapterIdentifier Id;

	DS18B20_Init();
	if(!DS18B20_ReadIdentifier(&Id))
		return FALSE;

	LOGIC_ClampHeightMm = Id.ClampHeightMm;
	return TRUE;
}
// ----------------------------------------

static Boolean LOGIC_ValidateAdapter()
{
	DataTable[REG_ADAPTER_MISMATCH] = ADAPTER_MISMATCH_NONE;

	if(DataTable[REG_ADAPTER_ID] != DataTable[REG_DEV_CASE])
	{
		DataTable[REG_ADAPTER_MISMATCH] = ADAPTER_MISMATCH_CODE;
		DataTable[REG_ADAPTER_MATCH] = ADAPTER_MATCH_FAIL;
		return FALSE;
	}

	if(DataTable[REG_ADAPTER_MAX_CURRENT] < DataTable[REG_TEST_CURRENT])
	{
		DataTable[REG_ADAPTER_MISMATCH] = ADAPTER_MISMATCH_CURRENT;
		DataTable[REG_ADAPTER_MATCH] = ADAPTER_MATCH_FAIL;
		return FALSE;
	}

	if(DataTable[REG_ADAPTER_MAX_VOLTAGE] < DataTable[REG_TEST_VOLTAGE])
	{
		DataTable[REG_ADAPTER_MISMATCH] = ADAPTER_MISMATCH_VOLTAGE;
		DataTable[REG_ADAPTER_MATCH] = ADAPTER_MATCH_FAIL;
		return FALSE;
	}

	if(DataTable[REG_ADAPTER_CLAMP_HEIGHT] < ADAPTER_CLAMP_HEIGHT_MIN || DataTable[REG_ADAPTER_CLAMP_HEIGHT] > ADAPTER_CLAMP_HEIGHT_MAX)
		{
			DataTable[REG_ADAPTER_MISMATCH] = ADAPTER_MISMATCH_HEIGHT;
			DataTable[REG_ADAPTER_MATCH] = ADAPTER_MATCH_FAIL;
			return FALSE;
		}

	DataTable[REG_ADAPTER_MATCH] = ADAPTER_MATCH_OK;
	return TRUE;
}
// ----------------------------------------

Boolean LOGIC_IsCycleActive()
{
	switch(CONTROL_State)
	{
		case DS_Homing:
		case DS_Clamping:
		case DS_ClampingDone:
		case DS_ClampingRelease:
		case DS_AdapterHold:
		case DS_AdapterRelease:
			return TRUE;
		default:
			return FALSE;
	}
}
// ----------------------------------------

static void LOGIC_MonitorCycleFaults()
{
	if(!LOGIC_IsCycleActive())
		return;

	if(!LL_FilterSafetyCircuit(LL_IsSafetyS3Ok()) || !LL_FilterSafetyCircuit(LL_IsSafetyS5Ok()))
	{
		CONTROL_Halt();
		return;
	}

	if(!LL_SPI_IsCoil24VOk())
		CONTROL_Halt();

	if(DataTable[REG_ADAPTER_MATCH] == ADAPTER_MATCH_FAIL
			&& (CONTROL_State == DS_Clamping || CONTROL_State == DS_ClampingDone))
		CONTROL_SwitchToFault(FAULT_ADAPTER_MISMATCH);
}
// ----------------------------------------

static void LOGIC_ProcessSelfTest()
{
	static DeviceState SelfTestLatch = DS_None;

	if(CONTROL_State != DS_SelfTest)
	{
		SelfTestLatch = DS_None;
		return;
	}

	if(SelfTestLatch == DS_SelfTest)
		return;

	SelfTestLatch = DS_SelfTest;

	DataTable[REG_SELFTEST_RESULT] = SELFTEST_Run();

	if(DataTable[REG_SELFTEST_RESULT] == 0)
		CONTROL_SetDeviceState(DS_Ready, DSS_None);
	else
		CONTROL_SwitchToFault(FAULT_SELFTEST);
}
// ----------------------------------------

void LOGIC_Process()
{
	LOGIC_ProcessSelfTest();
	LOGIC_MonitorCycleFaults();

	if(CONTROL_State == DS_Fault || CONTROL_State == DS_Halt)
		return;

	switch(CONTROL_State)
	{
		case DS_Homing:
			switch(CONTROL_SubState)
			{
				case DSS_HomingSearchSensor:
					if(SM_IsHomingDone())
					{
						LOGIC_StateTimeout = CONTROL_TimeCounter + HOMING_PAUSE;
						CONTROL_SetDeviceState(CONTROL_State, DSS_HomingPause);
					}
					break;

				case DSS_HomingPause:
					if(CONTROL_TimeCounter > LOGIC_StateTimeout)
					{
						LOGIC_PrepareHoming();
						CONTROL_SetDeviceState(CONTROL_State, DSS_HomingMakeOffset);
					}
					break;

				case DSS_HomingMakeOffset:
					if(SM_IsPositioningDone())
					{
						SM_ResetZeroPoint();
						HomingDuration = CONTROL_TimeCounter - HomingDuration;
						ClampingDuration = ReleaseDuration = 0;
						RequestSaveToFlash = TRUE;
						CONTROL_SetDeviceState(DS_Ready, DSS_None);
					}
					break;

				default:
					break;
			}
			break;

		case DS_AdapterHold:
			switch(CONTROL_SubState)
			{
				case DSS_AdapterHold_CheckPressure:
					if(LOGIC_OnSubStateEntry(CONTROL_State, CONTROL_SubState))
						LOGIC_StateTimeout = CONTROL_TimeCounter + ADAPTER_HOLD_PRESSURE_TIMEOUT;

					if(MEAS_IsPressureOk())
						CONTROL_SetDeviceState(CONTROL_State, DSS_AdapterHold_ConnectAdapter);
					else if(CONTROL_TimeCounter > LOGIC_StateTimeout)
						CONTROL_SwitchToFault(FAULT_PRESSURE);
					break;

				case DSS_AdapterHold_ConnectAdapter:
					if(LOGIC_OnSubStateEntry(CONTROL_State, CONTROL_SubState))
					{
						LL_SPI_SetOutBit(SPI_OUT_ADAPTER, true);
						LL_SPI_FlushOut();
						LOGIC_StartSpiWait();
					}
					else if(LOGIC_WaitSpiInBit(SPI_IN_ADAPTER_HELD))
						CONTROL_SetDeviceState(CONTROL_State, DSS_AdapterHold_ConnectBus);
					break;

				case DSS_AdapterHold_ConnectBus:
					if(LOGIC_OnSubStateEntry(CONTROL_State, CONTROL_SubState))
					{
						LL_SPI_SetOutBit(SPI_OUT_BUS, true);
						LL_SPI_FlushOut();
						LOGIC_StartSpiWait();
					}
					else if(LOGIC_WaitSpiInBit(SPI_IN_BUS_HELD))
						CONTROL_SetDeviceState(CONTROL_State, DSS_AdapterHold_ReadId);
					break;

				case DSS_AdapterHold_ReadId:
					if(LOGIC_OnSubStateEntry(CONTROL_State, CONTROL_SubState))
					{
						if(LOGIC_ReadAdapterId())
						{
							if(LOGIC_ValidateAdapter())
								CONTROL_SetDeviceState(CONTROL_State, DSS_AdapterHold_Done);
							else
								CONTROL_SwitchToFault(FAULT_ADAPTER_MISMATCH);
						}
						else
							CONTROL_SwitchToFault(FAULT_ADAPTER_MISMATCH);
					}
					break;

				case DSS_AdapterHold_Done:
					CONTROL_SetDeviceState(DS_Ready, DSS_None);
					break;

				default:
					break;
			}
			break;

		case DS_Clamping:
			switch(CONTROL_SubState)
			{
				case DSS_None:
					LOGIC_PrepareClamping(TRUE);
					CONTROL_SetDeviceState(CONTROL_State, DSS_ClampingOperating);
					break;

				case DSS_ClampingOperating:
					if(SM_IsPositioningDone())
					{
						ClampingDuration = CONTROL_TimeCounter - ClampingDuration;
						HomingDuration = ReleaseDuration = 0;
						RequestSaveToFlash = TRUE;
						CONTROL_SetDeviceState(DS_ClampingDone, DSS_None);
					}
					break;

				default:
					break;
			}
			break;

		case DS_ClampingRelease:
			switch(CONTROL_SubState)
			{
				case DSS_None:
					LOGIC_PrepareClamping(FALSE);
					CONTROL_SetDeviceState(CONTROL_State, DSS_ClampingReleaseOperating);
					break;

				case DSS_ClampingReleaseOperating:
					if(SM_IsPositioningDone())
					{
						ReleaseDuration = CONTROL_TimeCounter - ReleaseDuration;
						HomingDuration = ClampingDuration = 0;
						RequestSaveToFlash = TRUE;
						CONTROL_SetDeviceState(DS_Ready, DSS_None);
					}
					break;

				default:
					break;
			}
			break;

		case DS_AdapterRelease:
			switch(CONTROL_SubState)
			{
				case DSS_AdapterRelease_Bus:
					if(LOGIC_OnSubStateEntry(CONTROL_State, CONTROL_SubState))
					{
						LL_SPI_SetOutBit(SPI_OUT_BUS, false);
						LL_SPI_FlushOut();
						LOGIC_StartSpiWait();
					}
					else if(LOGIC_WaitSpiInBit(SPI_IN_BUS_RELEASED))
						CONTROL_SetDeviceState(CONTROL_State, DSS_AdapterRelease_Adapter);
					break;

				case DSS_AdapterRelease_Adapter:
					if(LOGIC_OnSubStateEntry(CONTROL_State, CONTROL_SubState))
					{
						LL_SPI_SetOutBit(SPI_OUT_ADAPTER, false);
						LL_SPI_FlushOut();
						LOGIC_StartSpiWait();
					}
					else if(LOGIC_WaitSpiInBit(SPI_IN_ADAPTER_RELEASED))
						CONTROL_SetDeviceState(CONTROL_State, DSS_AdapterRelease_HeatingOff);
					break;

				case DSS_AdapterRelease_HeatingOff:
					if(LOGIC_OnSubStateEntry(CONTROL_State, CONTROL_SubState))
					{
						TRMError error = TRME_None;

						if(DataTable[REG_USE_HEATING])
							TRM_Stop(TRM_CH1_ADDR, &error);

						if(error != TRME_None)
						{
							DataTable[REG_TRM_ERROR] = error;
							CONTROL_SwitchToFault(FAULT_TRM);
							break;
						}

						LL_SPI_SetOutBit(SPI_OUT_FAN1, false);
						LL_SPI_SetOutBit(SPI_OUT_FAN2, false);
						LL_SPI_FlushOut();
						HeatingActive = FALSE;

						LOGIC_ClampHeightMm = 0;
						DataTable[REG_ADAPTER_MATCH] = ADAPTER_MATCH_NONE;
						CONTROL_SetDeviceState(CONTROL_State, DSS_AdapterRelease_Done);
					}
					break;

				case DSS_AdapterRelease_Done:
					CONTROL_SetDeviceState(DS_Ready, DSS_None);
					break;

				default:
					break;
			}
			break;

		default:
			break;
	}
}
// ----------------------------------------
