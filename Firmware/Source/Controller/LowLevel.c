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
	GPIO_InitPushPullOutput(GPIO_LED);
	GPIO_InitPushPullOutput(GPIO_RS485_CTRL);
	GPIO_InitPushPullOutput(GPIO_STPM_DIR);
	GPIO_InitPushPullOutput(GPIO_STPM_STEP);
	GPIO_InitPushPullOutput(GPIO_STPM_EN);
	GPIO_InitPushPullOutput(GPIO_SPI_SS);
	GPIO_InitPushPullOutput(GPIO_TEST);

	GPIO_InitOpenDrainOutput(GPIO_SPI_LD, NoPull);
	GPIO_InitOpenDrainOutput(GPIO_SPI_OE, NoPull);

	GPIO_InitInput(GPIO_SEN_S1, NoPull);
	GPIO_InitInput(GPIO_SEN_S3, NoPull);
	GPIO_InitInput(GPIO_SEN_S2, NoPull);
	GPIO_InitInput(GPIO_SEN_S4, NoPull);
	GPIO_InitInput(GPIO_SEN_S5, NoPull);
	GPIO_InitInput(GPIO_HOMING, NoPull);

	GPIO_SetState(GPIO_LED, false);
	GPIO_SetState(GPIO_RS485_CTRL, false);
	GPIO_SetState(GPIO_STPM_DIR, false);
	GPIO_SetState(GPIO_STPM_STEP, false);
	GPIO_SetState(GPIO_STPM_EN, false);
	GPIO_SetState(GPIO_SPI_SS, false);
	GPIO_SetState(GPIO_TEST, false);
	GPIO_SetState(GPIO_SPI_LD, false);
	GPIO_SetState(GPIO_SPI_OE, true);

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
	(void)SPIDevice;
}
//-----------------------------

Boolean LL_IsSafetySensorOk()
{
	return GPIO_GetState(GPIO_SEN_S3);
}
//-----------------------------

Boolean LL_HomeSensorActuate()
{
	return GPIO_GetState(GPIO_HOMING);
}
//-----------------------------

Boolean LL_IsBusToolingSensorOk()
{
	return false;
}
//-----------------------------

Boolean LL_IsAdapterToolingSensorOk()
{
	return false;
}
//-----------------------------

void LL_SwitchPowerConnection(Boolean State)
{
	(void)State;
}
//-----------------------------

Boolean LL_IsPowerConnected()
{
	return false;
}
//-----------------------------

void LL_SwitchControlConnection(Boolean State)
{
	(void)State;
}
//-----------------------------

Boolean LL_IsControlConnected()
{
	return false;
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
	(void)State;
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
