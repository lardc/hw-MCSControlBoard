// ----------------------------------------
// SM driver module
// ----------------------------------------

// Header
#include "StepperMotor.h"
#include "DataTable.h"

// Includes
#include "SysConfig.h"
#include "Global.h"
#include "Timer3_Ch4PWM.h"
#include <stdlib.h>

// Definitions
#define SM_SPEED_CHANGE_STEPS		(2 * SM_FULL_ROUND_STEPS)	// Длина линейного разгона/торможения, шаги

// Types
typedef void (*xTimerAlterHandler)();

typedef enum __MotorState
{
	MS_None	= 0,
	MS_Stop = 1,
	MS_HomingSearch = 2,
	MS_HomingRelease = 3,
	MS_Movement = 4
} MotorState;

static volatile MotorState Motor_State = MS_None;

// Variables
static xTimerAlterHandler AlterHandler = NULL;

static volatile Int32S SM_GlobalStepsCounter = 0, SM_DestSteps = 0, SM_StartSteps = 0;
static Int16U SM_CyclesToToggle, SM_MinCycles, SM_MaxCycles;	// MinCycles — быстрый ход, MaxCycles — медленный ход

// Forward functions
void SM_LogicHandler();
Int16U SM_SpeedToCycles(Int16U Speed);
Int32U SM_PosToSteps(Int16U NewPos);
void SM_UpDirection(Boolean State);
void SM_ToggleCyclesToTarget(Int16U Target);
void SM_StopMotion();

// Functions
//
void SM_TimerHandler()
{
	if(AlterHandler)
	{
		xTimerAlterHandler AlterHandlerCopy = AlterHandler;
		AlterHandlerCopy();
	}
	else
		SM_LogicHandler();
}
// -----------------------------------------

// Подключение альтернативного обработчика таймера (диагностика)
void SM_ConnectAlterHandler(void *Handler)
{
	SM_StopMotion();
	AlterHandler = (xTimerAlterHandler)Handler;

}
// ----------------------------------------

// Основной обработчик логики (вызывается на каждый шаг по прерыванию TIM3)
void SM_LogicHandler()
{
	Int32U StepsToGo, StepsTraveled;
	Int16U Target, AccelTarget, DecelTarget;
	switch(Motor_State)
	{
		case MS_Stop:
			SM_DestSteps = SM_GlobalStepsCounter;
			SM_StopMotion();
			break;
		case MS_HomingSearch:
			if(LL_HomeSensorActuate())
			{
				SM_UpDirection(TRUE);
				Motor_State = MS_HomingRelease;
			}
			break;
		case MS_HomingRelease:
			if(!LL_HomeSensorActuate())
			{
				SM_DestSteps = SM_GlobalStepsCounter = 0;
				SM_StopMotion();
				return;
			}
			break;
		case MS_Movement:
			// Счёт шагов позиционирования
			SM_GlobalStepsCounter += (LL_IsDirUp()) ? 1 : -1;

			if(SM_IsPositioningDone())
			{
				SM_StopMotion();
				return;
			}

			// Линейный разгон и торможение
			StepsToGo = abs(SM_DestSteps - SM_GlobalStepsCounter);
			StepsTraveled = abs(SM_GlobalStepsCounter - SM_StartSteps);
			Target = SM_MinCycles;

			if(StepsTraveled < SM_SPEED_CHANGE_STEPS)
			{
				AccelTarget = SM_MaxCycles
						- (Int16U)((SM_MaxCycles - SM_MinCycles) * StepsTraveled / SM_SPEED_CHANGE_STEPS);

				if(AccelTarget > Target)
					Target = AccelTarget;
			}

			if(StepsToGo <= SM_SPEED_CHANGE_STEPS)
			{
				DecelTarget = SM_MinCycles
						+ (Int16U)((SM_MaxCycles - SM_MinCycles) * (SM_SPEED_CHANGE_STEPS - StepsToGo)
								/ SM_SPEED_CHANGE_STEPS);

				if(DecelTarget > Target)
					Target = DecelTarget;
			}

			SM_ToggleCyclesToTarget(Target);
			break;

		default:
			SM_StopMotion();
			break;
	}
}
// -----------------------------------------

// Направление вращения
void SM_UpDirection(Boolean State)
{
	LL_SwitchUpDir(State);
}
// ----------------------------------------

