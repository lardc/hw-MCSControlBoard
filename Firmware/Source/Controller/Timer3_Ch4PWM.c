// Header
#include "Timer3_Ch4PWM.h"

// Includes
#include "SysConfig.h"
#include "ZwTIM.h"
#include "ZwRCC.h"

#define T3CH4PWM_UPDATE_TIMEOUT_LOOPS	(2ul * SYSCLK)

// Variables
static uint32_t PWMBase = 0;

// Functions
static Boolean T3Ch4PWM_WaitUpdateFlag()
{
	uint32_t Timeout = T3CH4PWM_UPDATE_TIMEOUT_LOOPS;

	while(!(TIM3->SR & TIM_SR_UIF) && Timeout > 0)
		--Timeout;

	return (TIM3->SR & TIM_SR_UIF) != 0;
}

static void T3Ch4PWM_ApplyPeriod(uint32_t PeriodTicks)
{
	if(PeriodTicks < 2)
		PeriodTicks = 2;

	TIM3->ARR = PeriodTicks - 1;
	TIM3->CCR4 = PeriodTicks / 2;
}
//------------------------------------------------

void T3Ch4PWM_Init(uint32_t SystemClock, uint32_t Period)
{
	// Расчёт размерности ШИМ
	uint32_t Prescaler = (uint32_t)((float)SystemClock / 1000000 * Period / 65536);
	PWMBase = (uint32_t)((SystemClock / ((Prescaler + 1) * 1000000)) * Period);

	// Стандартная инициализация
	TIM_Clock_En(TIM_3);

	TIM3->PSC = Prescaler;
	TIM3->ARR = PWMBase - 1;
	TIM3->CR1 |= TIM_CR1_ARPE;

	// Канал 4 - STEP шагового двигателя
	// Режим ШИМ - PWM mode 1, c функцией Preload
	TIM3->CCMR2 = (TIM3->CCMR2 & ~TIM_CCMR2_OC4M) |
			(TIM_CCMR2_OC4M_2 | TIM_CCMR2_OC4M_1 | TIM_CCMR2_OC4PE);
	TIM3->CCER |= TIM_CCER_CC4E;
	TIM3->CCR4 = 0;

	// Сброс выхода
	T3Ch4PWM_SetPeriodTicks(0);

	// Инициализация обновления регистров
	TIM3->SR = ~TIM_SR_UIF;
	TIM3->EGR |= TIM_EGR_UG;
	T3Ch4PWM_WaitUpdateFlag();
	TIM3->SR = ~TIM_SR_UIF;

	TIM_Interupt(TIM3, 0, true);
	TIM3->DIER &= ~(TIM_DIER_UIE | TIM_DIER_CC4IE);
}
//------------------------------------------------

uint32_t T3Ch4PWM_GetMaxCycles()
{
	uint32_t MaxCycles = (uint32_t)(PWMBase * T3CH4PWM_MAX_OUTPUT);
	return (MaxCycles > 65535ul) ? 65535ul : MaxCycles;
}
//------------------------------------------------

void T3Ch4PWM_SetPeriodTicks(uint32_t Cycles)
{
	uint32_t MaxCycles = T3Ch4PWM_GetMaxCycles();

	if(Cycles == 0)
	{
		TIM3->CCR4 = 0;
		return;
	}

	// Насыщение: не выходим за 95% от базового периода и за предел ARR
	if(Cycles > MaxCycles)
		Cycles = MaxCycles;

	T3Ch4PWM_ApplyPeriod(Cycles);
}
//------------------------------------------------

Boolean T3Ch4PWM_Start()
{
	TIM3->DIER &= ~TIM_DIER_CC4IE;
	TIM3->SR = ~(TIM_SR_CC4IF | TIM_SR_UIF);
	TIM3->EGR |= TIM_EGR_UG;
	if(!T3Ch4PWM_WaitUpdateFlag())
	{
		TIM3->SR = ~(TIM_SR_CC4IF | TIM_SR_UIF);
		return FALSE;
	}
	TIM3->SR = ~(TIM_SR_CC4IF | TIM_SR_UIF);

	TIM3->DIER |= TIM_DIER_CC4IE;
	TIM_Start(TIM3);
	return TRUE;
}
//------------------------------------------------

Boolean T3Ch4PWM_Stop()
{
	// CC4IE остаётся выключенным до следующего Start()
	TIM3->DIER &= ~TIM_DIER_CC4IE;
	TIM3->SR = ~(TIM_SR_CC4IF | TIM_SR_UIF);

	T3Ch4PWM_SetPeriodTicks(0);
	TIM_Stop(TIM3);

	TIM3->EGR |= TIM_EGR_UG;
	if(!T3Ch4PWM_WaitUpdateFlag())
	{
		TIM3->SR = ~(TIM_SR_CC4IF | TIM_SR_UIF);
		return FALSE;
	}
	TIM3->SR = ~(TIM_SR_CC4IF | TIM_SR_UIF);
	return TRUE;
}
//------------------------------------------------
