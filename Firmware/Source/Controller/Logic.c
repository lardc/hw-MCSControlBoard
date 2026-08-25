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
#include "TRM10.h"
#include "MemLabel.h"

// Variables
static void LOGIC_AdapterIdPublish(pAdapterIdentifier Id);
static Int16U LOGIC_ClampHeightMm = 0;
static Int64U LOGIC_WaitDeadline = 0;
static Int64U LOGIC_StateTimeout = 0;
static Boolean IsHolding = false;
static Boolean LOGIC_FaultSpiAfterRelease = FALSE;
AdapterIdentifier LOGIC_Id = {0};

// Forward functions
static Boolean LOGIC_PrepareClamping(Boolean Clamp);
static Boolean LOGIC_PrepareHoming();
static Boolean LOGIC_WaitSpiInBit(Int8U Bit);
static Boolean LOGIC_ReadAdapterId();
static void LOGIC_AbortHoldToRelease();
static void LOGIC_MonitorCycleFaults();
static Int16U LOGIC_GetClampHeightMm();

// Functions
//
static Boolean LOGIC_PrepareClamping(Boolean Clamp)
{
	SM_Params Params;

	if(Clamp)
		SM_Config(&Params, LOGIC_GetClampHeightMm());
	else
		SM_Config(&Params, 0);

	return SM_GoToPosition(&Params);
}
// ----------------------------------------

static Boolean LOGIC_PrepareHoming()
{
	SM_Params Params;

	Params.NewPosition = DataTable[REG_HOMING_OFFSET];
	Params.MaxSpeed = DataTable[REG_HOMING_SPEED];
	Params.MinSpeed = DataTable[REG_HOMING_SPEED];

	return SM_GoToPosition(&Params);
}
// ----------------------------------------

static Int16U LOGIC_GetClampHeightMm()
{
	if(LOGIC_ClampHeightMm != 0)
		return LOGIC_ClampHeightMm;

	return DataTable[REG_ADAPTER_CLAMP_HEIGHT];
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
	LOGIC_AdapterIdInit();
	if(!LOGIC_AdapterIdRead(&LOGIC_Id))
		return FALSE;

	LOGIC_ClampHeightMm = LOGIC_Id.ClampHeightMm;
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
	DataTable[REG_ADAPTER_CODE] = Id->Code;
	DataTable[REG_ADAPTER_CLAMP_HEIGHT] = Id->ClampHeightMm;
	DataTable[REG_ADAPTER_MAX_CURRENT] = Id->MaxCurrent;
	DataTable[REG_ADAPTER_MAX_VOLTAGE] = Id->MaxVoltage;
	DataTable[REG_ADAPTER_SERIAL] = Id->Serial;
	DataTable[REG_ADAPTER_VERSION] = Id->Version;
}
// ----------------------------------------

Boolean LOGIC_AdapterIdRead(pAdapterIdentifier Id)
{
	MemLabelEntry Labels[MEM_LABEL_MAX_LABELS];
	Int8U LabelCount;
	Int8U FilledCount = 0;

	Id->Cached = FALSE;

	DataTable[REG_ADAPTER_CODE] = 0;
	DataTable[REG_ADAPTER_VERSION] = 0;
	DataTable[REG_ADAPTER_CLAMP_HEIGHT] = 0;
	DataTable[REG_ADAPTER_MAX_CURRENT] = 0;
	DataTable[REG_ADAPTER_MAX_VOLTAGE] = 0;
	DataTable[REG_ADAPTER_SERIAL] = 0;

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
			case ML_SerialNumber:
				Id->Serial = Labels[i].Value;
				FilledCount++;
				break;
			case ML_Version:
				Id->Version = Labels[i].Value;
				FilledCount++;
				break;
			case ML_Device:
				if(Labels[i].Value != 0)
				{
					CONTROL_FinishedWithProblem(PROBLEM_INCORRECT_DEVICE);
					return FALSE;
				}
				break;
			default:
				break;
		}
	}

	if(FilledCount < 6)
	{
		CONTROL_FinishedWithProblem(PROBLEM_MISSING_LABEL);
		return FALSE;
	}

	LOGIC_AdapterIdPublish(Id);
	Id->Cached = TRUE;
	LOGIC_Id = *Id;
	return TRUE;
}
// ----------------------------------------

