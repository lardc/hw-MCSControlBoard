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

// Variables
static xTimerAlterHandler AlterHandler = NULL;

static Int32S SM_GlobalStepsCounter = 0, SM_DestSteps = 0, SM_StartSteps = 0;
static Int16U SM_CyclesToToggle, SM_MinCycles, SM_MaxCycles;	// MinCycles - быстрый ход, MaxCycles - медленный ход
static Boolean SM_HomingFlag = FALSE, SM_RequestStopFlag = FALSE;

// Forward functions
void SM_LogicHandler();
Int16U SM_SpeedToHalfPeriod(Int16U Speed);
Int32U SM_PosToSteps(Int16U NewPos);
void SM_UpDirection(Boolean State);
void SM_ToggleHalfPeriodToTarget(Int16U Target);
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
	T3Ch4PWM_Stop();
	AlterHandler = (xTimerAlterHandler)Handler;
}
// ----------------------------------------

// Основной обработчик логики (вызывается на каждый шаг по прерыванию TIM3)
void SM_LogicHandler()
{
	Int32U StepsToGo, StepsTraveled;
	Int16U Target, AccelTarget, DecelTarget;

	if(SM_IsPositioningDone() && !SM_HomingFlag)
	{
		SM_StopMotion();
		return;
	}

	if(SM_HomingFlag)
	{
		// Условие завершения хоуминга
		if(LL_HomeSensorActuate())
		{
			SM_HomingFlag = FALSE;
			SM_DestSteps = SM_GlobalStepsCounter = 0;
			SM_StopMotion();
			return;
		}
	}
	else
	{
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
			DecelTarget = SM_MinCycles + (Int16U)((SM_MaxCycles - SM_MinCycles) * (SM_SPEED_CHANGE_STEPS - StepsToGo)
					/ SM_SPEED_CHANGE_STEPS);

			if(DecelTarget > Target)
				Target = DecelTarget;
		}

		SM_ToggleHalfPeriodToTarget(Target);
	}

	// Обработка запроса на остановку
	if(SM_RequestStopFlag)
	{
		SM_RequestStopFlag = FALSE;
		SM_HomingFlag = FALSE;
		SM_DestSteps = SM_GlobalStepsCounter;
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
	SM_RequestStopFlag = FALSE;

	SM_StartSteps = SM_GlobalStepsCounter;
	SM_DestSteps = SM_PosToSteps(Params->NewPosition);

	if(SM_DestSteps == SM_GlobalStepsCounter)
	{
		SM_StopMotion();
		return;
	}

	SM_UpDirection(SM_DestSteps > SM_GlobalStepsCounter);

	SM_MinCycles = SM_SpeedToHalfPeriod(Params->MaxSpeed);
	SM_MaxCycles = SM_SpeedToHalfPeriod(Params->MinSpeed);
	SM_CyclesToToggle = SM_MaxCycles;

	T3Ch4PWM_SetFrequency(SM_CyclesToToggle);
	T3Ch4PWM_Start();
}
// ----------------------------------------

// Хоуминг
void SM_Homing()
{
	SM_RequestStopFlag = FALSE;
	SM_HomingFlag = TRUE;
	SM_UpDirection(FALSE);
	SM_CyclesToToggle = SM_SpeedToHalfPeriod(DataTable[REG_HOMING_SPEED]);

	T3Ch4PWM_SetFrequency(SM_CyclesToToggle);
	T3Ch4PWM_Start();
}
// ----------------------------------------

// Хоуминг завершён?
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

// Перевод позиции, мм, в шаги
Int32U SM_PosToSteps(Int16U NewPos)
{
	return 1000ul * NewPos * SM_FULL_ROUND_STEPS / SM_MOVING_RER_ROUND;
}
// ----------------------------------------

// Перевод скорости, мм/с, в полупериод ШИМ
Int16U SM_SpeedToHalfPeriod(Int16U Speed)
{
	Int32U TimerClk, StepsPerSec, HalfPeriod, MaxHalfPeriod;

	if(Speed == 0)
		Speed = 1;

	TimerClk = SYSCLK / (TIM3->PSC + 1);
	StepsPerSec = (Int32U)Speed * 1000ul * SM_FULL_ROUND_STEPS / SM_MOVING_RER_ROUND;
	HalfPeriod = TimerClk / (2 * StepsPerSec);
	MaxHalfPeriod = (Int32U)(T3Ch4PWM_GetPWMBase() * T3CH4PWM_MAX_OUTPUT);

	if(HalfPeriod < 1)
		HalfPeriod = 1;
	else if(HalfPeriod > MaxHalfPeriod)
		HalfPeriod = MaxHalfPeriod;

	return (Int16U)HalfPeriod;
}
// ----------------------------------------

void SM_ResetZeroPoint()
{
	SM_HomingFlag = FALSE;
	SM_DestSteps = SM_GlobalStepsCounter = 0;
	SM_StopMotion();
}
// ----------------------------------------

// Плавное изменение скорости ШИМ
void SM_ToggleHalfPeriodToTarget(Int16U Target)
{
	static Int16U EnableCounter = 0;

	if(++EnableCounter > DataTable[REG_SM_TOGGLE_ACCELERATION])
	{
		EnableCounter = 0;

		if(SM_CyclesToToggle < Target)
			++SM_CyclesToToggle;
		else if(SM_CyclesToToggle > Target)
			--SM_CyclesToToggle;

		T3Ch4PWM_SetFrequency(SM_CyclesToToggle);
	}
}
// ----------------------------------------

// Остановка ШИМ
void SM_StopMotion()
{
	T3Ch4PWM_Stop();
}
// ----------------------------------------