// Заполнение профиля скоростей (рег. 12, 16)
void SM_Config(pSM_Params Params, Int16U PositionMm)
{
	Params->NewPosition = PositionMm;
	Params->MaxSpeed = DataTable[REG_POS_SPEED_MAX];
	Params->MinSpeed = DataTable[REG_CLAMP_SPEED_MIN];
}
// ----------------------------------------

// Переход в новую позицию, мм; скорости в мм/с
void SM_GoToPosition(pSM_Params Params)
{
	SM_StartSteps = SM_GlobalStepsCounter;
	SM_DestSteps = SM_PosToSteps(Params->NewPosition);

	if(SM_DestSteps == SM_GlobalStepsCounter)
	{
		SM_StopMotion();
		return;
	}

	SM_UpDirection(SM_DestSteps > SM_GlobalStepsCounter);

	SM_MinCycles = SM_SpeedToCycles(Params->MaxSpeed);
	SM_MaxCycles = SM_SpeedToCycles(Params->MinSpeed);
	SM_CyclesToToggle = SM_MaxCycles;

	T3Ch4PWM_SetPeriodTicks(SM_CyclesToToggle);
	T3Ch4PWM_Start();
	Motor_State = MS_Movement;
}
// ----------------------------------------

// Хоуминг
void SM_Homing()
{
	if(LL_HomeSensorActuate())
	{
		SM_UpDirection(TRUE);
		Motor_State = MS_HomingRelease;
	}
	else
	{
		SM_UpDirection(FALSE);
		Motor_State = MS_HomingSearch;
	}
	SM_CyclesToToggle = SM_SpeedToCycles(DataTable[REG_HOMING_SPEED]);

	T3Ch4PWM_SetPeriodTicks(SM_CyclesToToggle);
	T3Ch4PWM_Start();
}
// ----------------------------------------

// Хоуминг завершён?
Boolean SM_IsHomingDone()
{
	return (Motor_State == MS_None) && SM_IsPositioningDone();
}
// ----------------------------------------

Boolean SM_IsPositioningDone()
{
	return SM_DestSteps == SM_GlobalStepsCounter;
}
// ----------------------------------------

void SM_RequestStop()
{
	Motor_State = MS_Stop;
}
// ----------------------------------------

// Перевод позиции, мм, в шаги
Int32U SM_PosToSteps(Int16U NewPos)
{
	return 1000ul * NewPos * SM_FULL_ROUND_STEPS / SM_MOVING_RER_ROUND;
}
// ----------------------------------------

// Перевод скорости, мм/с, в полный период ШИМ
Int16U SM_SpeedToCycles(Int16U Speed)
{
	Int32U TimerClk, StepsPerSec, Cycles, MaxCycles;

	if(Speed == 0)
		Speed = 1;

	TimerClk = SYSCLK / (TIM3->PSC + 1);
	StepsPerSec = (Int32U)Speed * 1000ul * SM_FULL_ROUND_STEPS / SM_MOVING_RER_ROUND;
	Cycles = TimerClk / StepsPerSec;
	MaxCycles = T3Ch4PWM_GetMaxCycles();

	if(Cycles < 2)
		Cycles = 2;
	else if(Cycles > MaxCycles)
		Cycles = MaxCycles;

	return (Int16U)Cycles;
}
// ----------------------------------------

void SM_ResetZeroPoint()
{
	SM_DestSteps = SM_GlobalStepsCounter = 0;
	SM_StopMotion();
}
// ----------------------------------------

// Плавное изменение скорости ШИМ
void SM_ToggleCyclesToTarget(Int16U Target)
{
	static Int16U EnableCounter = 0;
	// Сброс при достижении целевого периода
	if(SM_CyclesToToggle == Target)
	{
		EnableCounter = 0;
		return;
	}

	if(++EnableCounter > DataTable[REG_SM_TOGGLE_ACCELERATION])
	{
		EnableCounter = 0;

		if(SM_CyclesToToggle < Target)
			++SM_CyclesToToggle;
		else if(SM_CyclesToToggle > Target)
			--SM_CyclesToToggle;

		T3Ch4PWM_SetPeriodTicks(SM_CyclesToToggle);
	}
}
// ----------------------------------------

// Остановка ШИМ
void SM_StopMotion()
{
	T3Ch4PWM_Stop();
	Motor_State = MS_None;
}
// ----------------------------------------
