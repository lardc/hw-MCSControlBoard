// Header
#include "Timer3_Ch4PWM.h"

// Includes
#include "ZwTIM.h"
#include "ZwRCC.h"

// Variables
static uint32_t PWMBase = 0;

// Functions
static void T3Ch4PWM_ApplyPeriod(uint32_t PeriodTicks)
{
	if(PeriodTicks < 2)
		PeriodTicks = 2;

	TIM3->ARR = PeriodTicks;
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
	TIM3->ARR = PWMBase;
	TIM3->CR1 |= TIM_CR1_ARPE;

	// Канал 4 - STEP шагового двигателя
	// Режим ШИМ - PWM mode 1, c функцией Preload
	TIM3->CCMR2 = (TIM3->CCMR2 & ~TIM_CCMR2_OC4M) |
			(TIM_CCMR2_OC4M_2 | TIM_CCMR2_OC4M_1 | TIM_CCMR2_OC4PE);
	TIM3->CCER |= TIM_CCER_CC4E;
	TIM3->CCR4 = 0;

	// Сброс выхода
	T3Ch4PWM_SetFrequency(0);

	// Инициализация обновления регистров
	TIM3->SR &= ~TIM_SR_UIF;
	TIM3->EGR |= TIM_EGR_UG;
	while(!(TIM3->SR & TIM_SR_UIF));
	TIM3->SR &= ~TIM_SR_UIF;

	// Разрешение прерывания
	TIM_Interupt(TIM3, 0, true);
}
//------------------------------------------------

uint32_t T3Ch4PWM_GetMaxCycles()
{
	return (uint32_t)(PWMBase * T3CH4PWM_MAX_OUTPUT * 2.0f);
}
//------------------------------------------------

void T3Ch4PWM_SetFrequency(uint32_t Cycles)
{
	uint32_t MaxCycles = T3Ch4PWM_GetMaxCycles();

	if(Cycles == 0)
	{
		TIM3->CCR4 = 0;
		return;
	}

	// Насыщение: не выходим за 95% от удвоенной базы ARR
	if(Cycles > MaxCycles)
		Cycles = MaxCycles;

	T3Ch4PWM_ApplyPeriod(Cycles);
}
//------------------------------------------------

void T3Ch4PWM_Start()
{
	TIM_Start(TIM3);
}
//------------------------------------------------

void T3Ch4PWM_Stop()
{
	T3Ch4PWM_SetFrequency(0);
	TIM_Stop(TIM3);

	// Запрет прерывания и очистка флага
	TIM3->DIER &= ~TIM_DIER_UIE;
	TIM3->SR &= ~TIM_SR_UIF;

	// Форсирование обновления регистров таймера
	TIM3->EGR |= TIM_EGR_UG;
	while(!(TIM3->SR & TIM_SR_UIF));
	TIM3->SR &= ~TIM_SR_UIF;

	// Разрешение прерывания
	TIM3->DIER |= TIM_DIER_UIE;
}
//------------------------------------------------
