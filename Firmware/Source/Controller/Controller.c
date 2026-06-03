// Header
#include "Controller.h"
//
// Includes
#include "Board.h"
#include "Delay.h"
#include "DeviceProfile.h"
#include "Interrupts.h"
#include "LowLevel.h"
#include "SysConfig.h"
#include "DebugActions.h"
#include "Diagnostic.h"
#include "Logic.h"
#include "BCCIxParams.h"
#include "Constraints.h"

// Types
//
typedef void (*FUNC_AsyncDelegate)();

// Variables
//
volatile DeviceState CONTROL_State = DS_None, SavedState;
volatile DeviceSubState CONTROL_SubState = SS_None, SavedSubState;
static Boolean CycleActive = false;
//
volatile Int64U CONTROL_TimeCounter = 0;

// Forward functions
//
static Boolean CONTROL_DispatchAction(Int16U ActionID, pInt16U pUserError);
void CONTROL_UpdateWatchDog();
void CONTROL_ResetToDefaultState();
Int16U CONTROL_CSMPrepareLogic();
void CONTROL_ClampLogic();
void CONTROL_SamplePressureValue();

// Functions
//
void CONTROL_Init()
{
	// Конфигурация сервиса работы Data-table и EPROM
	EPROMServiceConfig EPROMService = {(FUNC_EPROM_WriteValues)&NFLASH_WriteDT, (FUNC_EPROM_ReadValues)&NFLASH_ReadDT};
	// Инициализация data table
	DT_Init(EPROMService, false);
	DT_SaveFirmwareInfo(CAN_NID, 0);
	// Инициализация device profile
	DEVPROFILE_Init(&CONTROL_DispatchAction, &CycleActive);
	
	// Сброс значений
	DEVPROFILE_ResetControlSection();
	CONTROL_ResetToDefaultState();
}
//------------------------------------------

void CONTROL_ResetToDefaultState()
{
	DataTable[REG_FAULT_REASON] = DF_NONE;
	DataTable[REG_DISABLE_REASON] = DF_NONE;
	DataTable[REG_WARNING] = WARNING_NONE;
	DataTable[REG_PROBLEM] = PROBLEM_NONE;
	DataTable[REG_OP_RESULT] = OPRESULT_NONE;

	CONTROL_SetDeviceState(DS_None, SS_None);
}
//------------------------------------------

void CONTROL_Idle()
{
	DEVPROFILE_ProcessRequests();
	
	LOGIC_UpdateSensors();
	CONTROL_SamplePressureValue();
	CONTROL_ClampLogic();
	
	CONTROL_UpdateWatchDog();
}
//------------------------------------------