Boolean LOGIC_AdapterIdWrite(pAdapterIdentifier Id)
{
	if(DS2431_GetDeviceCount() == 0)
	{
		CONTROL_FinishedWithProblem(PROBLEM_OW_NO_DEVICE);
		return FALSE;
	}

	if(!MemLabel_EraseAll(0))
	{
		CONTROL_FinishedWithProblem(CONTROL_ProblemFromDs2431());
		return false;
	}

	MemLabelEntry Labels[] =
	{
		{.Type = ML_AdapterCode,	.Value = Id->Code },
		{.Type = ML_ClampHeight,	.Value = Id->ClampHeightMm },
		{.Type = ML_MaxCurrent,		.Value = Id->MaxCurrent },
		{.Type = ML_MaxVoltage,		.Value = Id->MaxVoltage },
		{.Type = ML_SerialNumber,	.Value = Id->Serial },
		{.Type = ML_Version,		.Value = Id->Version },
		{.Type = ML_Device,			.Value = Id->Device },
	};

	const Int8U Count = sizeof(Labels) / sizeof(Labels[0]);

	if(!MemLabel_AddArray(0, Labels, Count))
	{
		CONTROL_FinishedWithProblem(CONTROL_ProblemFromDs2431());
		return false;
	}

	return true;
}
// ----------------------------------------

