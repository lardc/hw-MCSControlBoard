// Header
//
#include "DS18B20.h"

// Definitions
//
#define DQ_WRITE_INVERSION			TRUE
#define DQ_READ_INVERSION			FALSE
#define DQ_STRONG_PULLUP_INVERSION	TRUE
#define DS18B20_USE_PARASITE_POWER	TRUE
//
#define DS18B20_REGISTERS			9
#define DS18B20_WRITE_TIMEOUT		5

// Include
//
#include "BoardConfig.h"
#include "DataTable.h"

// Function prototypes
//
void DS18B20_SetDQ(Boolean State);
Boolean DS18B20_ReadDQ();
Boolean DS18B20_ReadBit();
void DS18B20_WriteBit(Boolean Bit);
Int16U DS18B20_ReadByte();
void DS18B20_WriteByte(Int16U Data);
void DS18B20_StrongPullUpDQ(Boolean State);

// Functions
//
Boolean DS18B20_Reset()
{
	Boolean InitState;

	DS18B20_SetDQ(FALSE);
	DELAY_US(700);
	DS18B20_SetDQ(TRUE);
	DELAY_US(90);

	InitState = !DS18B20_ReadDQ();

	DELAY_US(250);

	return InitState;
}
//-------------------

Boolean DS18B20_ReadROM(pInt16U Data)
{
	Int16U i;

	if(DS18B20_Reset())
	{
		DS18B20_WriteByte(DS18B20_READ_ROM);

		for(i = 0; i < 8; i++)
			*(Data + i) = DS18B20_ReadByte();

		return 1;
	}

	return 0;
}
//-------------------

Boolean DS18B20_WriteReg(pInt16U Data)
{
	Int16U TimeoutCounter = 0;

	if(DS18B20_Reset())
	{
		DS18B20_WriteByte(DS18B20_SKIP_ROM);
		DS18B20_WriteByte(DS18B20_WRITE_SCRATCHPAD);
		DS18B20_WriteByte((*Data >> 8) & 0xFF);
		DS18B20_WriteByte(*Data & 0xFF);
		DS18B20_WriteByte(CONFIG_RES_12BIT);

		if(DS18B20_Reset())
		{
			DS18B20_WriteByte(DS18B20_SKIP_ROM);
			DS18B20_WriteByte(DS18B20_COPY_SCRATCHPAD);

#ifdef DS18B20_USE_PARASITE_POWER
			DS18B20_StrongPullUpDQ(TRUE);
			DELAY_US(10000);
			DS18B20_StrongPullUpDQ(FALSE);
#endif

			while(!DS18B20_Reset())
			{
				TimeoutCounter++;

				if(TimeoutCounter >= DS18B20_WRITE_TIMEOUT)
					return 0;
			}

			return 1;
		}
		else
			return 0;
	}

	return 0;
}
//-------------------

Boolean DS18B20_ReadReg(pInt16U Data)
{
	Int16U i;
	Int16U ReadBytes[DS18B20_REGISTERS];

	if(DS18B20_Reset())
	{
		DS18B20_WriteByte(DS18B20_SKIP_ROM);
		DS18B20_WriteByte(DS18B20_READ_SCRATCHPAD);

		for(i = 0; i < DS18B20_REGISTERS; i++)
			ReadBytes[i] = DS18B20_ReadByte();

		*(Data) = ReadBytes[REG_USER_BYTE_1] << 8 | ReadBytes[REG_USER_BYTE_2];

		return 1;
	}

	return 0;
}
//-------------------

Int16U DS18B20_ReadByte()
{
	Int16U Data = 0;
	Int16U i;

	for(i = 0; i < 8; i++)
		Data |= DS18B20_ReadBit() << i;

	return Data;
}
//-------------------

void DS18B20_WriteByte(Int16U Data)
{
	Int16U i;

	for(i = 0; i < 8; i++)
		DS18B20_WriteBit(Data >> i & 0x1);
}
//-------------------

void DS18B20_WriteBit(Boolean Bit)
{
	DS18B20_SetDQ(FALSE);
	DELAY_US(Bit ? 5 : 65);
	DS18B20_SetDQ(TRUE);
	DELAY_US(Bit ? 65 : 5);
}
//-------------------

Boolean DS18B20_ReadBit()
{
	Boolean Bit;

	DS18B20_SetDQ(FALSE);
	DELAY_US(5);
	DS18B20_SetDQ(TRUE);
	DELAY_US(10);

	Bit = DS18B20_ReadDQ();

	DELAY_US(55);

	return Bit;
}
//-------------------

void DS18B20_SetDQ(Boolean State)
{
	(DQ_WRITE_INVERSION) ? ZwGPIO_WritePin(PIN_ADAPTER_ID_CTRL, !State) : ZwGPIO_WritePin(PIN_ADAPTER_ID_CTRL, State);
}
//-------------------

Boolean DS18B20_ReadDQ()
{
	return (DQ_READ_INVERSION) ? !ZwGPIO_ReadPin(PIN_ADAPTER_ID_DATA) : ZwGPIO_ReadPin(PIN_ADAPTER_ID_DATA);
}
//-------------------

void DS18B20_StrongPullUpDQ(Boolean State)
{
	(DQ_STRONG_PULLUP_INVERSION) ? ZwGPIO_WritePin(PIN_ADAPTER_ID_PWR, !State) : ZwGPIO_WritePin(PIN_ADAPTER_ID_PWR, State);
}
//-------------------
