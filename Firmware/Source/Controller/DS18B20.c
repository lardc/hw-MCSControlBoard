#include "DS18B20.h"
#include "OneWire.h"
#include "Board.h"
#include "Delay.h"

#define DS18B20_WRITE_TIMEOUT		5
#define DS18B20_CONVERT_DELAY_US	750000	// 12-bit, parasite power

static Boolean DS18B20_ReadScratchpad(Int8U *Scratchpad);
static Boolean DS18B20_StartConvert();

void DS18B20_Init()
{
	bool UsePowerPin = true;
	bool SinglePin = false;
	bool InvertWrite = true;
	bool InvertPower = true;

	OneWire_Init(GPIO_DQ_CTRL, GPIO_DQ_IN, GPIO_DQ_PWR, UsePowerPin, SinglePin, InvertWrite, InvertPower);
}
//-------------------

Boolean DS18B20_WriteReg(pInt16U Data)
{
	Int8U Scratchpad[DS18B20_SCRATCHPAD_SIZE];

	if(OneWire_Reset())
	{
		OneWire_Skip();
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

		if(OneWire_Reset())
		{
			OneWire_Skip();
			OneWire_Write(DS18B20_COPY_SCRATCHPAD, 1);
			DELAY_US(10000);
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
	}

	return false;
}
//-------------------

Boolean DS18B20_ReadReg(pInt16U Data)
{
	Int8U Scratchpad[DS18B20_SCRATCHPAD_SIZE];

	if(!DS18B20_ReadScratchpad(Scratchpad))
		return false;

	*Data = (Int16U)Scratchpad[REG_USER_BYTE_1] << 8 | Scratchpad[REG_USER_BYTE_2];
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

	Raw = (Int16S)((Int16U)Scratchpad[REG_TEMPERATURE_LSB] | ((Int16U)Scratchpad[REG_TEMPERATURE_MSB] << 8));
	*Data = (Int16S)(((Int32S)Raw * 10) / 16);

	return true;
}
//-------------------

static Boolean DS18B20_StartConvert()
{
	if(!OneWire_Reset())
		return false;

	OneWire_Skip();
	OneWire_Write(DS18B20_CONVERT_T, 1);
	DELAY_US(DS18B20_CONVERT_DELAY_US);
	OneWire_Depower();

	return true;
}
//-------------------

static Boolean DS18B20_ReadScratchpad(Int8U *Scratchpad)
{
	if(!OneWire_Reset())
		return false;

	OneWire_Skip();
	OneWire_Write(DS18B20_READ_SCRATCHPAD, 0);
	OneWire_ReadBytes(Scratchpad, DS18B20_SCRATCHPAD_SIZE);

	return OneWire_CheckCrc8(Scratchpad, REG_CRC, Scratchpad[REG_CRC]);
}
//-------------------
