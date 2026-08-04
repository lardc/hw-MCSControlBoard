// Header
#include "Logic.h"
// Includes
#include "Controller.h"
#include "SysConfig.h"
#include "DataTable.h"
#include "DeviceObjectDictionary.h"
#include "LowLevel.h"
#include "Measurement.h"
#include "SelfTest.h"
#include "StepperMotor.h"
#include "TRM101.h"
#include "MemLabel.h"

static void LOGIC_AdapterIdPublish(pAdapterIdentifier Id);
static Int16U LOGIC_ClampHeightMm = 0;
static Int64U LOGIC_WaitDeadline = 0;
static Int64U LOGIC_StateTimeout = 0;
static DeviceState LOGIC_LatchState = DS_None;
static DeviceSubState LOGIC_LatchSubState = DSS_None;
static Boolean IsHolding = false;
static Boolean LOGIC_FaultSpiAfterRelease = FALSE;

static void LOGIC_PrepareHoming();
static Boolean LOGIC_WaitSpiInBit(Int8U Bit);
static Boolean LOGIC_OnSubStateEntry(DeviceState State, DeviceSubState SubState);
static Boolean LOGIC_ReadAdapterId();
static void LOGIC_AbortHoldToRelease();
static void LOGIC_ProcessSelfTest();
static void LOGIC_MonitorCycleFaults();
static void LOGIC_PrepareClamping(Boolean Clamp);
static Int16U LOGIC_GetClampHeightMm();

static void LOGIC_PrepareClamping(Boolean Clamp)
{
	SM_Params Params;

	if(Clamp)
		SM_Config(&Params, LOGIC_GetClampHeightMm());
	else
		SM_Config(&Params, 0);

	SM_GoToPosition(&Params);
}
// ----------------------------------------

