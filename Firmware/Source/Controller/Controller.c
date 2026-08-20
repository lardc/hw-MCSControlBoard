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
#include "DS2431.h"
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
volatile Int16U CONTROL_ExtInfoCounter = 0;
volatile Int16U CONTROL_ValuesCounter = 0;
static Int64U CT_SaveTimer = 0;

volatile Int32U HomingDuration = 0, ClampingDuration = 0, ReleaseDuration = 0;
volatile Boolean RequestSaveToFlash = FALSE;

float CONTROL_MotorMovement[VALUES_x_SIZE] = {0};
float CONTROL_MotorSpeed[VALUES_x_SIZE] = {0};
volatile float CONTROL_ExtInfoData[VALUES_EXT_INFO_SIZE];

// Forward functions
static void CONTROL_FillWPPartDefault();
static Boolean CONTROL_DispatchAction(Int16U ActionID, pInt16U UserError);
static void CONTROL_SetFans(Boolean State);
void CONTROL_UpdateTRMTemperature();
static void CONTROL_InitStoragePointers();
static Boolean CONTROL_ShouldMonitorPressureFault();
static void CONTROL_WatchDogUpdate();
void CONTROL_ResetOutputRegisters();

// Functions
void CONTROL_Init()
{
	// Переменные для конфигурации EndPoint
	Int16U FEPIndexes[FEP_COUNT] = {EP_MotorMovement, EP_MotorSpeed, EP_ExtInfoData};

	Int16U FEPSized[FEP_COUNT] = {VALUES_x_SIZE, VALUES_x_SIZE, VALUES_EXT_INFO_SIZE};

	pInt16U FEPCounters[FEP_COUNT] = {(pInt16U)&CONTROL_ValuesCounter, (pInt16U)&CONTROL_ValuesCounter, (pInt16U)&CONTROL_ExtInfoCounter};

	pFloat32 FEPDatas[FEP_COUNT] = {(pFloat32)CONTROL_MotorMovement, (pFloat32)CONTROL_MotorSpeed, (pFloat32)CONTROL_ExtInfoData};

	// Data-table EPROM service configuration
	EPROMServiceConfig EPROMService = {	(FUNC_EPROM_WriteValues)&NFLASH_WriteDT, (FUNC_EPROM_ReadValues)&NFLASH_ReadDT};

	DT_Init(EPROMService, FALSE);
	DT_SaveFirmwareInfo(CAN_NID, 0);
	// Fill state variables with default values
	CONTROL_FillWPPartDefault();

	// Device profile initialization
	DEVPROFILE_Init(&CONTROL_DispatchAction, &CycleActive);
	DEVPROFILE_InitFEPService(FEPIndexes, FEPSized, FEPCounters, FEPDatas);

	// Reset control values
	DEVPROFILE_ResetControlSection();

	SM_ResetZeroPoint();
	// Настройка указателей и счетчиков
	CONTROL_InitStoragePointers();
	STF_LoadCounters();

	if(DataTable[REG_USE_HEATING])
	{
		TRMError dummy_error;
		TRM_Stop(TRM_CH1_ADDR, &dummy_error);
	}

	CONTROL_SetDeviceState(DataTable[REG_USE_ST] ? DS_SelfTest : DS_None, DSS_None);
}
// ----------------------------------------

