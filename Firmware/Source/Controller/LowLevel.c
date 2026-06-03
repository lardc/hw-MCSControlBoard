// Header
#include "LowLevel.h"
// Include
#include "Board.h"
#include "Delay.h"
#include "Global.h"
#include "DataTable.h"
#include "Constraints.h"

// Forward functions
float LL_MeasureWrapper(ADC_TypeDef* ADCx, uint32_t ChannelNumber);

// Functions
void LL_ToggleBoardLED()
{
	GPIO_Toggle(GPIO_LED);
}
//-----------------------------

void LL_IndicateBlockCSM(bool State)
{
	GPIO_SetState(GPIO_IND_CSM, State);
}
//-----------------------------

void LL_IndicateBlockAdapter(bool State)
{
	GPIO_SetState(GPIO_IND_ADPTR, State);
}
//-----------------------------

void LL_HoldTopAdapter(bool State)
{
	GPIO_SetState(GPIO_PNEUM_TOP, State);
}
//-----------------------------

void LL_HoldBotAdapter(bool State)
{
	GPIO_SetState(GPIO_PNEUM_BOT, State);
}
//-----------------------------

void LL_ClampDUT(bool State)
{
	GPIO_SetState(GPIO_PNEUM_DUT, State);
}
//-----------------------------

void LL_SetSafetyOutput(bool State)
{
	GPIO_SetState(GPIO_SF_OUT, State);
}
//-----------------------------

bool LL_GetStatePresenceSensorDUT1()
{
	return GPIO_GetState(GPIO_DUT_1);
}
//-----------------------------

bool LL_GetStatePresenceSensorDUT2()
{
	return GPIO_GetState(GPIO_DUT_2);
}
//-----------------------------

bool LL_GetStatePresenceSensorDUT3()
{
	return GPIO_GetState(GPIO_DUT_3);
}
//-----------------------------

bool LL_GetStatePresenceSensorDUT4()
{
	return GPIO_GetState(GPIO_DUT_4);
}
//-----------------------------

bool LL_GetStateLimitSwitchTopAdapter()
{
	return GPIO_GetState(GPIO_ADPTR_TOP);
}
//-----------------------------

bool LL_GetStateLimitSwitchBotAdapter()
{
	return GPIO_GetState(GPIO_ADPTR_BOT);
}
//-----------------------------

float LL_MeasureIDTop()
{
	return LL_MeasureWrapper(ADC1, ADC_ID_TOP_CHANNEL);
}
//-----------------------------

float LL_MeasureIDBot()
{
	return LL_MeasureWrapper(ADC1, ADC_ID_BOT_CHANNEL);
}
//-----------------------------

float LL_MeasurePressure()
{
	float Offset = DataTable[REG_PRESSURE_OFFSET];
	float K = DataTable[REG_PRESSURE_K];

	// Такая формула для оффсета сделана для совместимости с уже записанными в DT значениями и по сути
	// не является корректной
	float Pressure = LL_MeasureWrapper(ADC2, ADC_PRESSURE_CHANNEL) * K - Offset * ADC_REF_VOLTAGE / ADC_RESOLUTION * K;
	Pressure = (Pressure > 0) ? Pressure : 0;

	// Тонкая подстройка измерения напряжения
	float P2 = DataTable[REG_PRESSURE_P2];
	float P1 = DataTable[REG_PRESSURE_P1];
	float P0 = DataTable[REG_PRESSURE_P0];
	Pressure = Pressure * Pressure * P2 + Pressure * P1 + P0;

	return (Pressure > 0) ? Pressure : 0;
}
//-----------------------------

float LL_MeasureWrapper(ADC_TypeDef* ADCx, uint32_t ChannelNumber)
{
	float Result = 0;
	float Samples = DataTable[REG_SAMPLING_AVG] ? DataTable[REG_SAMPLING_AVG] : AVG_SAMPLES_DEF;

	for(int i = 0; i < Samples; i++)
		Result += ADC_Measure(ADCx, ChannelNumber);

	return Result / Samples * ADC_REF_VOLTAGE / ADC_RESOLUTION;
}
//-----------------------------