static Boolean CONTROL_DispatchAction(Int16U ActionID, pInt16U pUserError)
{
	*pUserError = ERR_NONE;
	
	switch(ActionID)
	{
		case ACT_HOMING:
			if(CONTROL_State == DS_None)
				CONTROL_SetDeviceState(DS_Ready, SS_None);
			else if(CONTROL_State != DS_Ready)
				*pUserError = ERR_OPERATION_BLOCKED;
			break;
			
			
		case ACT_CLR_FAULT:
			if(CONTROL_State == DS_Fault)
			{
				CONTROL_SetDeviceState(DS_None, SS_None);
				DataTable[REG_FAULT_REASON] = DF_NONE;
			}
			break;
			
		case ACT_CLR_WARNING:
			DataTable[REG_WARNING] = WARNING_NONE;
			break;

		case ACT_CLR_HALT:
			if(CONTROL_State == DS_Halt)
				CONTROL_SetDeviceState(SavedState, SavedSubState);
			break;

		case ACT_CLAMP:
			if(CONTROL_State == DS_Ready)
			{
				DataTable[REG_PROBLEM] = PROBLEM_NONE;
				DataTable[REG_OP_RESULT] = OPRESULT_NONE;

				Int16U PrepareResult = CONTROL_CSMPrepareLogic();
				if(PrepareResult == PROBLEM_NONE)
					CONTROL_SetDeviceState(DS_Clamping, SS_BlockAdapters);
				else
				{
					DataTable[REG_PROBLEM] = PrepareResult;
					DataTable[REG_OP_RESULT] = OPRESULT_FAIL;
				}
			}
			else if(CONTROL_State == DS_Clamping)
				*pUserError = ERR_OPERATION_BLOCKED;
			else
				*pUserError = ERR_DEVICE_NOT_READY;
			break;
			
		case ACT_RELEASE:
			if(CONTROL_State == DS_ClampingDone)
			{
				DataTable[REG_OP_RESULT] = OPRESULT_NONE;
				CONTROL_SetDeviceState(DS_ClampingRelease, SS_StartRelease);
			}
			else if(CONTROL_State == DS_ClampingRelease)
				*pUserError = ERR_OPERATION_BLOCKED;
			else
				*pUserError = ERR_DEVICE_NOT_READY;
			break;

		case ACT_HALT:
			CONTROL_Halt();
			break;

		case ACT_CHECK_ADPTS_STATUS:
			DataTable[REG_TOP_ADPT_MISMATCHED] = (LOGIC_AdapterIDMatch(LL_MeasureIDTop()) == DataTable[REG_ID_ADPTR_SET]) ? 0 : 1;
			DataTable[REG_BOT_ADPT_MISMATCHED] = (LOGIC_AdapterIDMatch(LL_MeasureIDBot()) == DataTable[REG_ID_ADPTR_SET]) ? 0 : 1;
			break;

		case ACT_RELEASE_ADAPTER:
			LL_IndicateBlockAdapter(false);
			LL_HoldTopAdapter(false);
			DELAY_US(PNEUMO_DELAY);
			break;

		case ACT_HOLD_ADAPTER:
			if(CONTROL_State == DS_Ready)
			{
				LL_IndicateBlockAdapter(true);
				LL_HoldTopAdapter(true);
				DELAY_US(PNEUMO_DELAY);
			}
			else
				*pUserError = ERR_OPERATION_BLOCKED;
			break;

		default:
			return DIAG_HandleDiagnosticAction(ActionID, pUserError);
			
	}
	return true;
}
//-----------------------------------------------

Int16U CONTROL_CSMPrepareLogic()
{
	DUTType TopID = LOGIC_AdapterIDMatch(LL_MeasureIDTop());
	DUTType BotID = LOGIC_AdapterIDMatch(LL_MeasureIDBot());

	if(LL_GetStateLimitSwitchTopAdapter())
		return PROBLEM_TOP_ADAPTER_OPENED;

	else if(LL_GetStateLimitSwitchBotAdapter())
		return PROBLEM_BOT_ADAPTER_OPENED;

	else if(TopID != DataTable[REG_ID_ADPTR_SET])
		return PROBLEM_TOP_ADAPTER_MISMATCHED;

	else if(BotID != DataTable[REG_ID_ADPTR_SET])
		return PROBLEM_BOT_ADAPTER_MISMATCHED;

	else
		return PROBLEM_NONE;
}
//-----------------------------------------------

