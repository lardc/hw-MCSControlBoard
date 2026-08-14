// ----------------------------------------
// SM driver module diag
// ----------------------------------------

// Header
//
#include "StepperMotorDiag.h"
#include "StepperMotor.h"
#include "DataTable.h"
#include "SysConfig.h"
#include "Timer3_Ch4PWM.h"
#include "Constraints.h"
#include "LowLevel.h"

// Variables
//
static Boolean RequestStop = FALSE;
static Int16U StepDivisorLimit = 0, TicksMaxCounter = 0;

// Functions
//
static Int16U SMD_DivisorToCycles(Int16U Divisor)
{
	Int32U TimerClk, Cycles, MaxCycles;

	if(Divisor == 0)
		Divisor = 1;

	// Коэффициент деления задан в тиках системного таймера 50 мкс
	TimerClk = SYSCLK / (TIM3->PSC + 1);
	Cycles = 2 * (Int32U)(Divisor * TIMER1_uS * (TimerClk * 0.000001));
	MaxCycles = T3Ch4PWM_GetMaxCycles();

	if(Cycles < 2)
		Cycles = 2;
	else if(Cycles > MaxCycles)
		Cycles = MaxCycles;

	return (Int16U)Cycles;
}
// ----------------------------------------

// Обработчик логики диагностики (вызывается на каждый шаг по прерыванию TIM3)
void SMD_LogicHandler()
{
	static Int16U TicksCounter = 0;

	++TicksCounter;

	if(RequestStop || (TicksMaxCounter != 0 && TicksCounter >= TicksMaxCounter))
	{
		TicksCounter = 0;
		RequestStop = FALSE;
		SM_ConnectAlterHandler(NULL);
	}
}
// ----------------------------------------

void SMD_RequstStop()
{
	RequestStop = TRUE;
}
// ----------------------------------------

void SMD_ConnectHandler()
{
	StepDivisorLimit = DataTable[REG_DBG_STEP_DIV];
	TicksMaxCounter = DataTable[REG_DBG_STEPS_MAX];

	SM_ConnectAlterHandler(&SMD_LogicHandler);
	LL_SetStepperEnable(true);
	T3Ch4PWM_SetPeriodTicks(SMD_DivisorToCycles(StepDivisorLimit));
	T3Ch4PWM_Start();
}
// ----------------------------------------

Boolean SMD_GoToDistanceMm(Int16U PositionMm)
{
	SM_Params Params;

	if(SM_IsBusy())
		return FALSE;

	if(PositionMm > POS_MAX)
		return FALSE;

	if((Int16U)DataTable[REG_POS_SPEED_MIN] > (Int16U)DataTable[REG_POS_SPEED_MAX])
		return FALSE;

	SM_Config(&Params, PositionMm);
	return SM_GoToPosition(&Params);
}
// ----------------------------------------
