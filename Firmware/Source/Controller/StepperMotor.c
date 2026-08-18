// ----------------------------------------
// SM driver module
// ----------------------------------------

// Header
#include "StepperMotor.h"
#include "DataTable.h"

// Includes
#include "SysConfig.h"
#include "Global.h"
#include "Controller.h"
#include "Timer3_Ch4PWM.h"
#include <stdlib.h>

// Types
typedef void (*xTimerAlterHandler)();

typedef enum __MotorState
{
	MS_None	= 0,
	MS_Stop = 1,
	MS_HomingSearch = 2,
	MS_HomingPause = 3,
	MS_HomingRelease = 4,
	MS_Movement = 5
} MotorState;

static volatile MotorState Motor_State = MS_None;

// Variables
static xTimerAlterHandler AlterHandler = NULL;

static volatile Boolean SM_HomingDoneFlag = FALSE;
static volatile Int32S SM_GlobalStepsCounter = 0, SM_DestSteps = 0, SM_StartSteps = 0;
static volatile Int16U SM_CyclesToToggle;
static Int16U SM_MinCycles, SM_MaxCycles;	// MinCycles — быстрый ход, MaxCycles — медленный ход
static Int32U SM_SpeedChangeSteps = 0;
static Int64U SM_HomingPauseDeadline = 0;
volatile Int16U SM_LogScaleCoef = 1;
volatile Int16U SM_LogScaleCounter = 1;
static Boolean SM_LogContinue = FALSE;

