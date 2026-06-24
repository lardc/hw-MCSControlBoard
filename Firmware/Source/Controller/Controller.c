// Header
#include "Controller.h"

// Includes
#include "SysConfig.h"
#include "Global.h"
#include "DataTable.h"
#include "DeviceObjectDictionary.h"
#include "DeviceProfile.h"
#include "TRM101.h"
#include "StepperMotor.h"
#include "LowLevel.h"
#include "Measurement.h"
#include "Logic.h"
#include "ZwNFLASH.h"
#include "ZwIWDG.h"
#include "SaveToFlash.h"
#include "DebugActions.h"

// Variables
static Boolean CycleActive = FALSE;
Boolean HeatingActive = FALSE;

volatile Int64U CONTROL_TimeCounter = 0;
volatile DeviceState CONTROL_State = DS_None;
volatile DeviceSubState CONTROL_SubState = DSS_None;

volatile Int32U HomingDuration = 0, ClampingDuration = 0, ReleaseDuration = 0;
volatile Boolean RequestSaveToFlash = FALSE;

// Forward functions
static void CONTROL_FillWPPartDefault();
static Boolean CONTROL_DispatchAction(Int16U ActionID, pInt16U UserError);
static void CONTROL_SetFans(Boolean State);
void CONTROL_UpdateTRMTemperature();
static void CONTROL_InitStoragePointers();
static Boolean CONTROL_ShouldMonitorPressureFault();
static void CONTROL_WatchDogUpdate();

// Functions
void CONTROL_Init()
{
	// Data-table EPROM service configuration
	EPROMServiceConfig EPROMService = {
		(FUNC_EPROM_WriteValues)&NFLASH_WriteDT,
		(FUNC_EPROM_ReadValues)&NFLASH_ReadDT
	};

	DT_Init(EPROMService, FALSE);
	DT_SaveFirmwareInfo(CAN_NID, 0);
	// Fill state variables with default values
	CONTROL_FillWPPartDefault();

	// Device profile initialization
	DEVPROFILE_Init(&CONTROL_DispatchAction, &CycleActive);

	// Reset control values
	DEVPROFILE_ResetControlSection();
	CONTROL_InitStoragePointers();
	SM_ResetZeroPoint();

	if(DataTable[REG_USE_HEATING])
	{
		TRMError dummy_error;
		TRM_Stop(TRM_CH1_ADDR, &dummy_error);
	}

	CONTROL_SetDeviceState(DS_SelfTest, DSS_None);
}
// ----------------------------------------

void CONTROL_Idle()
{
	CycleActive = LOGIC_IsCycleActive();
	DEVPROFILE_ProcessRequests();
	CONTROL_UpdateTRMTemperature();

	DataTable[REG_SENSOR_S2] = LL_IsTableSensorOk();
	DataTable[REG_SENSOR_S3] = LL_FilterSafetyCircuit(LL_IsSafetyS3Ok());
	DataTable[REG_SENSOR_S5] = LL_FilterSafetyCircuit(LL_IsSafetyS5Ok());
	DataTable[REG_HOMING_SENSOR] = LL_HomeSensorActuate();
	DataTable[REG_BUS_TOOLING_SENSOR] = LL_SPI_GetInBit(SPI_IN_BUS_HELD);
	DataTable[REG_ADAPTER_TOOLING_SENSOR] = LL_SPI_GetInBit(SPI_IN_ADAPTER_HELD);
	DataTable[REG_SPI_IN_STATE] = LL_SPI_ReadInRaw();
	DataTable[REG_PRESSURE] = MEAS_GetPressureMilliBar();
	CONTROL_UpdatePressureOK();
	LOGIC_Process();

	if(RequestSaveToFlash)
	{
		RequestSaveToFlash = FALSE;
		STF_SaveDiagData();
	}

	CONTROL_WatchDogUpdate();
}
// ----------------------------------------

static void CONTROL_WatchDogUpdate()
{
	if(BOOT_LOADER_VARIABLE != BOOT_LOADER_REQUEST)
		IWDG_Refresh();
}
// ----------------------------------------

static void CONTROL_FillWPPartDefault()
{
	DataTable[REG_DEV_STATE] = (Int16U)DS_None;
	DataTable[REG_FAULT_REASON] = FAULT_NONE;
	DataTable[REG_DISABLE_REASON] = DISABLE_NONE;
	DataTable[REG_WARNING] = WARNING_NONE;
	DataTable[REG_PROBLEM] = PROBLEM_NONE;
	DataTable[REG_ADAPTER_MATCH] = ADAPTER_MATCH_NONE;
	DataTable[REG_ADAPTER_MISMATCH] = ADAPTER_MISMATCH_NONE;
}
// ----------------------------------------

