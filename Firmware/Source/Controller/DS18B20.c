#include "DS18B20.h"
#include "OneWire.h"
#include "Board.h"
#include "DataTable.h"
#include "DeviceObjectDictionary.h"
#include "Delay.h"

// Definitions
//
#define DS18B20_REGISTERS			9
#define DS18B20_WRITE_TIMEOUT		5

// Functions
//
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
	if(OneWire_Reset())
	{
		OneWire_Skip();
		OneWire_Write(DS18B20_WRITE_SCRATCHPAD, 0);
		OneWire_Write((*Data >> 8) & 0xFF, 0);
		OneWire_Write(*Data & 0xFF, 0);
		OneWire_Write(CONFIG_RES_12BIT, 0);

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
	Int8U ReadBytes[DS18B20_REGISTERS];

	if(OneWire_Reset())
	{
		OneWire_Skip();
		OneWire_Write(DS18B20_READ_SCRATCHPAD, 0);
		OneWire_ReadBytes(ReadBytes, DS18B20_REGISTERS);

		*Data = ReadBytes[REG_USER_BYTE_1] << 8 | ReadBytes[REG_USER_BYTE_2];
		return true;
	}

	return false;
}
//-------------------

static void DS18B20_PublishIdentifier(pAdapterIdentifier Id)
{
	DataTable[REG_ADAPTER_ID] = Id->Code;
	DataTable[REG_ADAPTER_CLAMP_HEIGHT] = Id->ClampHeightMm;
	DataTable[REG_ADAPTER_MAX_CURRENT] = Id->MaxCurrent;
	DataTable[REG_ADAPTER_MAX_VOLTAGE] = Id->MaxVoltage;
	DataTable[REG_ADAPTER_SERIAL] = Id->Serial;
}
//-------------------

static void DS18B20_LoadIdentifierStub(pAdapterIdentifier Id)
{
	Id->ClampHeightMm = DataTable[REG_ADAPTER_CLAMP_HEIGHT];
	Id->MaxCurrent = DataTable[REG_ADAPTER_MAX_CURRENT];
	Id->MaxVoltage = DataTable[REG_ADAPTER_MAX_VOLTAGE];
	Id->Serial = DataTable[REG_ADAPTER_SERIAL];
}
//-------------------

Boolean DS18B20_ReadIdentifier(pAdapterIdentifier Id)
{
	if(!DS18B20_ReadReg(&Id->Code))
		return false;

	// v1: layout EEPROM уточняется; поля кроме кода — зеркало DataTable
	DS18B20_LoadIdentifierStub(Id);
	DS18B20_PublishIdentifier(Id);

	return true;
}
//-------------------

Boolean DS18B20_WriteIdentifier(pAdapterIdentifier Id)
{
	// v1: запись кода как в CS; полный блок EEPROM — TODO
	if(!DS18B20_WriteReg(&Id->Code))
		return false;

	DS18B20_PublishIdentifier(Id);
	return true;
}
//-------------------
