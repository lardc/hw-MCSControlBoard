#include "Measurement.h"

#include "DataTable.h"
#include "DeviceObjectDictionary.h"
#include "Global.h"
#include "InitConfig.h"
#include "SysConfig.h"

float MEAS_GetRawVoltage()
{
	Int32U Sum = 0;
	Int16U i;

	for(i = 0; i < ADC_PRESSURE_BUF_SIZE; ++i)
		Sum += INITCFG_PressureAdcBuffer[i];

	return (float)Sum / ADC_PRESSURE_BUF_SIZE * ADC_REF_VOLTAGE / ADC_RESOLUTION;
}
//-----------------------------

float MEAS_GetPressureBar()
{
	float voltageMv = MEAS_GetRawVoltage() * ADC_PRESSURE_INPUT_GAIN;
	float Pressure = voltageMv * DataTable[REG_PRESSURE_K] + DataTable[REG_PRESSURE_OFFSET];

	return (Pressure > 0.0f) ? Pressure : 0.0f;
}
//-----------------------------

Int32U MEAS_GetPressureMilliBar()
{
	return (Int32U)(MEAS_GetPressureBar() * 1000.0f);
}
//-----------------------------

Boolean MEAS_IsPressureOk()
{
	return MEAS_GetPressureMilliBar() >= DataTable[REG_PRESSURE_OK];
}
//-----------------------------
