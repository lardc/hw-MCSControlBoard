// ----------------------------------------
// Controller logic
// ----------------------------------------

// Header
#include "Controller.h"

// Includes
#include "SysConfig.h"
#include "Global.h"
#include "SCCISlave.h"
#include "ZbBoard.h"
#include "DataTable.h"
#include "DeviceObjectDictionary.h"
#include "DeviceProfile.h"
#include "TRM101.h"
#include "StepperMotor.h"
#include "StepperMotorDiag.h"
#include "DS18B20.h"
#include "LowLevel.h"
#include "Measurement.h"
#include "SelfTest.h"
#include "ZwNFLASH.h"
#include "SaveToFlash.h"

// Types
typedef void (*FUNC_AsyncDelegate)();

// Variables
static Boolean CycleActive = FALSE, HeatingActive = FALSE;
static volatile FUNC_AsyncDelegate DPCDelegate = NULL;

volatile Int64U FanTimeout = 0, CONTROL_TimeCounter = 0, Timeout;
volatile DeviceState CONTROL_State = DS_None;
volatile DeviceSubState CONTROL_SubState = DSS_None;

volatile Int16U CONTROL_Values_Counter = 0, CONTROL_ExtInfoCounter = 0;
volatile Int32U HomingDuration = 0, ClampingDuration = 0, ReleaseDuration = 0;
volatile Boolean RequestSaveToFlash = FALSE;

volatile Int16U CONTROL_BootLoaderRequest = 0;

// Forward functions
static void CONTROL_HandleFanControl();
static void CONTROL_HandleClampActions();
static void CONTROL_ProcessSelfTest();
static void CONTROL_SetDeviceState(DeviceState NewState, DeviceSubState NewSubState);
static void CONTROL_FillWPPartDefault();
static Boolean CONTROL_DispatchAction(Int16U ActionID, pInt16U UserError);
void CONTROL_SwitchToFault(Int16U Reason);
void CONTROL_PreparePositioningX(Int16U NewPosition, Int16U SlowDownDistance,
		Int16U MaxSpeed, Int16U SlowSpeed, Int16U MinSpeed);
void CONTROL_PrepareHomingOffset();
void CONTROL_PrepareClamping(Boolean Clamp);
void CONTROL_Halt();
void CONTROL_UpdateTRMTemperature();
static void CONTROL_InitStoragePointers();

// Functions
void CONTROL_Init()
{
	// Variables for endpoint configuration
	// TODO
	
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
	// TODO DEVPROFILE_InitEPService16

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
	CONTROL_ProcessSelfTest();

	// Process deferred procedures
	if(DPCDelegate)
	{
		FUNC_AsyncDelegate del = DPCDelegate;
		DPCDelegate = NULL;
		del();
	}

	if (RequestSaveToFlash)
	{
		RequestSaveToFlash = FALSE;
		STF_SaveDiagData();
	}
}
// ----------------------------------------

void CONTROL_UpdateLow()
{
	CONTROL_HandleFanControl();
	CONTROL_HandleClampActions();
}
// ----------------------------------------

static void CONTROL_ProcessSelfTest()
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

static void CONTROL_FillWPPartDefault()
{
	// Set volatile states
	DataTable[REG_DEV_STATE] = (Int16U)DS_None;
	DataTable[REG_FAULT_REASON] = FAULT_NONE;
	DataTable[REG_DISABLE_REASON] = DISABLE_NONE;
	DataTable[REG_WARNING] = WARNING_NONE;
	DataTable[REG_PROBLEM] = PROBLEM_NONE;
	DataTable[REG_ADAPTER_MATCH] = ADAPTER_MATCH_NONE;
	DataTable[REG_ADAPTER_MISMATCH] = ADAPTER_MISMATCH_NONE;
}
// ----------------------------------------

static void CONTROL_SetDeviceState(DeviceState NewState, DeviceSubState NewSubState)
{
	if(NewState == DS_Clamping)
		FanTimeout = CONTROL_TimeCounter + FAN_TIMEOUT;
	
	CONTROL_State = NewState;
	DataTable[REG_DEV_STATE] = NewState;

	CONTROL_SubState = NewSubState;
	DataTable[REG_DEV_SUBSTATE] = NewSubState;
}
// ----------------------------------------

static void CONTROL_HandleFanControl()
{
	Boolean FanOn = (FanTimeout > CONTROL_TimeCounter) || HeatingActive;

	LL_SPI_SetOutBit(SPI_OUT_FAN1, FanOn);
	LL_SPI_SetOutBit(SPI_OUT_FAN2, FanOn);
	LL_SPI_FlushOut();
}
// ----------------------------------------