static void LOGIC_PrepareHoming()
{
	SM_Params Params;

	Params.NewPosition = DataTable[REG_HOMING_OFFSET];
	Params.MaxSpeed = DataTable[REG_HOMING_SPEED];
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

static Boolean LOGIC_WaitSpiInBit(Int8U Bit)
{
	if(LL_SPI_GetInBit(Bit))
		return TRUE;

	if(CONTROL_TimeCounter > LOGIC_WaitDeadline)
	{
		if(CONTROL_State == DS_AdapterHold)
		{
			LOGIC_FaultSpiAfterRelease = TRUE;
			LOGIC_AbortHoldToRelease();
		}
		else
			CONTROL_SwitchToFault(DF_SPI_TIMEOUT);
	}

	return FALSE;
}
// ----------------------------------------

static void LOGIC_AbortHoldToRelease()
{
	IsHolding = false;
	CONTROL_SetDeviceState(DS_AdapterRelease, DSS_AdapterRelease_Bus);
}
// ----------------------------------------

static Boolean LOGIC_ReadAdapterId()
{
	AdapterIdentifier Id;

	LOGIC_AdapterIdInit();
	if(!LOGIC_AdapterIdRead(&Id))
		return FALSE;

	LOGIC_ClampHeightMm = Id.ClampHeightMm;
	return TRUE;
}
// ----------------------------------------

void LOGIC_AdapterIdInit()
{
	DS2431_Init();
}
// ----------------------------------------

static void LOGIC_AdapterIdPublish(pAdapterIdentifier Id)
{
	DataTable[REG_ADAPTER_ID] = Id->Code;
	DataTable[REG_ADAPTER_CLAMP_HEIGHT] = Id->ClampHeightMm;
	DataTable[REG_ADAPTER_MAX_CURRENT] = Id->MaxCurrent;
	DataTable[REG_ADAPTER_MAX_VOLTAGE] = Id->MaxVoltage;
	DataTable[REG_ADAPTER_SERIAL] = Id->Serial;
}
// ----------------------------------------

Boolean LOGIC_AdapterIdRead(pAdapterIdentifier Id)
{
	MemLabelEntry Labels[MEM_LABEL_MAX_LABELS];
	Int8U LabelCount;
	Int8U FilledCount = 0;

	if(DS2431_GetDeviceCount() == 0)
	{
		CONTROL_FinishedWithProblem(PROBLEM_OW_NO_DEVICE);
		return FALSE;
	}

	LabelCount = MemLabel_Read(0, Labels, MEM_LABEL_MAX_LABELS);
	if(LabelCount == 0 && DS2431_GetLastError() != DS2431_OK)
	{
		CONTROL_FinishedWithProblem(CONTROL_ProblemFromDs2431());
		return FALSE;
	}

	for(Int8U i = 0; i < LabelCount; i++)
	{
		switch(Labels[i].Type)
		{
			case ML_AdapterCode:
				Id->Code = Labels[i].Value;
				FilledCount++;
				break;
			case ML_ClampHeight:
				Id->ClampHeightMm = Labels[i].Value;
				FilledCount++;
				break;
			case ML_MaxCurrent:
				Id->MaxCurrent = Labels[i].Value;
				FilledCount++;
				break;
			case ML_MaxVoltage:
				Id->MaxVoltage = Labels[i].Value;
				FilledCount++;
				break;
			case ML_Serial:
				Id->Serial = Labels[i].Value;
				FilledCount++;
				break;
			default:
				break;
		}
	}

	if(FilledCount < 5)
	{
		CONTROL_FinishedWithProblem(PROBLEM_MISSING_LABEL);
		return FALSE;
	}

	LOGIC_AdapterIdPublish(Id);
	return TRUE;
}
// ----------------------------------------

Boolean LOGIC_AdapterIdWrite(pAdapterIdentifier Id)
{
	// TODO: запись идентификатора в отдельную микросхему
	LOGIC_AdapterIdPublish(Id);
	return TRUE;
}
// ----------------------------------------

Boolean LOGIC_ValidateAdapter()
{
	DataTable[REG_ADAPTER_MISMATCH] = ADAPTER_MISMATCH_NONE;
	DataTable[REG_ADAPTER_MATCH] = false;

	if(DataTable[REG_ADAPTER_ID] != DataTable[REG_DEV_CASE])
	{
		DataTable[REG_ADAPTER_MISMATCH] = ADAPTER_MISMATCH_CODE;
		return FALSE;
	}

	if(DataTable[REG_ADAPTER_MAX_CURRENT] < DataTable[REG_TEST_CURRENT])
	{
		DataTable[REG_ADAPTER_MISMATCH] = ADAPTER_MISMATCH_CURRENT;
		return FALSE;
	}

	if(DataTable[REG_ADAPTER_MAX_VOLTAGE] < DataTable[REG_TEST_VOLTAGE])
	{
		DataTable[REG_ADAPTER_MISMATCH] = ADAPTER_MISMATCH_VOLTAGE;
		return FALSE;
	}

	if(DataTable[REG_ADAPTER_CLAMP_HEIGHT] < ADAPTER_CLAMP_HEIGHT_MIN || DataTable[REG_ADAPTER_CLAMP_HEIGHT] > ADAPTER_CLAMP_HEIGHT_MAX)
	{
		DataTable[REG_ADAPTER_MISMATCH] = ADAPTER_MISMATCH_HEIGHT;
		return FALSE;
	}

	DataTable[REG_ADAPTER_MATCH] = true;
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
		CONTROL_SwitchToFault(DF_SELFTEST);
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
					{
						IsHolding = false;
						DataTable[REG_ADAPTER_MATCH] = false;
						LOGIC_FaultSpiAfterRelease = FALSE;
						LOGIC_StateTimeout = CONTROL_TimeCounter + ADAPTER_HOLD_PRESSURE_TIMEOUT;
					}

					if(MEAS_IsPressureOk())
						CONTROL_SetDeviceState(CONTROL_State, DSS_AdapterHold_ConnectAdapter);
					else if(CONTROL_TimeCounter > LOGIC_StateTimeout)
						CONTROL_SwitchToFault(DF_PRESSURE);
					break;

				case DSS_AdapterHold_ConnectAdapter:
					if(LOGIC_OnSubStateEntry(CONTROL_State, CONTROL_SubState))
					{
						LL_SPI_SetOutBit(SPI_OUT_ADAPTER, true);
						LL_SPI_FlushOut();
						LOGIC_WaitDeadline = CONTROL_TimeCounter + SPI_WAIT_TIMEOUT;
					}
					else if(LOGIC_WaitSpiInBit(SPI_IN_ADAPTER_HELD))
						CONTROL_SetDeviceState(CONTROL_State, DSS_AdapterHold_ConnectBus);
					break;

				case DSS_AdapterHold_ConnectBus:
					if(LOGIC_OnSubStateEntry(CONTROL_State, CONTROL_SubState))
					{
						LL_SPI_SetOutBit(SPI_OUT_BUS, true);
						LL_SPI_FlushOut();
						LOGIC_WaitDeadline = CONTROL_TimeCounter + SPI_WAIT_TIMEOUT;
					}
					else if(LOGIC_WaitSpiInBit(SPI_IN_BUS_HELD))
						CONTROL_SetDeviceState(CONTROL_State, DSS_AdapterHold_ReadId);
					break;

				case DSS_AdapterHold_ReadId:
					if(LOGIC_OnSubStateEntry(CONTROL_State, CONTROL_SubState))
					{
						if(LOGIC_ReadAdapterId())
							CONTROL_SetDeviceState(CONTROL_State, DSS_AdapterHold_Done);
						else
							LOGIC_AbortHoldToRelease();
					}
					break;

				case DSS_AdapterHold_Done:
					CONTROL_SetDeviceState(DS_Ready, DSS_None);
					IsHolding = true;
					DataTable[REG_OP_RESULT] = OPRESULT_OK;
					break;

				default:
					break;
			}
			break;

		case DS_Clamping:
			switch(CONTROL_SubState)
			{
				case DSS_None:
					if(DataTable[REG_ADAPTER_MATCH] && IsHolding )
					{
						LOGIC_PrepareClamping(TRUE);
						CONTROL_SetDeviceState(CONTROL_State, DSS_ClampingOperating);
					}
					else
					{
						CONTROL_FinishedWithProblem(PROBLEM_NO_HOLD_OR_MISMATCH);
						CONTROL_SetDeviceState(DS_Ready, DSS_None);
					}
					break;

				case DSS_ClampingOperating:
					if(SM_IsPositioningDone())
					{
						ClampingDuration = CONTROL_TimeCounter - ClampingDuration;
						HomingDuration = ReleaseDuration = 0;
						RequestSaveToFlash = TRUE;
						DataTable[REG_OP_RESULT] = OPRESULT_OK;
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
						LOGIC_WaitDeadline = CONTROL_TimeCounter + SPI_WAIT_TIMEOUT;
					}
					else if(LOGIC_WaitSpiInBit(SPI_IN_BUS_RELEASED))
						CONTROL_SetDeviceState(CONTROL_State, DSS_AdapterRelease_Adapter);
					break;

				case DSS_AdapterRelease_Adapter:
					if(LOGIC_OnSubStateEntry(CONTROL_State, CONTROL_SubState))
					{
						LL_SPI_SetOutBit(SPI_OUT_ADAPTER, false);
						LL_SPI_FlushOut();
						LOGIC_WaitDeadline = CONTROL_TimeCounter + SPI_WAIT_TIMEOUT;
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
							CONTROL_SwitchToFault(DF_TRM);
							break;
						}

						LL_SPI_SetOutBit(SPI_OUT_FAN1, false);
						LL_SPI_SetOutBit(SPI_OUT_FAN2, false);
						LL_SPI_FlushOut();
						HeatingActive = FALSE;

						LOGIC_ClampHeightMm = 0;
						DataTable[REG_ADAPTER_MATCH] = false;
						CONTROL_SetDeviceState(CONTROL_State, DSS_AdapterRelease_Done);
					}
					break;

				case DSS_AdapterRelease_Done:
					IsHolding = false;
					if(LOGIC_FaultSpiAfterRelease)
					{
						LOGIC_FaultSpiAfterRelease = FALSE;
						CONTROL_SwitchToFault(DF_SPI_TIMEOUT);
					}
					else
					{
						if(DataTable[REG_PROBLEM] == PROBLEM_NONE)
							DataTable[REG_OP_RESULT] = OPRESULT_OK;
						CONTROL_SetDeviceState(DS_Ready, DSS_None);
					}
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