void CONTROL_SetDeviceState(DeviceState NewState, DeviceSubState NewSubState)
{
	CONTROL_State = NewState;
	DataTable[REG_DEV_STATE] = NewState;

	CONTROL_SubState = NewSubState;
	DataTable[REG_DEV_SUBSTATE] = NewSubState;
}
// ----------------------------------------

static void CONTROL_SetFans(Boolean State)
{
	LL_SPI_SetOutBit(SPI_OUT_FAN1, State);
	LL_SPI_SetOutBit(SPI_OUT_FAN2, State);
	LL_SPI_FlushOut();
}
// ----------------------------------------

static Boolean CONTROL_DispatchAction(Int16U ActionID, pInt16U UserError)
{
	switch(ActionID)
	{
		case ACT_ADAPTER_WRITE_ID:
			LOGIC_AdapterIdInit();
			{
				AdapterIdentifier Id;
				Id.Code = DataTable[REG_ADAPTER_ID];
				Id.ClampHeightMm = DataTable[REG_ADAPTER_CLAMP_HEIGHT];
				Id.MaxCurrent = DataTable[REG_ADAPTER_MAX_CURRENT];
				Id.MaxVoltage = DataTable[REG_ADAPTER_MAX_VOLTAGE];
				Id.Serial = DataTable[REG_ADAPTER_SERIAL];
				if(!LOGIC_AdapterIdWrite(&Id))
					*UserError = ERR_DEVICE_NOT_READY;
			}
			break;

		case ACT_ADAPTER_READ_ID:
			LOGIC_AdapterIdInit();
			{
				AdapterIdentifier Id;
				if(!LOGIC_AdapterIdRead(&Id))
					*UserError = ERR_DEVICE_NOT_READY;
			}
			break;

		case ACT_HOMING:
			if(CONTROL_State == DS_None || CONTROL_State == DS_Halt || CONTROL_State == DS_Ready)
			{
				HomingDuration = CONTROL_TimeCounter;
				SM_Homing();
				CONTROL_SetDeviceState(DS_Homing, DSS_HomingSearchSensor);
			}
			else
				*UserError = ERR_OPERATION_BLOCKED;
			break;

		case ACT_START_CLAMPING:
			if(CONTROL_State == DS_Ready)
			{
				if(!LL_IsTableSensorOk())
				{
					*UserError = ERR_DEVICE_NOT_READY;
					break;
				}

				ClampingDuration = CONTROL_TimeCounter;
				DataTable[REG_PROBLEM] = PROBLEM_NONE;
				CONTROL_SetDeviceState(DS_Clamping, DSS_None);
			}
			else
				*UserError = ERR_DEVICE_NOT_READY;
			break;

		case ACT_RELEASE_CLAMPING:
			if(CONTROL_State == DS_Halt || CONTROL_State == DS_ClampingDone || CONTROL_State == DS_Ready)
			{
				ReleaseDuration = CONTROL_TimeCounter;
				HomingDuration = 0;
				CONTROL_SetDeviceState(DS_ClampingRelease, DSS_None);
			}
			else
				*UserError = ERR_OPERATION_BLOCKED;
			break;

		case ACT_HALT:
			CONTROL_Halt();
			break;

		case ACT_RELEASE_ADAPTER:
			if(CONTROL_State == DS_None || CONTROL_State == DS_Ready)
				CONTROL_SetDeviceState(DS_AdapterRelease, DSS_AdapterRelease_Bus);
			else
				*UserError = ERR_OPERATION_BLOCKED;
			break;

		case ACT_HOLD_ADAPTER:
			if(CONTROL_State == DS_None || CONTROL_State == DS_Ready)
			{
				DataTable[REG_ADAPTER_MATCH] = ADAPTER_MATCH_NONE;
				CONTROL_SetDeviceState(DS_AdapterHold, DSS_AdapterHold_CheckPressure);
			}
			else
				*UserError = ERR_OPERATION_BLOCKED;
			break;

		case ACT_SET_TEMPERATURE:
			{
				if(DataTable[REG_USE_HEATING])
				{
					TRMError error;
					if(DataTable[REG_TEMP_SETPOINT] < TRM_TEMP_THR)
					{
						TRM_SetTemp(TRM_CH1_ADDR, DataTable[REG_TEMP_SETPOINT], &error);
						if(error == TRME_None)
							TRM_Stop(TRM_CH1_ADDR, &error);

						HeatingActive = FALSE;
					}
					else
					{
						TRM_SetTemp(TRM_CH1_ADDR, DataTable[REG_TEMP_SETPOINT], &error);
						if(error == TRME_None)
							TRM_Start(TRM_CH1_ADDR, &error);

						HeatingActive = TRUE;
					}

					CONTROL_SetFans(HeatingActive);

					if(error != TRME_None)
					{
						CONTROL_SwitchToFault(FAULT_TRM);
						DataTable[REG_TRM_ERROR] = error;
						*UserError = ERR_TRM_COMM_ERR;
					}
				}
				else
					DataTable[REG_TEMP_CH1] = DataTable[REG_TEMP_SETPOINT];
			}
			break;

		case ACT_CLR_FAULT:
			{
				if(CONTROL_State == DS_Fault)
					CONTROL_SetDeviceState(DS_None, DSS_None);
				else if(CONTROL_State == DS_Disabled)
					*UserError = ERR_OPERATION_BLOCKED;

				DataTable[REG_FAULT_REASON] = FAULT_NONE;
				DataTable[REG_PROBLEM] = PROBLEM_NONE;
			}
			break;

		case ACT_CLR_WARNING:
			DataTable[REG_WARNING] = WARNING_NONE;
			break;

		case ACT_CLR_HALT:
			if(CONTROL_State == DS_Halt)
				CONTROL_SetDeviceState(DS_Ready, DSS_None);
			break;

		default:
			return DEBUG_HandleDiagnosticAction(ActionID, UserError);
	}

	return TRUE;
}
// ----------------------------------------

