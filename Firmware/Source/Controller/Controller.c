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
#include "ZbCSAdapter.h"
#include "ZbMemory.h"
#include "SaveToFlash.h"

// Definitions
//
#define SC_TYPE_MASK		0xFC00

// Types
typedef void (*FUNC_AsyncDelegate)();

// Variables
static volatile Boolean CycleActive = FALSE, HeatingActive = FALSE;
static volatile FUNC_AsyncDelegate DPCDelegate = NULL;

volatile Int64U FanTimeout = 0, CONTROL_TimeCounter = 0, Timeout;
volatile DeviceState CONTROL_State = DS_None;
volatile DeviceSubState CONTROL_SubState = DSS_None;

Int16U CONTROL_Values_1[VALUES_x_SIZE];
Int32U CONTROL_ExtInfoData[VALUES_x_SIZE];
volatile Int16U CONTROL_Values_Counter = 0, CSPressure = 0, AdapterID = 0, CONTROL_ExtInfoCounter = 0;
volatile Int32U HomingDuration = 0, ClampingDuration = 0, ReleaseDuration = 0;
volatile Boolean RequestSaveToFlash = FALSE;

// Boot-loader flag
#pragma DATA_SECTION(CONTROL_BootLoaderRequest, "bl_flag");
volatile Int16U CONTROL_BootLoaderRequest = 0;

// Forward functions
static void CONTROL_HandleFanControl();
static void CONTROL_HandleClampActions();
static void CONTROL_SetDeviceState(DeviceState NewState, DeviceSubState NewSubState);
static void CONTROL_FillWPPartDefault();
static Boolean CONTROL_DispatchAction(Int16U ActionID, pInt16U UserError);
void CONTROL_SwitchToFault(Int16U Reason);
void CONTROL_PreparePositioningX(Int16U NewPosition, Int16U SlowDownDistance,
		Int16U MaxSpeed, Int16U LowSpeed, Int16U MinSpeed);
void CONTROL_PreparePositioning();
void CONTROL_PrepareHomingOffset();
void CONTROL_PrepareClamping(Boolean Clamp);
void CONTROL_Halt();
void CONTROL_UpdateTRMTemperature();
void UpdatePressureOK();
static void CONTROL_InitStoragePointers();

// Functions
void CONTROL_Init(Boolean BadClockDetected)
{
	// Variables for endpoint configuration
	Int16U EPIndexes_16[EP_COUNT_16] = {0};
	Int16U EPSized_16[EP_COUNT_16] = {VALUES_x_SIZE};
	pInt16U EPCounters_16[EP_COUNT_16] = {(pInt16U)&CONTROL_Values_Counter};
	pInt16U EPDatas_16[EP_COUNT_16] = {CONTROL_Values_1};
	
	// Variables for endpoint configuration
	Int16U EPIndexes_32[EP_COUNT_32] = {EP32_ExtInfoData};
	Int16U EPSized_32[EP_COUNT_32] = {VALUES_x_SIZE};
	pInt16U EPCounters_32[EP_COUNT_32] = {(pInt16U)&CONTROL_ExtInfoCounter};
	pInt16U EPDatas_32[EP_COUNT_32] = {(pInt16U)CONTROL_ExtInfoData};
	
	// Data-table EPROM service configuration
	EPROMServiceConfig EPROMService = {&ZbMemory_WriteValuesEPROM, &ZbMemory_ReadValuesEPROM};
	
	// Init data table
	DT_Init(EPROMService, BadClockDetected);
	DT_SaveFirmwareInfo(DEVICE_CAN_ADDRESS, 0);
	// Fill state variables with default values
	CONTROL_FillWPPartDefault();
	
	// Device profile initialization
	DEVPROFILE_Init(&CONTROL_DispatchAction, &CycleActive);
	DEVPROFILE_InitEPService16(EPIndexes_16, EPSized_16, EPCounters_16, EPDatas_16);
	DEVPROFILE_InitEPService32(EPIndexes_32, EPSized_32, EPCounters_32, EPDatas_32);
	// Reset control values
	DEVPROFILE_ResetControlSection();
	
	CONTROL_InitStoragePointers();

	SM_ResetZeroPoint();
	ZwTimer_StartT1();

	// Sliding system init
	if(!BadClockDetected)
	{
		if(ZwSystem_GetDogAlarmFlag())
		{
			DataTable[REG_WARNING] = WARNING_WATCHDOG_RESET;
			ZwSystem_ClearDogAlarmFlag();
		}
		
		// Heating system init
		if(DataTable[REG_USE_HEATING])
		{
			TRMError dummy_error;
			
			// Terminate heating for sure
			TRM_Stop(TRM_CH1_ADDR, &dummy_error);
		}
	}
	else
	{
		CycleActive = TRUE;
		DataTable[REG_DISABLE_REASON] = DISABLE_BAD_CLOCK;
		CONTROL_SetDeviceState(DS_Disabled, DSS_None);
	}

}
// ----------------------------------------