void CONTROL_ClampLogic()
{
	static Int64U Delay = 0;

	if(CONTROL_State == DS_Clamping || CONTROL_State == DS_ClampingRelease)
	{
		switch(CONTROL_SubState)
		{
			case SS_BlockAdapters:
				Delay = CONTROL_TimeCounter + PNEUMO_DELAY;
				LL_IndicateBlockCSM(true);
				LL_IndicateBlockAdapter(true);
				LL_HoldTopAdapter(true);
				LL_HoldBotAdapter(true);
				CONTROL_SetDeviceState(DS_Clamping, SS_BlockDelay);
				break;

			case SS_BlockDelay:
				if(CONTROL_TimeCounter > Delay)
				{
					Delay = CONTROL_TimeCounter + PNEUMO_DELAY;
					LL_ClampDUT(true);
					CONTROL_SetDeviceState(DS_Clamping, SS_ClampDelay);
				}
				break;

			case SS_ClampDelay:
				if(CONTROL_TimeCounter > Delay)
				{
					LL_SetSafetyOutput(true);
					DataTable[REG_OP_RESULT] = OPRESULT_OK;
					CONTROL_SetDeviceState(DS_ClampingDone, SS_None);
				}
				break;

			case SS_StartRelease:
				Delay = CONTROL_TimeCounter + PNEUMO_DELAY;
				LL_SetSafetyOutput(false);
				LL_ClampDUT(false);
				CONTROL_SetDeviceState(DS_ClampingRelease, SS_ReleaseDelay);
				break;

			case SS_ReleaseDelay:
				if(CONTROL_TimeCounter > Delay)
				{
					Delay = CONTROL_TimeCounter + PNEUMO_DELAY;
					LL_HoldBotAdapter(false);
					LL_IndicateBlockCSM(false);
					CONTROL_SetDeviceState(DS_ClampingRelease, SS_UnblockDelay);
				}
				break;
				
			case SS_UnblockDelay:
				if(CONTROL_TimeCounter > Delay)
				{
					DataTable[REG_OP_RESULT] = OPRESULT_OK;
					CONTROL_SetDeviceState(DS_Ready, SS_None);
				}
				break;
				
			default:
				break;
		}
	}
}
//-----------------------------------------------

void CONTROL_SamplePressureValue()
{
	static Int64U NextSampleTime = 0;
	static Int16U CounterErrPress = 0;

	if(CONTROL_TimeCounter > NextSampleTime)
	{
		NextSampleTime = PRESSURE_SAMPLE_PERIOD + CONTROL_TimeCounter;

		float Pressure = LL_MeasurePressure();
		DataTable[REG_PRESSURE_VALUE] = Pressure;

		if(CONTROL_State == DS_Ready || CONTROL_State == DS_Clamping ||
				CONTROL_State == DS_ClampingDone || CONTROL_State == DS_ClampingRelease)
		{
			if(Pressure < DataTable[REG_SET_PRESSURE_VALUE])
				CounterErrPress++;
			else
				CounterErrPress = 0;

			if(CounterErrPress > DataTable[REG_COUNTER_MAX_ERR_PRESS])
				CONTROL_SwitchToFault(DF_PRESSURE_ERROR_EXCEED);
		}
	}
}
//------------------------------------------

void CONTROL_SwitchToFault(Int16U Reason)
{
	LL_SetSafetyOutput(false);

	LL_IndicateBlockAdapter(false);
	LL_IndicateBlockCSM(false);

	LL_ClampDUT(false);
	LL_HoldTopAdapter(false);
	LL_HoldBotAdapter(false);

	CONTROL_SetDeviceState(DS_Fault, SS_None);
	DataTable[REG_FAULT_REASON] = Reason;
	DataTable[REG_OP_RESULT] = OPRESULT_FAIL;
}
//------------------------------------------

void CONTROL_SetDeviceState(DeviceState NewState, DeviceSubState NewSubState)
{
	CONTROL_State = NewState;
	DataTable[REG_DEV_STATE] = NewState;
	
	CONTROL_SubState = NewSubState;
	DataTable[REG_SUB_STATE] = NewSubState;
}
//------------------------------------------

void CONTROL_UpdateWatchDog()
{
	if(BOOT_LOADER_VARIABLE != BOOT_LOADER_REQUEST)
		IWDG_Refresh();
}
//------------------------------------------
void CONTROL_Halt()
{
	SavedState = CONTROL_State;
	SavedSubState = CONTROL_SubState;
	CONTROL_SetDeviceState(DS_Halt, SS_None);
}
// ----------------------------------------