Boolean LOGIC_ValidateAdapter(pAdapterIdentifier Id)
{
	DataTable[REG_ADAPTER_MISMATCH_CODE] = ADAPTER_MISMATCH_NONE;
	DataTable[REG_ADAPTER_MATCH] = false;

	if(Id->Code != DataTable[REG_DEV_CASE])
	{
		DataTable[REG_ADAPTER_MISMATCH_CODE] = ADAPTER_MISMATCH_CODE;
		return FALSE;
	}

	if(Id->MaxCurrent < DataTable[REG_TEST_CURRENT])
	{
		DataTable[REG_ADAPTER_MISMATCH_CODE] = ADAPTER_MISMATCH_CURRENT;
		return FALSE;
	}

	if(Id->MaxVoltage < DataTable[REG_TEST_VOLTAGE])
	{
		DataTable[REG_ADAPTER_MISMATCH_CODE] = ADAPTER_MISMATCH_VOLTAGE;
		return FALSE;
	}

	if(Id->ClampHeightMm < ADAPTER_CLAMP_HEIGHT_MIN || Id->ClampHeightMm > ADAPTER_CLAMP_HEIGHT_MAX)
	{
		DataTable[REG_ADAPTER_MISMATCH_CODE] = ADAPTER_MISMATCH_HEIGHT;
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
		case DS_Movement:
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

	DataTable[REG_SENSOR_S3] = LL_FilterSafetyCircuit(SC_CH_S3, LL_IsSafetyS3Ok());
	DataTable[REG_SENSOR_S5] = LL_FilterSafetyCircuit(SC_CH_S5, LL_IsSafetyS5Ok());
	if(!DataTable[REG_SENSOR_S3] || !DataTable[REG_SENSOR_S5])
	{
		CONTROL_FinishedWithProblem(PROBLEM_SAFETY);
		CONTROL_Halt();
		return;
	}

	if(!LL_SPI_IsCoil24VOk())
	{
		CONTROL_FinishedWithProblem(PROBLEM_SAFETY);
		CONTROL_Halt();
	}
}
// ----------------------------------------

void LOGIC_Process()
{
	if(DataTable[REG_USE_SAFETY])
		LOGIC_MonitorCycleFaults();
	
	if(CONTROL_State == DS_Fault || CONTROL_State == DS_Halt)
		return;

	switch(CONTROL_State)
	{
		case DS_SelfTest:
			{
				Int16U Result = SELFTEST_Run();

				if(Result == SELFTEST_IN_PROGRESS)
					break;

				DataTable[REG_SELFTEST_RESULT] = Result;
				if(Result == 0)
					CONTROL_SetDeviceState(DS_Ready, DSS_None);
				else
					CONTROL_SwitchToFault(DF_SELFTEST);
			}
			break;

		case DS_Movement:
			switch(CONTROL_SubState)
			{
				case DSS_MovementStart:
					{
						SM_Params Params;
						SM_Config(&Params, (Int16U)DataTable[REG_CUSTOM_POS]);
						LOGIC_StateTimeout = CONTROL_TimeCounter + MOVEMENT_TIMEOUT;
						if(!SM_GoToPosition(&Params))
						{
							CONTROL_FinishedWithProblem(PROBLEM_MOTOR_START);
							CONTROL_SetDeviceState(DS_Ready, DSS_None);
						}
						else
							 CONTROL_SetDeviceState(CONTROL_State, DSS_MovementEnd);
					}
					break;

				case DSS_MovementEnd:
					if(!SM_IsBusy())
					{
						DataTable[REG_OP_RESULT] = OPRESULT_OK;
						CONTROL_SetDeviceState(DS_Ready, DSS_None);
					}
					else if(CONTROL_TimeCounter > LOGIC_StateTimeout)
					{
						SM_RequestStop();
						CONTROL_SetDeviceState(DS_Ready, DSS_None);
						CONTROL_FinishedWithProblem(PROBLEM_MOVEMENT_TIMEOUT);
					}
					break;
				default:
					break;
			}
			break;

		case DS_Homing:
			switch(CONTROL_SubState)
			{
				case DSS_HomingSearchSensor:
					if(!SM_Homing())
					{
						CONTROL_FinishedWithProblem(PROBLEM_MOTOR_START);
						CONTROL_SetDeviceState(DS_Ready, DSS_None);
					}
					else
					{
						LOGIC_StateTimeout = CONTROL_TimeCounter + HOMING_TIMEOUT;
						CONTROL_SetDeviceState(CONTROL_State, DSS_HomingSearchSensorWait);
					}
					break;

				case DSS_HomingSearchSensorWait:
					if(SM_IsHomingDone())
					{
						LOGIC_StateTimeout = CONTROL_TimeCounter + HOMING_PAUSE;
						CONTROL_SetDeviceState(CONTROL_State, DSS_HomingPauseBeforeOffset);
					}
					else if(CONTROL_TimeCounter > LOGIC_StateTimeout)
					{
						SM_RequestStop();
						CONTROL_SwitchToFault(DF_HOMING_TIMEOUT);
					}
					break;

				case DSS_HomingPauseBeforeOffset:
					if(CONTROL_TimeCounter > LOGIC_StateTimeout)
					{
						if(LOGIC_PrepareHoming())
							CONTROL_SetDeviceState(CONTROL_State, DSS_HomingMakeOffset);
						else
						{
							SM_RequestStop();
							CONTROL_FinishedWithProblem(PROBLEM_MOTOR_START);
							CONTROL_SetDeviceState(DS_Ready, DSS_None);
						}
					}
					break;

				case DSS_HomingMakeOffset:
					if(SM_IsPositioningDone())
					{
						SM_ResetZeroPoint();
						HomingDuration = CONTROL_TimeCounter - HomingDuration;
						ClampingDuration = ReleaseDuration = 0;
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
					IsHolding = false;
					LOGIC_Id.Cached = FALSE;
					DataTable[REG_ADAPTER_MATCH] = false;
					LOGIC_FaultSpiAfterRelease = FALSE;
					LOGIC_StateTimeout = CONTROL_TimeCounter + ADAPTER_HOLD_PRESSURE_TIMEOUT;
					CONTROL_SetDeviceState(CONTROL_State, DSS_AdapterHold_CheckPressureWait);
					break;

				case DSS_AdapterHold_CheckPressureWait:
					if(MEAS_IsPressureOk())
						CONTROL_SetDeviceState(CONTROL_State, DSS_AdapterHold_ConnectAdapter);
					else if(CONTROL_TimeCounter > LOGIC_StateTimeout)
						CONTROL_SwitchToFault(DF_PRESSURE);
					break;

				case DSS_AdapterHold_ConnectAdapter:
					LL_SPI_SetOutBit(SPI_OUT_ADAPTER, true);
					LL_SPI_FlushOut();
					LOGIC_WaitDeadline = CONTROL_TimeCounter + SPI_WAIT_TIMEOUT;
					CONTROL_SetDeviceState(CONTROL_State, DSS_AdapterHold_ConnectAdapterWait);
					break;

				case DSS_AdapterHold_ConnectAdapterWait:
					if(LOGIC_WaitSpiInBit(SPI_IN_ADAPTER_HELD))
						CONTROL_SetDeviceState(CONTROL_State, DSS_AdapterHold_ConnectBus);
					break;

				case DSS_AdapterHold_ConnectBus:
					LL_SPI_SetOutBit(SPI_OUT_BUS, true);
					LL_SPI_FlushOut();
					LOGIC_WaitDeadline = CONTROL_TimeCounter + SPI_WAIT_TIMEOUT;
					CONTROL_SetDeviceState(CONTROL_State, DSS_AdapterHold_ConnectBusWait);
					break;

				case DSS_AdapterHold_ConnectBusWait:
					if(LOGIC_WaitSpiInBit(SPI_IN_BUS_HELD))
						CONTROL_SetDeviceState(CONTROL_State, DSS_AdapterHold_ReadId);
					break;

				case DSS_AdapterHold_ReadId:
					if(LOGIC_ReadAdapterId())
						CONTROL_SetDeviceState(CONTROL_State, DSS_AdapterHold_Done);
					else
						LOGIC_AbortHoldToRelease();
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
						if(LOGIC_PrepareClamping(TRUE))
							CONTROL_SetDeviceState(CONTROL_State, DSS_ClampingOperating);
						else
						{
							CONTROL_FinishedWithProblem(PROBLEM_MOTOR_START);
							CONTROL_SetDeviceState(DS_Ready, DSS_None);
						}
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
						if(DataTable[REG_CNT_ACTIVE])
							CycleCounters[2]++;
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
					if(LOGIC_PrepareClamping(FALSE))
						CONTROL_SetDeviceState(CONTROL_State, DSS_ClampingReleaseOperating);
					else
					{
						CONTROL_FinishedWithProblem(PROBLEM_MOTOR_START);
						CONTROL_SetDeviceState(DS_Ready, DSS_None);
					}
					break;

				case DSS_ClampingReleaseOperating:
					if(SM_IsPositioningDone())
					{
						ReleaseDuration = CONTROL_TimeCounter - ReleaseDuration;
						HomingDuration = ClampingDuration = 0;
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
					LL_SPI_SetOutBit(SPI_OUT_BUS, false);
					LL_SPI_FlushOut();
					LOGIC_WaitDeadline = CONTROL_TimeCounter + SPI_WAIT_TIMEOUT;
					CONTROL_SetDeviceState(CONTROL_State, DSS_AdapterRelease_BusWait);
					break;

				case DSS_AdapterRelease_BusWait:
					if(LOGIC_WaitSpiInBit(SPI_IN_BUS_RELEASED))
						CONTROL_SetDeviceState(CONTROL_State, DSS_AdapterRelease_Adapter);
					break;

				case DSS_AdapterRelease_Adapter:
					LL_SPI_SetOutBit(SPI_OUT_ADAPTER, false);
					LL_SPI_FlushOut();
					LOGIC_WaitDeadline = CONTROL_TimeCounter + SPI_WAIT_TIMEOUT;
					CONTROL_SetDeviceState(CONTROL_State, DSS_AdapterRelease_AdapterWait);
					break;

				case DSS_AdapterRelease_AdapterWait:
					if(LOGIC_WaitSpiInBit(SPI_IN_ADAPTER_RELEASED))
						CONTROL_SetDeviceState(CONTROL_State, DSS_AdapterRelease_HeatingOff);
					break;

				case DSS_AdapterRelease_HeatingOff:
				{
					TRMError error = TRME_None;

					if(DataTable[REG_USE_HEATING])
						TRM10_Stop(TRM_CH1_ADDR, &error);

					IsHolding = false;
					LOGIC_ClampHeightMm = 0;
					DataTable[REG_ADAPTER_MATCH] = false;
					LOGIC_Id.Cached = FALSE;

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

					CONTROL_SetDeviceState(CONTROL_State, DSS_AdapterRelease_Done);
				}
					break;

				case DSS_AdapterRelease_Done:
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