void CONTROL_Idle()
{
	DEVPROFILE_ProcessRequests();
	DEVPROFILE_UpdateCANDiagStatus();
	
	CONTROL_UpdateTRMTemperature();

	DataTable[REG_SAFETY_SENSOR] = ZbGPIO_IsSafetySensorOk();
	DataTable[REG_HOMING_SENSOR] = ZbGPIO_HomeSensorActuate();
	DataTable[REG_BUS_TOOLING_SENSOR] = ZbGPIO_IsBusToolingSensorOk();
	DataTable[REG_ADAPTER_TOOLING_SENSOR] = ZbGPIO_IsAdapterToolingSensorOk();
	UpdatePressureOK();

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

#ifdef BOOT_FROM_FLASH
#pragma CODE_SECTION(CONTROL_UpdateLow, "ramfuncs");
#endif
void CONTROL_UpdateLow()
{
	CONTROL_HandleFanControl();
	CONTROL_HandleClampActions();
	ZbSU_UpdateTimeCounter(CONTROL_TimeCounter);
}
// ----------------------------------------

#ifdef BOOT_FROM_FLASH
#pragma CODE_SECTION(CONTROL_NotifyCANaFault, "ramfuncs");
#endif
void CONTROL_NotifyCANaFault(ZwCAN_SysFlags Flag)
{
	DEVPROFILE_NotifyCANaFault(Flag);
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
}
// ----------------------------------------

static void CONTROL_SetDeviceState(DeviceState NewState, DeviceSubState NewSubState)
{
	if(NewState == DS_Clamping || NewState == DS_Position)
		FanTimeout = CONTROL_TimeCounter + FAN_TIMEOUT;
	
	CONTROL_State = NewState;
	DataTable[REG_DEV_STATE] = NewState;

	CONTROL_SubState = NewSubState;
	DataTable[REG_DEV_SUBSTATE] = NewSubState;
}
// ----------------------------------------

static void CONTROL_HandleFanControl()
{
	ZbGPIO_SwitchFan((FanTimeout > CONTROL_TimeCounter) || HeatingActive);
}
// ----------------------------------------

static void CONTROL_HandleClampActions()
{
	static Int64U Timeout = 0;
	Boolean IsIGBTAdapterOk, IsBusClampOk, IsAdapterClampOk;

	switch(CONTROL_State)
	{
		case DS_Homing:
		case DS_Position:
		case DS_Clamping:
		case DS_ClampingRelease:
			if(DataTable[REG_USE_SAFETY_SENSOR] && !ZbGPIO_IsSafetySensorOk())
				CONTROL_Halt();
			break;
	}

	// Обработка общей логики отключения управления
	switch(CONTROL_SubState)
	{
		case DSS_Com_CheckControl:
			{
				// Раннее включение поджатия адаптера если зажимается прибор
				if(CONTROL_State == DS_Clamping)
					ZbGPIO_SwitchPowerConnection(TRUE);

				// Проверка состояния управления и пауза для разжатия
				if(ZbGPIO_IsControlConnected())
				{
					ZbGPIO_SwitchControlConnection(FALSE);

					Timeout = CONTROL_TimeCounter + PNEUMATIC_CTRL_PAUSE;
					CONTROL_SetDeviceState(CONTROL_State, DSS_Com_ControlRelease);
				}
				else
					CONTROL_SetDeviceState(CONTROL_State, DSS_Com_ReleaseDone);
			}
			break;

		case DSS_Com_ControlRelease:
			if(CONTROL_TimeCounter > Timeout)
				CONTROL_SetDeviceState(CONTROL_State, DSS_Com_ReleaseDone);
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
			}
			break;

		case DS_Position:
			switch(CONTROL_SubState)
			{
				case DSS_Com_ReleaseDone:
					CONTROL_PreparePositioning();
					CONTROL_SetDeviceState(CONTROL_State, DSS_PositionOperating);
					break;

				case DSS_PositionOperating:
					if(SM_IsPositioningDone())
						CONTROL_SetDeviceState(DS_Ready, DSS_None);
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
				{
					DUT_Type DUTType = (DUT_Type)DataTable[REG_CASE_THYRISTOR];
					IsBusClampOk = ZbGPIO_IsBusToolingSensorOk();
					IsAdapterClampOk = ZbGPIO_IsAdapterToolingSensorOk();

					if(!DataTable[REG_USE_TOOLING_SENSOR] || (IsBusClampOk && IsAdapterClampOk))
					{
						AdapterID = (DUTType == DT_Thyristor) ? DataTable[REG_DEV_CASE] : CONTROL_ReadIGBTAdapterID(&IsIGBTAdapterOk);

						if(DataTable[REG_DEV_CASE] == SC_Type_MIADAP && AdapterID == DataTable[REG_SERT_UPPER_ADAP_ID])
							AdapterID = SC_Type_MIADAP;

						if(!IsIGBTAdapterOk)
							CONTROL_SwitchToFault(FAULT_IGBT_ADAPTER_CONN);
						else
						{
							if ((DUTType == DT_Thyristor && (AdapterID & SC_TYPE_MASK)) || (DUTType == DT_IGBT && !(AdapterID & SC_TYPE_MASK)) ||
									(DUTType == DT_IGBT && AdapterID != DataTable[REG_DEV_CASE]))
							{
								DataTable[REG_PROBLEM] = PROBLEM_TOP_ADAPTER_MISMATCHED;
								ZbGPIO_SwitchPowerConnection(FALSE);
								CONTROL_SetDeviceState(DS_Ready, DSS_None);
								break;
							}
							else
							{
								CONTROL_PrepareClamping(TRUE);
								CONTROL_SetDeviceState(CONTROL_State, DSS_ClampingOperating);
							}
						}
					}
					else
						if(CONTROL_TimeCounter > Timeout)
							CONTROL_SwitchToFault(IsAdapterClampOk ? FAULT_BUS_SEN : FAULT_ADAPTER_SEN);
				}
				break;

				case DSS_ClampingOperating:
					if(SM_IsPositioningDone())
					{
						if(DataTable[REG_DEV_CASE] == SC_Type_C1 || DataTable[REG_DEV_CASE] == SC_Type_F1)
						{
							ClampingDuration = CONTROL_TimeCounter - ClampingDuration;
							HomingDuration = ReleaseDuration = 0;
							RequestSaveToFlash = TRUE;
							CONTROL_SetDeviceState(DS_ClampingDone, DSS_None);
						}
						else
						{
							ZbGPIO_SwitchControlConnection(TRUE);
							Timeout = CONTROL_TimeCounter + PNEUMATIC_CTRL_PAUSE;
							CONTROL_SetDeviceState(CONTROL_State, DSS_ClampingConnectControl);
						}
					}
					break;

				case DSS_ClampingConnectControl:
					if(CONTROL_TimeCounter > Timeout)
					{
						ClampingDuration = CONTROL_TimeCounter - ClampingDuration;
						HomingDuration = ReleaseDuration = 0;
						RequestSaveToFlash = TRUE;
						CONTROL_SetDeviceState(DS_ClampingDone, DSS_None);
					}
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
			}
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
			if(!CSAdapter_WriteID((Int16U*)&DataTable[REG_ADAPTER_ID]))
				*UserError = ERR_DEVICE_NOT_READY;
			ZbGPIO_CSMux(SPIMUX_EPROM);
			break;

		case ACT_ADAPTER_READ_ID:
			DS18B20_Init();
			if(!CSAdapter_ReadID((Int16U*)&DataTable[REG_ADAPTER_ID]))
				*UserError = ERR_DEVICE_NOT_READY;
			ZbGPIO_CSMux(SPIMUX_EPROM);
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
			
		case ACT_GOTO_POSITION:
			if(CONTROL_State == DS_Ready)
				CONTROL_SetDeviceState(DS_Position, DSS_Com_CheckControl);
			else
				*UserError = ERR_DEVICE_NOT_READY;
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
				ZbGPIO_SwitchPowerConnection(FALSE);
			else
				*UserError = ERR_OPERATION_BLOCKED;
			break;
			
		case ACT_HOLD_ADAPTER:
			if(CONTROL_State == DS_None || CONTROL_State == DS_Ready)
				ZbGPIO_SwitchPowerConnection(TRUE);

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
			
		case ACT_DBG_CONNECT_CONTROL:
			ZbGPIO_SwitchControlConnection(TRUE);
			break;
			
		case ACT_DBG_DISCONNECT_CONTROL:
			ZbGPIO_SwitchControlConnection(FALSE);
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
		Int16U MaxSpeed, Int16U LowSpeed, Int16U MinSpeed)
{
	SM_Config Config;
	Config.NewPosition = NewPosition;
	Config.SlowDownDistance = SlowDownDistance;
	Config.MaxSpeed = MaxSpeed;
	Config.LowSpeed = LowSpeed;
	Config.MinSpeed = MinSpeed;

	SM_GoToPosition(&Config);
}
// ----------------------------------------

void CONTROL_PrepareClamping(Boolean Clamp)
{
	if(Clamp)
	{
		Int16U Reg = 0;
		switch(DataTable[REG_DEV_CASE])
		{
			case SC_Type_A2:
				Reg = 0;
				break;
			case SC_Type_B1:
				Reg = 1;
				break;
			case SC_Type_C1:
				Reg = 2;
				break;
			case SC_Type_D0:
				Reg = 3;
				break;
			case SC_Type_E0:
				Reg = 4;
				break;
			case SC_Type_F1:
				Reg = 5;
				break;
			case SC_Type_ADAP:
				Reg = 6;
				break;

			case SC_Type_E2M:
				Reg = 7;
				break;
//
			case SC_Type_MIAA:
				Reg = 40;
				break;
			case SC_Type_MIDA:
				Reg = 41;
				break;
			case SC_Type_MIFA:
				Reg = 42;
				break;
			case SC_Type_MIHA:
				Reg = 43;
				break;
			case SC_Type_MIHM:
				Reg = 44;
				break;
			case SC_Type_MIHV:
				Reg = 45;
				break;
			case SC_Type_MISM:
				Reg = 46;
				break;
			case SC_Type_MISM2_CH:
				Reg = 47;
				break;
			case SC_Type_MISM2_SS_SD:
				Reg = 48;
				break;
			case SC_Type_MISV:
				Reg = 49;
				break;
			case SC_Type_MIXM:
				Reg = 50;
				break;
			case SC_Type_MIXV:
				Reg = 51;
				break;

			case SC_Type_MIADAP:
				Reg = 52;
				break;
		}

		CONTROL_PreparePositioningX(DataTable[Reg], DataTable[REG_SLOW_DOWN_DIST],
				DataTable[REG_CLAMP_SPEED_MAX], DataTable[REG_CLAMP_SPEED_LOW], DataTable[REG_CLAMP_SPEED_MIN]);
	}
	else
		CONTROL_PreparePositioningX(0, 0,
				DataTable[REG_CLAMP_SPEED_MAX], DataTable[REG_CLAMP_SPEED_LOW], DataTable[REG_CLAMP_SPEED_MIN]);
}
// ----------------------------------------

void CONTROL_PreparePositioning()
{
	CONTROL_PreparePositioningX(DataTable[REG_CUSTOM_POS], DataTable[REG_SLOW_DOWN_DIST],
			DataTable[REG_POS_SPEED_MAX], DataTable[REG_POS_SPEED_LOW], DataTable[REG_POS_SPEED_MIN]);
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

void CONTROL_PressureMeasuring(Int16U * const restrict pResults)
{
	Int32U Pressure = *(Int16U *)pResults;
	CSPressure = (Pressure * DataTable[REG_PRESSURE_K] / 1000) + DataTable[REG_PRESSURE_OFFSET];
	DataTable[REG_PRESSURE] = CSPressure;
}
// ----------------------------------------

void UpdatePressureOK()
{
	static Int64U PressureOkTime = 0;

	ZwADC_StartSEQ1();

	// Control Pressure
	ZwADC_SubscribeToResults1(&CONTROL_PressureMeasuring);

	if(CSPressure >= DataTable[REG_PRESSURE_OK])
		PressureOkTime = CONTROL_TimeCounter;

	if (CONTROL_TimeCounter > PressureOkTime + PNEUMATIC_READ_PAUSE)
	{
		DataTable[REG_DBG] = CSPressure;
		CONTROL_SwitchToFault(FAULT_PRESSURE);
	}
}
// ----------------------------------------

Int16U CONTROL_ReadIGBTAdapterID(pBoolean AdapterOk)
{
	pInt16U AdapterID = 0;

	DS18B20_Init();
	*AdapterOk = CSAdapter_ReadID(AdapterID);

	return *AdapterID;
}
// ----------------------------------------

void CONTROL_InitStoragePointers()
{
	STF_AssignPointer(0, (Int32U)&HomingDuration);
	STF_AssignPointer(1, (Int32U)&ClampingDuration);
	STF_AssignPointer(2, (Int32U)&ReleaseDuration);
}
//--------------------
