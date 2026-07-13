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

// Variables
//
static Boolean RequestStop = FALSE;
static Int16U StepDivisorLimit = 0, TicksMaxCounter = 0;

// Functions
//
static Int16U SMD_DivisorToHalfPeriod(Int16U Divisor)
{
	Int32U TimerClk, HalfPeriod, MaxHalfPeriod;

	if(Divisor == 0)
		Divisor = 1;

	// Коэффициент деления задан в тиках системного таймера 50 мкс
	TimerClk = SYSCLK / (TIM3->PSC + 1);
	HalfPeriod = (Int32U)Divisor * TIMER1_uS * TimerClk / 1000000ul;
	MaxHalfPeriod = (Int32U)(T3Ch4PWM_GetPWMBase() * 0.95f);

	if(HalfPeriod < 1)
		HalfPeriod = 1;
	else if(HalfPeriod > MaxHalfPeriod)
		HalfPeriod = MaxHalfPeriod;

	return (Int16U)HalfPeriod;
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
	T3Ch4PWM_SetDutyCycle(SMD_DivisorToHalfPeriod(StepDivisorLimit));
	T3Ch4PWM_Start();
}
// ----------------------------------------