void CONTROL_Halt()
{
	SM_RequestStop();
	CONTROL_SetDeviceState(DS_Halt, DSS_None);
}
// ----------------------------------------

void CONTROL_FinishedWithProblem(Int16U Problem)
{
	DataTable[REG_OP_RESULT] = OPRESULT_FAIL;
	DataTable[REG_PROBLEM] = Problem;
}
// ----------------------------------------

void CONTROL_SwitchToFault(Int16U Reason)
{
	CONTROL_SetDeviceState(DS_Fault, DSS_None);
	DataTable[REG_FAULT_REASON] = Reason;
}
// ----------------------------------------

void CONTROL_UpdateTRMTemperature()
{
	static Int64U ReadTimeout = 0;
	static TRMError error = TRME_None;

	// Условие разрешения считывания температуры
	if(DataTable[REG_USE_HEATING] && error == TRME_None && CONTROL_State != DS_Fault && CONTROL_TimeCounter > ReadTimeout)
	{
		DataTable[REG_TEMP_CH1] = TRM_ReadTemp(TRM_CH1_ADDR, &error);
		ReadTimeout = CONTROL_TimeCounter + TRM_READ_PAUSE;
	}

	// Фолт при ошибке срабатывает только после завершения операции зажатия
	if(CONTROL_State == DS_Ready && error != TRME_None)
	{
		CONTROL_SwitchToFault(FAULT_TRM);
		DataTable[REG_TEMP_CH1] = 0;
		DataTable[REG_TRM_ERROR] = error;
		error = TRME_None;
	}
}
// ----------------------------------------

static Boolean CONTROL_ShouldMonitorPressureFault()
{
	if(CycleActive)
		return TRUE;

	if(LL_SPI_GetInBit(SPI_IN_ADAPTER_HELD) || LL_SPI_GetInBit(SPI_IN_BUS_HELD))
		return TRUE;

	return !SM_IsHomingDone() || !SM_IsPositioningDone();
}
// ----------------------------------------

void CONTROL_UpdatePressureOK()
{
	static Int64U PressureOkTime = 0;
	Int32U Pressure = DataTable[REG_PRESSURE];

	if(Pressure >= DataTable[REG_PRESSURE_OK])
		PressureOkTime = CONTROL_TimeCounter;

	if(CONTROL_ShouldMonitorPressureFault()
			&& CONTROL_TimeCounter > PressureOkTime + PNEUMATIC_READ_PAUSE)
	{
		DataTable[REG_DBG] = Pressure;
		CONTROL_SwitchToFault(FAULT_PRESSURE);
	}
}
// ----------------------------------------

void CONTROL_InitStoragePointers()
{
	STF_AssignPointer(0, (Int32U)&HomingDuration);
	STF_AssignPointer(1, (Int32U)&ClampingDuration);
	STF_AssignPointer(2, (Int32U)&ReleaseDuration);
}
//--------------------
