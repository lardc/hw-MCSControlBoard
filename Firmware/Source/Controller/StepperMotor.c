// ----------------------------------------
// SM driver module
// ----------------------------------------

// Header
#include "StepperMotor.h"
#include "DeviceObjectDictionary.h"
#include "DataTable.h"

// Includes
#include "SysConfig.h"
#include "Global.h"

// Definitions
#define SM_SPEED_CHANGE_STEPS		(2 * SM_FULL_ROUND_STEPS)	// Acceleration in steps
#define SM_STEPS_RESERVE			10			// Safety area of steps to destination position

// Types
typedef void (*xTimerAlterHandler)();

// Variables
static xTimerAlterHandler AlterHandler = NULL;

static Int32S SM_GlobalStepsCounter = 0, SM_DestSteps = 0;
static Int16U SM_LowSpeedSteps, SM_CyclesToToggle, SM_LowSpeedCycles, SM_MinCycles, SM_MaxCycles;
static Boolean SM_HomingFlag = FALSE, SM_RequestStopFlag = FALSE, SM_SafetyEvent = FALSE;

// Forward functions
void SM_LogicHandler();
Int16U SM_SpeedToCycles(Int16U Speed);
Int32U SM_PosToSteps(Int16U NewPos);
void SM_UpDirection(Boolean State);
void SM_ToggleCyclesToTarget(Int16U Target);

// Functions
//
// Timer 1 ISR
ISRCALL Timer1_ISR()
{
	if(AlterHandler)
	{
		xTimerAlterHandler AlterHandlerCopy = AlterHandler;
		AlterHandlerCopy();
	}
	else
		SM_LogicHandler();

	// no PIE
	TIMER1_ISR_DONE;
}
// -----------------------------------------

// Connect alter handler for timer processing
void SM_ConnectAlterHandler(void *Handler)
{
	AlterHandler = (xTimerAlterHandler)Handler;
}
// ----------------------------------------

// Main logic handler
void SM_LogicHandler()
{
	static Boolean TickHigh = FALSE;
	static Int16U SM_CycleCounter = 0;

	if(!SM_IsPositioningDone() || SM_HomingFlag)
	{
		// Генератор шагов
		if(++SM_CycleCounter >= SM_CyclesToToggle)
		{
			SM_CycleCounter = 0;
			ZbGPIO_SwitchStep(TickHigh = !TickHigh);

			// Счёт для перемещения по нарастающему фронту тиков
			if(TickHigh)
			{
				if(SM_HomingFlag)
				{
					// Условие завершения хоуминга
					if(ZbGPIO_HomeSensorActuate())
					{
						SM_HomingFlag = FALSE;
						SM_DestSteps = SM_GlobalStepsCounter = 0;
					}
				}
				else
				{
					// Проверка условия позиционирования
					SM_GlobalStepsCounter += (ZbGPIO_IsDirUp()) ? 1 : -1;
					Int32U StepsToPos = abs(SM_DestSteps - SM_GlobalStepsCounter);

					// 1-2. acceleration to Vmax or running at Vmax
					if(StepsToPos > SM_SPEED_CHANGE_STEPS + SM_LowSpeedSteps + SM_STEPS_RESERVE)
						SM_ToggleCyclesToTarget(SM_MinCycles);

					// 3-4. acceleration or deceleration to Vend
					else if(StepsToPos > (Int32U)SM_SPEED_CHANGE_STEPS * SM_MinCycles / SM_LowSpeedCycles + SM_STEPS_RESERVE)
						SM_ToggleCyclesToTarget(SM_LowSpeedCycles);

					// 5. deceleration to Vmin
					else
						SM_ToggleCyclesToTarget(SM_MaxCycles);
				}
			}

			// По спаду фронта обработка запросов на остановку
			else if(SM_RequestStopFlag)
			{
				SM_RequestStopFlag = FALSE;
				SM_HomingFlag = FALSE;
				SM_DestSteps = SM_GlobalStepsCounter;
				SM_SafetyEvent = TRUE;
			}
		}
	}
}
// -----------------------------------------

// Up or down direction
void SM_UpDirection(Boolean State)
{
	ZbGPIO_SwitchUpDir(State);
}
// ----------------------------------------

// Steps Enable
void SM_Enable(Boolean State)
{
	ZbGPIO_SwitchEnable(!State);
}
// ----------------------------------------

// New position in mm, speed in mm/s
void SM_GoToPosition(pSM_Config Config)
{
	SM_RequestStopFlag = FALSE;

	SM_LowSpeedSteps = SM_PosToSteps(Config->SlowDownDistance);
	SM_DestSteps = SM_PosToSteps(Config->NewPosition);

	SM_UpDirection(SM_DestSteps > SM_GlobalStepsCounter);

	SM_MinCycles = SM_SpeedToCycles(Config->MaxSpeed);
	SM_LowSpeedCycles = SM_SpeedToCycles(Config->LowSpeed);
	SM_CyclesToToggle = SM_MaxCycles = SM_SpeedToCycles(Config->MinSpeed);
}
// ----------------------------------------

// Homing
void SM_Homing(Int16U HomingSpeed)
{
	SM_SafetyEvent = FALSE;
	SM_RequestStopFlag = FALSE;
	SM_HomingFlag = TRUE;
	SM_UpDirection(FALSE);
	SM_CyclesToToggle = SM_SpeedToCycles(HomingSpeed);
}
// ----------------------------------------

Boolean SM_IsSafetyEvent()
{
	return SM_SafetyEvent;
}
// ----------------------------------------

// Is homing done?
Boolean SM_IsHomingDone()
{
	return !SM_HomingFlag && SM_IsPositioningDone();
}
// ----------------------------------------

Boolean SM_IsPositioningDone()
{
	return SM_DestSteps == SM_GlobalStepsCounter;
}
// ----------------------------------------

void SM_RequestStop()
{
	SM_RequestStopFlag = TRUE;
}
// ----------------------------------------

// Position in mm to steps converter
Int32U SM_PosToSteps(Int16U NewPos)
{
	return 1000ul * NewPos * SM_FULL_ROUND_STEPS / SM_MOVING_RER_ROUND;
}
// ----------------------------------------

// Speed in mm/s to cycles to toggle raw converter
Int16U SM_SpeedToCycles(Int16U Speed)
{
	Int32U res = 1000ul * SM_MOVING_RER_ROUND / TIMER1_PERIOD / SM_FULL_ROUND_STEPS / Speed;
	return (res == 0) ? 1 : res;
}
// ----------------------------------------

void SM_ResetZeroPoint()
{
	SM_HomingFlag = FALSE;
	SM_DestSteps = SM_GlobalStepsCounter = 0;
}
// ----------------------------------------

void SM_ToggleCyclesToTarget(Int16U Target)
{
	static Int16U EnableCounter = 0;

	if(++EnableCounter > DataTable[REG_SM_TOGGLE_ACCELERATION])
	{
		EnableCounter = 0;

		if(SM_CyclesToToggle < Target)
			++SM_CyclesToToggle;
		else if(SM_CyclesToToggle > Target)
			--SM_CyclesToToggle;
	}
}
// ----------------------------------------