void CONTROL_Idle()
{
	CycleActive = LOGIC_IsCycleActive();
	DEVPROFILE_ProcessRequests();
	CONTROL_UpdateTRMTemperature();

	DataTable[REG_SENSOR_S2] = LL_IsTableSensorOk();
	DataTable[REG_HOMING_SENSOR] = LL_HomeSensorActuate();
	DataTable[REG_BUS_TOOLING_SENSOR] = LL_SPI_GetInBit(SPI_IN_BUS_HELD);
	DataTable[REG_ADAPTER_TOOLING_SENSOR] = LL_SPI_GetInBit(SPI_IN_ADAPTER_HELD);
	DataTable[REG_SPI_IN_STATE] = LL_SPI_ReadInRaw();
	DataTable[REG_PRESSURE] = MEAS_GetPressureMilliBar();
	CONTROL_UpdatePressureOK();
	LOGIC_Process();

	if(CONTROL_State == DS_None || CONTROL_State == DS_Fault ||  CONTROL_State == DS_Ready || CONTROL_State == DS_Halt
	|| CONTROL_State == DS_ClampingDone)
	{
		if(RequestSaveToFlash)
		{
			RequestSaveToFlash = FALSE;
			STF_SaveDiagData();
		}
		if(DataTable[REG_CNT_ACTIVE] && (CONTROL_TimeCounter - CT_SaveTimer) >= CT_SAVE_TIMEOUT)
		{
			STF_SaveCounterData();
			CT_SaveTimer = CONTROL_TimeCounter;
		}
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
	DataTable[REG_FAULT_REASON] = DF_NONE;
	DataTable[REG_DISABLE_REASON] = DISABLE_NONE;
	DataTable[REG_WARNING] = WARNING_NONE;
	DataTable[REG_PROBLEM] = PROBLEM_NONE;
	DataTable[REG_ADAPTER_MATCH] = false;
	DataTable[REG_ADAPTER_MISMATCH_CODE] = ADAPTER_MISMATCH_NONE;
}
// ----------------------------------------

void CONTROL_ResetOutputRegisters()
{
	DataTable[REG_PROBLEM] = PROBLEM_NONE;
	DataTable[REG_OP_RESULT] = OPRESULT_NONE;
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
		case ACT_ADAPTER_READ_ID:
			LOGIC_AdapterIdInit();
			if(!LOGIC_AdapterIdRead(&LOGIC_Id))
				*UserError = ERR_DEVICE_NOT_READY;
			break;

		case ACT_HOMING:
			if(CONTROL_State == DS_None || CONTROL_State == DS_Halt || CONTROL_State == DS_Ready)
			{
				HomingDuration = CONTROL_TimeCounter;
				CONTROL_SetDeviceState(DS_Homing, DSS_HomingSearchSensor);
			}
			else
				*UserError = ERR_OPERATION_BLOCKED;
			break;

		case ACT_GOTO_POSITION:
			if(CONTROL_State == DS_Ready)
			{
				CONTROL_ResetOutputRegisters();
				if(!SM_IsHomingDone())
				{
					CONTROL_FinishedWithProblem(PROBLEM_NO_HOMING);
					break;
				}
				else if(DataTable[REG_CUSTOM_POS] > POS_MAX)
				{
					CONTROL_FinishedWithProblem(PROBLEM_INVALID_POSITION);
					break;
				}
				else if(DataTable[REG_POS_SPEED_MIN] > DataTable[REG_POS_SPEED_MAX])
				{
					CONTROL_FinishedWithProblem(PROBLEM_INVALID_SPEED);
					break;
				}
				else
					CONTROL_SetDeviceState(DS_Movement, DSS_MovementStart);
			}
			else
				*UserError = ERR_DEVICE_NOT_READY;
			break;

		case ACT_START_CLAMPING:
			if(CONTROL_State == DS_Ready)
			{
				if(!LL_IsTableSensorOk())
				{
					*UserError = ERR_DEVICE_NOT_READY;
					break;
				}

				if(DataTable[REG_POS_SPEED_MIN] > DataTable[REG_POS_SPEED_MAX])
				{
					CONTROL_FinishedWithProblem(PROBLEM_INVALID_SPEED);
					break;
				}

				ClampingDuration = CONTROL_TimeCounter;
				CONTROL_ResetOutputRegisters();
				CONTROL_SetDeviceState(DS_Clamping, DSS_None);
			}
			else
				*UserError = ERR_DEVICE_NOT_READY;
			break;

		case ACT_RELEASE_CLAMPING:
			if(CONTROL_State == DS_Halt || CONTROL_State == DS_ClampingDone || CONTROL_State == DS_Ready)
			{
				if(DataTable[REG_POS_SPEED_MIN] > DataTable[REG_POS_SPEED_MAX])
				{
					CONTROL_FinishedWithProblem(PROBLEM_INVALID_SPEED);
					break;
				}

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
			{
				CONTROL_ResetOutputRegisters();
				CONTROL_SetDeviceState(DS_AdapterRelease, DSS_AdapterRelease_Bus);
			}
			else
				*UserError = ERR_OPERATION_BLOCKED;
			break;

		case ACT_HOLD_ADAPTER:
			if(CONTROL_State == DS_None || CONTROL_State == DS_Ready)
			{
				CONTROL_ResetOutputRegisters();
				CONTROL_SetDeviceState(DS_AdapterHold, DSS_AdapterHold_CheckPressure);
			}
			else
				*UserError = ERR_OPERATION_BLOCKED;
			break;

		case ACT_UPDATE_ADAPTER_MATCH:
			CONTROL_ResetOutputRegisters();
			if(!LOGIC_Id.Cached)
				*UserError = ERR_DEVICE_NOT_READY;
			else if(LOGIC_ValidateAdapter(&LOGIC_Id))
				DataTable[REG_OP_RESULT] = OPRESULT_OK;
			else
				CONTROL_FinishedWithProblem(PROBLEM_ADAPTER_MISMATCH);
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

					if(error != TRME_None)
					{
						CONTROL_SwitchToFault(DF_TRM);
						DataTable[REG_TRM_ERROR] = error;
						*UserError = ERR_TRM_COMM_ERR;
					}
					else
						CONTROL_SetFans(HeatingActive);
				}
				else
					DataTable[REG_TEMP_CH1] = DataTable[REG_TEMP_SETPOINT];
			}
			break;

		case ACT_CLR_FAULT:
			{
				if(CONTROL_State == DS_Fault)
					CONTROL_SetDeviceState(DS_None, DSS_None);

				DataTable[REG_FAULT_REASON] = DF_NONE;
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

Int16U CONTROL_ProblemFromDs2431()
{
	switch(DS2431_GetLastError())
	{
		case DS2431_ERR_LINE:		return PROBLEM_OW_ERROR_LINE;
		case DS2431_ERR_NO_DEVICE:	return PROBLEM_OW_NO_DEVICE;
		case DS2431_ERR_VERIFY:		return PROBLEM_OW_VERIFY;
		case DS2431_ERR_PARAM:		return PROBLEM_OW_PARAM;
		case DS2431_OK:
		default:					return PROBLEM_NONE;
	}
}
// ----------------------------------------

void CONTROL_SwitchToFault(Int16U Reason)
{
	CONTROL_SetDeviceState(DS_Fault, DSS_None);
	DataTable[REG_FAULT_REASON] = Reason;
	DataTable[REG_OP_RESULT] = OPRESULT_FAIL;
}
// ----------------------------------------

void CONTROL_UpdateTRMTemperature()
{
	static Int64U ReadTimeout = 0;
	static TRMError error = TRME_None;

	if(!DataTable[REG_USE_HEATING])
	{
		error = TRME_None;
		return;
	}

	// Условие разрешения считывания температуры
	if(error == TRME_None && CONTROL_State != DS_Fault && CONTROL_TimeCounter > ReadTimeout)
	{
		DataTable[REG_TEMP_CH1] = TRM_ReadTemp(TRM_CH1_ADDR, &error);
		ReadTimeout = CONTROL_TimeCounter + TRM_READ_PAUSE;
	}

	// Фолт при ошибке срабатывает только после выхода в Ready (хоуминг/зажатие)
	if(CONTROL_State == DS_Ready && error != TRME_None)
	{
		CONTROL_SwitchToFault(DF_TRM);
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

	return SM_IsBusy();
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
		DataTable[REG_PRESSURE] = Pressure;
		CONTROL_SwitchToFault(DF_PRESSURE);
	}
}
// ----------------------------------------

void CONTROL_InitStoragePointers()
{
	for(Int16U i = 0; i < COMMUTATION_TABLE_SIZE; ++i)
		STF_AssignCounterPointer(i, (Int32U)&CycleCounters[i]);

	STF_AssignPointer(0, (Int32U)&DataTable[REG_DEV_STATE]);
	STF_AssignPointer(1, (Int32U)&DataTable[REG_FAULT_REASON]);
	STF_AssignPointer(2, (Int32U)&DataTable[REG_DISABLE_REASON]);
	STF_AssignPointer(3, (Int32U)&DataTable[REG_WARNING]);
	STF_AssignPointer(4, (Int32U)&DataTable[REG_PROBLEM]);
	STF_AssignPointer(5, (Int32U)&DataTable[REG_OP_RESULT]);
	STF_AssignPointer(6, (Int32U)&DataTable[REG_TEMP_CH1]);
	STF_AssignPointer(7, (Int32U)&DataTable[REG_TRM_DATA]);
	STF_AssignPointer(8, (Int32U)&DataTable[REG_TRM_ERROR]);
	STF_AssignPointer(9, (Int32U)&DataTable[REG_PRESSURE]);
	STF_AssignPointer(10, (Int32U)&DataTable[REG_SENSOR_S2]);
	STF_AssignPointer(11, (Int32U)&DataTable[REG_HOMING_SENSOR]);
	STF_AssignPointer(12, (Int32U)&DataTable[REG_BUS_TOOLING_SENSOR]);
	STF_AssignPointer(13, (Int32U)&DataTable[REG_ADAPTER_TOOLING_SENSOR]);
	STF_AssignPointer(14, (Int32U)&DataTable[REG_DEV_SUBSTATE]);
	STF_AssignPointer(15, (Int32U)&DataTable[REG_SELFTEST_RESULT]);
	STF_AssignPointer(16, (Int32U)&DataTable[REG_ADAPTER_MATCH]);
	STF_AssignPointer(17, (Int32U)&DataTable[REG_ADAPTER_MISMATCH_CODE]);
	STF_AssignPointer(18, (Int32U)&DataTable[REG_SPI_IN_STATE]);
	STF_AssignPointer(19, (Int32U)&DataTable[REG_SENSOR_S3]);
	STF_AssignPointer(20, (Int32U)&DataTable[REG_SENSOR_S5]);
	STF_AssignPointer(21, (Int32U)&DataTable[REG_DEBUG_SCALING_COEF]);
	STF_AssignPointer(22, (Int32U)CONTROL_MotorMovement);
	STF_AssignPointer(23, (Int32U)CONTROL_MotorSpeed);
}
//--------------------
