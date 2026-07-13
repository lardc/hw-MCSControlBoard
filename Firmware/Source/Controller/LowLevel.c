#include "LowLevel.h"

#include "Delay.h"
#include "Global.h"
#include "SysConfig.h"
#include "ZwSPI.h"

static Int8U SpiOutShadow = 0;
//-----------------------------

static void LL_SPI_WriteRaw(Int8U Data)
{
	SPI_WriteByte8b(SPI3, Data);
}
//-----------------------------

static void LL_SPI_LatchOut()
{
	DELAY_US(TIME_SPI_DELAY_US);
	GPIO_SetState(GPIO_SPI_SS, true);
	DELAY_US(TIME_SPI_DELAY_US);
	GPIO_SetState(GPIO_SPI_SS, false);
	DELAY_US(TIME_SPI_DELAY_US);
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

void LL_SPI_SetOutBit(Int8U Bit, Boolean State)
{
	if(State)
		SpiOutShadow |= (Int8U)(1u << Bit);
	else
		SpiOutShadow &= (Int8U)~(1u << Bit);
}
//-----------------------------

void LL_SPI_FlushOut()
{
	LL_SPI_WriteRaw(SpiOutShadow);
	LL_SPI_LatchOut();
}
//-----------------------------

Int8U LL_SPI_ReadInRaw()
{
	Int8U Data;

	GPIO_SetState(GPIO_SPI_LD, false);
	DELAY_US(TIME_SPI_DELAY_US);
	GPIO_SetState(GPIO_SPI_LD, true);
	DELAY_US(TIME_SPI_DELAY_US);

	GPIO_SetState(GPIO_SPI_OE, false);
	DELAY_US(TIME_SPI_DELAY_US);
	Data = (Int8U)SPI_ReadByte8b(SPI3);
	GPIO_SetState(GPIO_SPI_OE, true);

	return Data;
}
//-----------------------------

Boolean LL_SPI_GetInBit(Int8U Bit)
{
	return (LL_SPI_ReadInRaw() & (1u << Bit)) != 0;
}
//-----------------------------

Boolean LL_SPI_IsCoil24VOk()
{
	return (LL_SPI_ReadInRaw() & SPI_IN_MASK_COIL_24V) == SPI_IN_MASK_COIL_24V;
}
//-----------------------------

Boolean LL_IsTableSensorOk()
{
	return GPIO_GetState(GPIO_SEN_S2);
}
//-----------------------------

Boolean LL_IsSafetyS3Ok()
{
	return GPIO_GetState(GPIO_SEN_S3);
}
//-----------------------------

Boolean LL_IsSafetyS5Ok()
{
	return GPIO_GetState(GPIO_SEN_S5);
}
//-----------------------------

Boolean LL_HomeSensorActuate()
{
	return GPIO_GetState(GPIO_HOMING);
}
//-----------------------------

void LL_SetTestLine(Boolean State)
{
	GPIO_SetState(GPIO_TEST, State);
}
//-----------------------------

void LL_RS485_SetTxMode(Boolean State)
{
	GPIO_SetState(GPIO_RS485_CTRL, State);
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

void LL_ToggleBoardLED()
{
	GPIO_Toggle(GPIO_LED);
}
//-----------------------------
