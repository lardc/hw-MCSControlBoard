// Header
#include "DS18B20.h"

// Includes
#include "OneWire.h"
#include "Board.h"
#include "Delay.h"

#define DS18B20_WRITE_TIMEOUT		5
#define DS18B20_CONVERT_DELAY_MS	750

// Variables
static Int8U SerialNumber[DS18B20_ONE_WIRE_MAC_SIZE];
static Boolean SkipRom = true;

// Forward functions
static Boolean DS18B20_StartTransmission();
static Boolean DS18B20_ReadScratchpad(Int8U *Scratchpad);
static Boolean DS18B20_StartConvert();

// Functions
void DS18B20_Init()
{
	Int8U addr[DS18B20_ONE_WIRE_MAC_SIZE];

#if ONEWIRE_THREE_PIN_BUS
	bool UsePowerPin = true;
	bool SinglePin = false;
	bool InvertWrite = true;
	bool InvertPower = true;

	OneWire_Init(GPIO_DQ_CTRL, GPIO_DQ_IN, GPIO_DQ_PWR, UsePowerPin, SinglePin, InvertWrite, InvertPower);
#else
	OneWire_Init(GPIO_DQ, GPIO_DQ, GPIO_DQ, false, true, false, false);
#endif

	OneWire_ResetSearch();
	OneWire_TargetSearch(DS18B20_FAMILY_CODE);

	if(OneWire_Search(addr, true) && OneWire_CheckCrc8(addr, 7, addr[7]))
		DS18B20_Begin(addr);
}
//-------------------

void DS18B20_Begin(Int8U serialNumber[DS18B20_ONE_WIRE_MAC_SIZE])
{
	for (Int8U i = 0; i < DS18B20_ONE_WIRE_MAC_SIZE; i++)
		SerialNumber[i] = serialNumber[i];

	SkipRom = false;
}
//-------------------

Boolean DS18B20_WriteReg(pInt16U Data)
{
	Int8U Scratchpad[DS18B20_SCRATCHPAD_SIZE];

	if(!DS18B20_StartTransmission())
		return false;

	OneWire_Write(DS18B20_WRITE_SCRATCHPAD, 0);
	OneWire_Write((*Data >> 8) & 0xFF, 0);
	OneWire_Write(*Data & 0xFF, 0);
	OneWire_Write(CONFIG_RES_12BIT, 0);

	if(!DS18B20_ReadScratchpad(Scratchpad))
		return false;

	if(Scratchpad[REG_USER_BYTE_1] != ((*Data >> 8) & 0xFF)
			|| Scratchpad[REG_USER_BYTE_2] != (*Data & 0xFF)
			|| Scratchpad[REG_CONFIGURATION] != CONFIG_RES_12BIT)
		return false;

	if(!DS18B20_StartTransmission())
		return false;

	OneWire_Write(DS18B20_COPY_SCRATCHPAD, 1);
	DELAY_MS(10);
	OneWire_Depower();

	{
		Int16U TimeoutCounter = 0;

		while(!OneWire_Reset())
		{
			DELAY_US(1000);
			if(++TimeoutCounter >= DS18B20_WRITE_TIMEOUT)
				return false;
		}
	}

	return true;
}
//-------------------

Boolean DS18B20_ReadReg(pInt16U Data)
{
	Int8U Scratchpad[DS18B20_SCRATCHPAD_SIZE];

	if(!DS18B20_ReadScratchpad(Scratchpad))
		return false;

	*Data = ((Int16U)Scratchpad[REG_USER_BYTE_1] << 8) | Scratchpad[REG_USER_BYTE_2];
	return true;
}
//-------------------

Boolean DS18B20_ReadTemperatureC10(pInt16S Data)
{
	Int8U Scratchpad[DS18B20_SCRATCHPAD_SIZE];
	Int16S Raw;

	if(!DS18B20_StartConvert())
		return false;

	if(!DS18B20_ReadScratchpad(Scratchpad))
		return false;

	Raw = (Int16S)((Int16U)Scratchpad[REG_TEMPERATURE_MSB] << 8 | Scratchpad[REG_TEMPERATURE_LSB]);
	*Data = (Raw * 10) / 16;

	return true;
}
//-------------------

static Boolean DS18B20_StartTransmission()
{
	if(!OneWire_Reset())
		return false;

	if(SkipRom)
		OneWire_Skip();
	else
		OneWire_Select(SerialNumber);

	return true;
}
//-------------------

static Boolean DS18B20_StartConvert()
{
	if(!DS18B20_StartTransmission())
		return false;

#if ONEWIRE_THREE_PIN_BUS
	OneWire_Write(DS18B20_CONVERT_T, 1);
	DELAY_MS(DS18B20_CONVERT_DELAY_MS);
	OneWire_Depower();
#else
	OneWire_Write(DS18B20_CONVERT_T, 0);
	OneWire_StrongPullupHold(true);
	DELAY_MS(DS18B20_CONVERT_DELAY_MS);
	OneWire_StrongPullupHold(false);
#endif

	return true;
}
//-------------------

static Boolean DS18B20_ReadScratchpad(Int8U *Scratchpad)
{
	if(!DS18B20_StartTransmission())
		return false;

	OneWire_Write(DS18B20_READ_SCRATCHPAD, 0);
	OneWire_ReadBytes(Scratchpad, DS18B20_SCRATCHPAD_SIZE);

	return OneWire_CheckCrc8(Scratchpad, REG_CRC, Scratchpad[REG_CRC]);
}
//-------------------
