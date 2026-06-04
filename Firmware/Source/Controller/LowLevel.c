#include "LowLevel.h"

#include "DS18B20.h"
#include "Global.h"
#include "DataTable.h"
#include "Delay.h"
#include "SysConfig.h"

static float LL_MeasureWrapper(ADC_TypeDef* ADCx, uint32_t ChannelNumber)
{
	float result = 0;
	Int16U samples = AVG_SAMPLES_DEF;
	Int16U i;

	for(i = 0; i < samples; i++)
		result += ADC_Measure(ADCx, ChannelNumber);

	return result / samples * ADC_REF_VOLTAGE / ADC_RESOLUTION;
}
//-----------------------------

void LL_InitGPIO()
{
	GPIO_InitPushPullOutput(GPIO_STPM_EN);
	GPIO_InitPushPullOutput(GPIO_STPM_DIR);
	GPIO_InitPushPullOutput(GPIO_STPM_STEP);
	GPIO_InitPushPullOutput(GPIO_OUT_CONTROL);
	GPIO_InitPushPullOutput(GPIO_FAN);
	GPIO_InitPushPullOutput(GPIO_LED);
	GPIO_InitPushPullOutput(GPIO_OUT_POWER);
	GPIO_InitPushPullOutput(GPIO_SPIMUX_A);
	GPIO_InitPushPullOutput(GPIO_SPIMUX_B);
	GPIO_InitPushPullOutput(GPIO_SPIMUX_C);

	GPIO_SetState(GPIO_STPM_EN, false);
	GPIO_SetState(GPIO_STPM_DIR, false);
	GPIO_SetState(GPIO_STPM_STEP, false);
	GPIO_SetState(GPIO_OUT_CONTROL, false);
	GPIO_SetState(GPIO_FAN, false);
	GPIO_SetState(GPIO_LED, false);
	GPIO_SetState(GPIO_OUT_POWER, false);
	GPIO_SetState(GPIO_SPIMUX_A, true);
	GPIO_SetState(GPIO_SPIMUX_B, true);
	GPIO_SetState(GPIO_SPIMUX_C, true);

	GPIO_InitInput(GPIO_SEN_BUS, NoPull);
	GPIO_InitInput(GPIO_SEN_ADAPTER, NoPull);
	GPIO_InitInput(GPIO_HOME, NoPull);
	GPIO_InitInput(GPIO_SAFETY_IN, NoPull);

	DS18B20_Init();
}
//-----------------------------

Boolean LL_FilterSafetyCircuit(Boolean NewState)
{
	static Int16U SafetyCircuitCounter = 0;

	if(!NewState)
		SafetyCircuitCounter = 0;
	else if(SafetyCircuitCounter < SC_FILTER_T)
		SafetyCircuitCounter++;

	return (SafetyCircuitCounter >= SC_FILTER_T);
}
//-----------------------------

void LL_CSMux(Int16U SPIDevice)
{
	GPIO_SetState(GPIO_SPIMUX_A, SPIDevice & BIT0);
	GPIO_SetState(GPIO_SPIMUX_B, SPIDevice & BIT1);
	GPIO_SetState(GPIO_SPIMUX_C, SPIDevice & BIT2);
	DELAY_US(1);
}
//-----------------------------

Boolean LL_IsSafetySensorOk()
{
	return !GPIO_GetState(GPIO_SAFETY_IN);
}
//-----------------------------

Boolean LL_HomeSensorActuate()
{
	return GPIO_GetState(GPIO_HOME);
}
//-----------------------------

Boolean LL_IsBusToolingSensorOk()
{
	return !GPIO_GetState(GPIO_SEN_BUS);
}
//-----------------------------

Boolean LL_IsAdapterToolingSensorOk()
{
	return !GPIO_GetState(GPIO_SEN_ADAPTER);
}
//-----------------------------

void LL_SwitchPowerConnection(Boolean State)
{
	GPIO_SetState(GPIO_OUT_POWER, State);
}
//-----------------------------

Boolean LL_IsPowerConnected()
{
	return GPIO_GetState(GPIO_OUT_POWER);
}
//-----------------------------

void LL_SwitchControlConnection(Boolean State)
{
	GPIO_SetState(GPIO_OUT_CONTROL, State);
}
//-----------------------------

Boolean LL_IsControlConnected()
{
	return GPIO_GetState(GPIO_OUT_CONTROL);
}
//-----------------------------

void LL_SwitchStep(Boolean State)
{
	GPIO_SetState(GPIO_STPM_STEP, State);
}
//-----------------------------

void LL_ToggleStep()
{
	GPIO_Toggle(GPIO_STPM_STEP);
}
//-----------------------------

void LL_SwitchUpDir(Boolean State)
{
	GPIO_SetState(GPIO_STPM_DIR, !State);
}
//-----------------------------

Boolean LL_IsDirUp()
{
	return !GPIO_GetState(GPIO_STPM_DIR);
}
//-----------------------------

void LL_SwitchEnable(Boolean State)
{
	GPIO_SetState(GPIO_STPM_EN, State);
}
//-----------------------------

void LL_SwitchFan(Boolean State)
{
	GPIO_SetState(GPIO_FAN, State);
}
//-----------------------------

void LL_ToggleBoardLED()
{
	GPIO_Toggle(GPIO_LED);
}
//-----------------------------

float LL_MeasurePressure()
{
	float offset = DataTable[REG_PRESSURE_OFFSET];
	float k = DataTable[REG_PRESSURE_K];
	float pressure = LL_MeasureWrapper(ADC2, ADC_PRESSURE_CHANNEL) * k
			- offset * ADC_REF_VOLTAGE / ADC_RESOLUTION * k;

	return (pressure > 0) ? pressure : 0;
}
//-----------------------------