// Forward functions
void SM_LogicHandler();
Int16U SM_SpeedToCycles(float SpeedMmS);
Int32U SM_PosToSteps(Int16U NewPos);
Int32U SM_EstimateMoveMs(Int32U DistSteps);
static void SM_MotorLogStart(Int32U ExpectedMs);
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
	SM_HomingDoneFlag = FALSE;
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
			SM_GlobalStepsCounter += (LL_IsDirUp()) ? 1 : -1;
			if(LL_HomeSensorActuate())
			{
				SM_HomingPauseDeadline = CONTROL_TimeCounter + HOMING_REVERSE_PAUSE;
				T3Ch4PWM_SetPeriodTicks(0);
				Motor_State = MS_HomingPause;
			}
			break;
		case MS_HomingPause:
			if(CONTROL_TimeCounter > SM_HomingPauseDeadline)
			{
				SM_UpDirection(TRUE);
				Motor_State = MS_HomingRelease;
				T3Ch4PWM_SetPeriodTicks(SM_CyclesToToggle);
			}
			break;
		case MS_HomingRelease:
			SM_GlobalStepsCounter += (LL_IsDirUp()) ? 1 : -1;
			if(!LL_HomeSensorActuate())
			{
				SM_HomingDoneFlag = TRUE;
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

			if(SM_SpeedChangeSteps > 0 && StepsTraveled <= SM_SpeedChangeSteps)
			{
				AccelTarget = SM_MaxCycles
						- (Int16U)((SM_MaxCycles - SM_MinCycles) * StepsTraveled / SM_SpeedChangeSteps);

				if(AccelTarget > Target)
					Target = AccelTarget;
			}

			if(SM_SpeedChangeSteps > 0 && StepsToGo <= SM_SpeedChangeSteps)
			{
				DecelTarget = SM_MinCycles
						+ (Int16U)((SM_MaxCycles - SM_MinCycles) * (SM_SpeedChangeSteps - StepsToGo)
								/ SM_SpeedChangeSteps);

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
	Params->MinSpeed = DataTable[REG_POS_SPEED_MIN];
}
// ----------------------------------------

// Переход в новую позицию, мм; скорости в мм/с
Boolean SM_GoToPosition(pSM_Params Params)
{
	Int16U SlowDownDist, AccelLimit;
	Int32U SpeedChangeStepsByDist, SpeedChangeStepsByAccel, TotalMoveSteps;
	Boolean ContinueLog = SM_LogContinue;

	SM_HomingDoneFlag = FALSE;
	SM_LogContinue = FALSE;

	if(Params->MinSpeed > Params->MaxSpeed)
	{
		SM_StopMotion();
		return FALSE;
	}

	SM_StartSteps = SM_GlobalStepsCounter;
	SM_DestSteps = SM_PosToSteps(Params->NewPosition);

	if(SM_DestSteps == SM_GlobalStepsCounter)
	{
		SM_StopMotion();
		return TRUE;
	}

	SM_UpDirection(SM_DestSteps > SM_GlobalStepsCounter);

	SM_MinCycles = SM_SpeedToCycles(Params->MaxSpeed);
	SM_MaxCycles = SM_SpeedToCycles(Params->MinSpeed);
	SM_CyclesToToggle = SM_MaxCycles;
	SlowDownDist = DataTable[REG_SLOW_DOWN_DIST];
	SpeedChangeStepsByDist = SM_PosToSteps(SlowDownDist);
	AccelLimit = DataTable[REG_SM_TOGGLE_ACCELERATION];
	if(AccelLimit == 0)
		AccelLimit = 1;

	SpeedChangeStepsByAccel = (SM_MaxCycles - SM_MinCycles + AccelLimit - 1) / AccelLimit;
	SM_SpeedChangeSteps = (SpeedChangeStepsByDist > SpeedChangeStepsByAccel)
			? SpeedChangeStepsByDist
			: SpeedChangeStepsByAccel;

	TotalMoveSteps = abs(SM_DestSteps - SM_GlobalStepsCounter);
	if(SM_SpeedChangeSteps > (TotalMoveSteps / 2))
		SM_SpeedChangeSteps = TotalMoveSteps / 2;

	LL_SetStepperEnable(true);
	T3Ch4PWM_SetPeriodTicks(SM_CyclesToToggle);
	T3Ch4PWM_Start();
	if(!ContinueLog)
		SM_MotorLogStart(SM_EstimateMoveMs((Int32U)abs(SM_DestSteps - SM_StartSteps)));
	Motor_State = MS_Movement;
	return TRUE;
}
// ----------------------------------------

// Хоуминг
void SM_Homing()
{
	Int32U OffsetMs;
	Int16U OffsetMm;

	SM_HomingDoneFlag = FALSE;
	SM_LogContinue = TRUE;
	SM_MinCycles = SM_MaxCycles = SM_SpeedToCycles(DataTable[REG_HOMING_SPEED]);
	SM_SpeedChangeSteps = 0;
	OffsetMm = (Int16U)DataTable[REG_HOMING_OFFSET];
	OffsetMs = SM_EstimateMoveMs(SM_PosToSteps(OffsetMm));
	SM_MotorLogStart(HOMING_TIMEOUT + HOMING_PAUSE + OffsetMs);
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
	SM_CyclesToToggle = SM_MinCycles;

	LL_SetStepperEnable(true);
	T3Ch4PWM_SetPeriodTicks(SM_CyclesToToggle);
	T3Ch4PWM_Start();
}
// ----------------------------------------

Boolean SM_IsBusy()
{
	return Motor_State != MS_None;
}
// ----------------------------------------

Boolean SM_IsLogging()
{
	return SM_IsBusy() || SM_LogContinue;
}
// ----------------------------------------

float SM_GetPositionMm()
{
	return (float)SM_GlobalStepsCounter * (float)SM_MOVING_RER_ROUND
			/ (1000.0f * (float)SM_FULL_ROUND_STEPS);
}
// ----------------------------------------

float SM_GetSpeedMmS()
{
	Int32U TimerClk, StepsPerSec;
	float Speed;

	if(Motor_State == MS_None || Motor_State == MS_Stop || Motor_State == MS_HomingPause
			|| SM_CyclesToToggle == 0)
		return 0.0f;

	TimerClk = SYSCLK / (TIM3->PSC + 1);
	StepsPerSec = TimerClk / SM_CyclesToToggle;
	Speed = (float)StepsPerSec * (float)SM_MOVING_RER_ROUND
			/ (1000.0f * (float)SM_FULL_ROUND_STEPS);

	return LL_IsDirUp() ? Speed : -Speed;
}
// ----------------------------------------

// Хоуминг завершён?
Boolean SM_IsHomingDone()
{
	return (Motor_State == MS_None) && SM_HomingDoneFlag;
}
// ----------------------------------------

Boolean SM_IsPositioningDone()
{
	return SM_DestSteps == SM_GlobalStepsCounter;
}
// ----------------------------------------

void SM_RequestStop()
{
	SM_HomingDoneFlag = FALSE;
	SM_LogContinue = FALSE;
	Motor_State = MS_Stop;
}
// ----------------------------------------

// Перевод позиции, мм, в шаги
Int32U SM_PosToSteps(Int16U NewPos)
{
	return 1000ul * NewPos * SM_FULL_ROUND_STEPS / SM_MOVING_RER_ROUND;
}
// ----------------------------------------

// Оценка длительности хода по трапеции разгон/круиз/торможение, мс
Int32U SM_EstimateMoveMs(Int32U DistSteps)
{
	Int32U TimerClk, AccelSteps, CruiseSteps;
	Int64U TotalCycles;

	if(DistSteps == 0)
		return 1;

	TimerClk = SYSCLK / (TIM3->PSC + 1);
	if(TimerClk == 0)
		TimerClk = 1;

	AccelSteps = SM_SpeedChangeSteps;
	if((AccelSteps * 2) > DistSteps)
		AccelSteps = DistSteps / 2;
	CruiseSteps = DistSteps - AccelSteps * 2;

	TotalCycles = (Int64U)CruiseSteps * SM_MinCycles
			+ (Int64U)AccelSteps * ((Int32U)SM_MaxCycles + SM_MinCycles);
	if(TotalCycles == 0)
		return 1;

	return (Int32U)((TotalCycles * 1000 + TimerClk - 1) / TimerClk);
}
// ----------------------------------------

static void SM_MotorLogStart(Int32U ExpectedMs)
{
	Int32U ExpectedTicks, Coef;

	if(ExpectedMs == 0)
		ExpectedMs = 1;

	ExpectedTicks = ExpectedMs * 1000ul / TIMER7_uS;
	if(ExpectedTicks == 0)
		ExpectedTicks = 1;

	Coef = (ExpectedTicks + VALUES_x_SIZE - 1) / VALUES_x_SIZE;
	if(Coef == 0)
		Coef = 1;
	else if(Coef > INT16U_MAX)
		Coef = INT16U_MAX;

	SM_LogScaleCoef = (Int16U)Coef;
	SM_LogScaleCounter = 1;
	CONTROL_ValuesCounter = 0;
	DataTable[REG_DEBUG_SCALING_COEF] = SM_LogScaleCoef;
	DEVPROFILE_ResetEPReadState();

	for(Int16U i = 0; i < VALUES_x_SIZE; ++i)
	{
		CONTROL_MotorMovement[i] = 0;
		CONTROL_MotorSpeed[i] = 0;
	}
}
// ----------------------------------------

// Перевод скорости, мм/с, в полный период ШИМ
Int16U SM_SpeedToCycles(float SpeedMmS)
{
	Int32U TimerClk, StepsPerSec, Cycles, MaxCycles;

	if(SpeedMmS <= 0.0f)
		SpeedMmS = 0.1f;

	TimerClk = SYSCLK / (TIM3->PSC + 1);
	StepsPerSec = (Int32U)(SpeedMmS * 1000.0f * (float)SM_FULL_ROUND_STEPS
			/ (float)SM_MOVING_RER_ROUND + 0.5f);
	if(StepsPerSec == 0)
		StepsPerSec = 1;

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
	SM_HomingDoneFlag = FALSE;
	SM_LogContinue = FALSE;
	SM_DestSteps = SM_GlobalStepsCounter = 0;
	SM_StopMotion();
}
// ----------------------------------------

// Установка периода ШИМ по целевому профилю скорости
void SM_ToggleCyclesToTarget(Int16U Target)
{
	if(SM_CyclesToToggle == Target)
		return;

	SM_CyclesToToggle = Target;
	T3Ch4PWM_SetPeriodTicks(SM_CyclesToToggle);
}
// ----------------------------------------

// Остановка ШИМ
void SM_StopMotion()
{
	T3Ch4PWM_Stop();
	LL_SetStepperEnable(false);
	Motor_State = MS_None;
}
// ----------------------------------------