static void CONTROL_HandleClampActions()
{
	static Int64U Timeout = 0;
	Boolean IsBusClampOk, IsAdapterClampOk;

	switch(CONTROL_State)
	{
		case DS_Homing:
		case DS_Clamping:
		case DS_ClampingRelease:
			if(!LL_FilterSafetyCircuit(LL_IsSafetyS3Ok()) || !LL_FilterSafetyCircuit(LL_IsSafetyS5Ok()))
				CONTROL_Halt();
			break;
		default:
			break;
	}

	// Обработка общей логики отключения управления
	switch(CONTROL_SubState)
	{
		case DSS_Com_CheckControl:
			{
				// Раннее включение поджатия адаптера если зажимается прибор
				CONTROL_SetDeviceState(CONTROL_State, DSS_Com_ReleaseDone);
			}
			break;

		case DSS_Com_ControlRelease:
			if(CONTROL_TimeCounter > Timeout)
				CONTROL_SetDeviceState(CONTROL_State, DSS_Com_ReleaseDone);
			break;

		default:
			break;
	}

	// Обработка машины подсостояний
	switch(CONTROL_State)
	{
		case DS_Homing:
			switch(CONTROL_SubState)
			{
				case DSS_Com_ReleaseDone:
					SM_Homing(DataTable[REG_HOMING_SPEED]);
					CONTROL_SetDeviceState(CONTROL_State, DSS_HomingSearchSensor);
					break;

				case DSS_HomingSearchSensor:
					if(SM_IsHomingDone())
					{
						Timeout = CONTROL_TimeCounter + HOMING_PAUSE;
						CONTROL_SetDeviceState(CONTROL_State, DSS_HomingPause);
					}
					break;

				case DSS_HomingPause:
					if(CONTROL_TimeCounter > Timeout)
					{
						CONTROL_PrepareHomingOffset();
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

		case DS_Clamping:
			switch(CONTROL_SubState)
			{
				case DSS_Com_ReleaseDone:
					Timeout = CONTROL_TimeCounter + PNEUMATIC_POWER_TIMEOUT;
					CONTROL_SetDeviceState(CONTROL_State, DSS_ClampingWaitSensors);
					break;

				case DSS_ClampingWaitSensors:
					IsBusClampOk = LL_SPI_GetInBit(SPI_IN_BUS_HELD);
					IsAdapterClampOk = LL_SPI_GetInBit(SPI_IN_ADAPTER_HELD);

					if(IsBusClampOk && IsAdapterClampOk)
					{
						CONTROL_PrepareClamping(TRUE);
						CONTROL_SetDeviceState(CONTROL_State, DSS_ClampingOperating);
					}
					else if(CONTROL_TimeCounter > Timeout)
						CONTROL_SwitchToFault(IsAdapterClampOk ? FAULT_BUS_SEN : FAULT_ADAPTER_SEN);
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
				case DSS_Com_ReleaseDone:
					CONTROL_PrepareClamping(FALSE);
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

		default:
			break;
	}
}
// ----------------------------------------

static Boolean CONTROL_DispatchAction(Int16U ActionID, pInt16U UserError)
{
	switch(ActionID)
	{
		case ACT_ADAPTER_WRITE_ID:
			DS18B20_Init();
			{
				AdapterIdentifier Id;
				Id.Code = DataTable[REG_ADAPTER_ID];
				Id.ClampHeightMm = DataTable[REG_ADAPTER_CLAMP_HEIGHT];
				Id.MaxCurrent = DataTable[REG_ADAPTER_MAX_CURRENT];
				Id.MaxVoltage = DataTable[REG_ADAPTER_MAX_VOLTAGE];
				Id.Serial = DataTable[REG_ADAPTER_SERIAL];
				if(!DS18B20_WriteIdentifier(&Id))
					*UserError = ERR_DEVICE_NOT_READY;
			}
			break;

		case ACT_ADAPTER_READ_ID:
			DS18B20_Init();
			{
				AdapterIdentifier Id;
				if(!DS18B20_ReadIdentifier(&Id))
					*UserError = ERR_DEVICE_NOT_READY;
			}
			break;

		case ACT_HOMING:
			if(CONTROL_State == DS_None || CONTROL_State == DS_Halt || CONTROL_State == DS_Ready)
			{
				HomingDuration = CONTROL_TimeCounter;
				CONTROL_SetDeviceState(DS_Homing, DSS_Com_CheckControl);
			}
			else
				*UserError = ERR_OPERATION_BLOCKED;
			break;
			
		case ACT_START_CLAMPING:
			if (CONTROL_State == DS_Ready)
			{
				ClampingDuration = CONTROL_TimeCounter;
				DataTable[REG_PROBLEM] = PROBLEM_NONE;
				CONTROL_SetDeviceState(DS_Clamping, DSS_Com_CheckControl);
			}
			else
				*UserError = ERR_DEVICE_NOT_READY;
			break;
			
		case ACT_RELEASE_CLAMPING:
			if(CONTROL_State == DS_Halt || CONTROL_State == DS_ClampingDone || CONTROL_State == DS_Ready)
			{
				ReleaseDuration = HomingDuration = CONTROL_TimeCounter;
				// После срабатывания шторки безопасности команда разжатия приводит к хоумингу
				CONTROL_SetDeviceState(SM_IsSafetyEvent() ? DS_Homing : DS_ClampingRelease, DSS_Com_CheckControl);
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
				LL_SPI_SetOutBit(SPI_OUT_ADAPTER, false);
				LL_SPI_SetOutBit(SPI_OUT_BUS, false);
				LL_SPI_FlushOut();
			}
			else
				*UserError = ERR_OPERATION_BLOCKED;
			break;
			
		case ACT_HOLD_ADAPTER:
			if(CONTROL_State == DS_None || CONTROL_State == DS_Ready)
			{
				LL_SPI_SetOutBit(SPI_OUT_ADAPTER, true);
				LL_SPI_SetOutBit(SPI_OUT_BUS, true);
				LL_SPI_FlushOut();
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
			}
			break;
			
		case ACT_CLR_WARNING:
			DataTable[REG_WARNING] = WARNING_NONE;
			break;
			
		case ACT_CLR_HALT:
			if(CONTROL_State == DS_Halt)
				CONTROL_SetDeviceState(DS_Ready, DSS_None);
			break;
			
		case ACT_DBG_READ_EXT_TEMP:
			{
				if(DataTable[REG_USE_HEATING])
				{
					TRMError error;
					DataTable[REG_TRM_DATA] = TRM_ReadTemp(DataTable[REG_DBG_TRM_ADDRESS], &error);
					DataTable[REG_TRM_ERROR] = error;

					if(error != TRME_None)
					*UserError = ERR_TRM_COMM_ERR;
				}
				else
					*UserError = ERR_OPERATION_BLOCKED;
			}
			break;

		case ACT_DBG_READ_TRM_TEMP:
			{
				if(DataTable[REG_USE_HEATING])
				{
					TRMError error;
					DataTable[REG_TRM_DATA] = TRM_ReadTemp(DataTable[REG_DBG_TRM_ADDRESS], &error);
					DataTable[REG_TRM_ERROR] = error;
					
					if(error != TRME_None)
						*UserError = ERR_TRM_COMM_ERR;
				}
				else
					*UserError = ERR_OPERATION_BLOCKED;
			}
			break;
			
		case ACT_DBG_READ_TRM_POWER:
			{
				if(DataTable[REG_USE_HEATING])
				{
					TRMError error;
					DataTable[REG_TRM_DATA] = TRM_ReadPower(DataTable[REG_DBG_TRM_ADDRESS], &error);
					DataTable[REG_TRM_ERROR] = error;
					
					if(error != TRME_None)
						*UserError = ERR_TRM_COMM_ERR;
				}
				else
					*UserError = ERR_OPERATION_BLOCKED;
			}
			break;
			
		case ACT_DBG_TRM_START:
			{
				if(DataTable[REG_USE_HEATING])
				{
					TRMError error;
					TRM_Start(DataTable[REG_DBG_TRM_ADDRESS], &error);
					DataTable[REG_TRM_ERROR] = error;
					
					if(error != TRME_None)
						*UserError = ERR_TRM_COMM_ERR;
				}
				else
					*UserError = ERR_OPERATION_BLOCKED;
			}
			break;
			
		case ACT_DBG_TRM_STOP:
			{
				if(DataTable[REG_USE_HEATING])
				{
					TRMError error;
					TRM_Stop(DataTable[REG_DBG_TRM_ADDRESS], &error);
					DataTable[REG_TRM_ERROR] = error;
					
					if(error != TRME_None)
						*UserError = ERR_TRM_COMM_ERR;
				}
				else
					*UserError = ERR_OPERATION_BLOCKED;
			}
			break;
			
		case ACT_DBG_MOTOR_START:
			SMD_ConnectHandler();
			break;
			
		case ACT_DBG_MOTOR_STOP:
			SMD_RequstStop();
			break;

		default:
			return FALSE;
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

void CONTROL_SwitchToFault(Int16U Reason)
{
	CONTROL_SetDeviceState(DS_Fault, DSS_None);
	DataTable[REG_FAULT_REASON] = Reason;
}
// ----------------------------------------

void CONTROL_PreparePositioningX(Int16U NewPosition, Int16U SlowDownDistance,
		Int16U MaxSpeed, Int16U SlowSpeed, Int16U MinSpeed)
{
	SM_Params Params;
	Params.NewPosition = NewPosition;
	Params.SlowDownDistance = SlowDownDistance;
	Params.MaxSpeed = MaxSpeed;
	Params.SlowSpeed = SlowSpeed;
	Params.MinSpeed = MinSpeed;

	SM_GoToPosition(&Params);
}
// ----------------------------------------

void CONTROL_PrepareClamping(Boolean Clamp)
{
	SM_Params Params;

	if(Clamp)
		SM_Config(&Params, DataTable[REG_ADAPTER_CLAMP_HEIGHT], TRUE);
	else
		SM_Config(&Params, 0, FALSE);

	SM_GoToPosition(&Params);
}
// ----------------------------------------

void CONTROL_PrepareHomingOffset()
{
	CONTROL_PreparePositioningX(DataTable[REG_HOMING_OFFSET], 0,
			DataTable[REG_HOMING_SPEED], DataTable[REG_HOMING_SPEED], DataTable[REG_HOMING_SPEED]);
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
