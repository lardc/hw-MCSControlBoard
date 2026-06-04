#include "DS18B20.h"
#include "Board.h"
#include "DataTable.h"
#include "ZbBoard.h"

// Definitions
//
#define DQ_WRITE_INVERSION			true
#define DQ_READ_INVERSION			false
#define DQ_STRONG_PULLUP_INVERSION	true
#define DS18B20_USE_PARASITE_POWER	true
//
#define DS18B20_REGISTERS			9
#define DS18B20_WRITE_TIMEOUT		5

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
void DS18B20_Init()
{
	GPIO_InitPushPullOutput(GPIO_ADAPTER_ID_PWR);
	GPIO_InitPushPullOutput(GPIO_ADAPTER_ID_CTRL);
	GPIO_InitInput(GPIO_ADAPTER_ID_DATA, NoPull);

	GPIO_SetState(GPIO_ADAPTER_ID_CTRL, false);
	GPIO_SetState(GPIO_ADAPTER_ID_PWR, true);
}
//-------------------

Boolean DS18B20_Reset()
{
	Boolean InitState;

	DS18B20_SetDQ(false);
	DELAY_US(700);
	DS18B20_SetDQ(true);
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

		return true;
	}

	return false;
}
//-------------------

Boolean DS18B20_WriteReg(pInt16U Data)
{
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
			DS18B20_StrongPullUpDQ(true);
			DELAY_US(10000);
			DS18B20_StrongPullUpDQ(false);
#endif
			{
				Int16U TimeoutCounter = 0;

				while(!DS18B20_Reset())
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
	Int16U i;
	Int16U ReadBytes[DS18B20_REGISTERS];

	if(DS18B20_Reset())
	{
		DS18B20_WriteByte(DS18B20_SKIP_ROM);
		DS18B20_WriteByte(DS18B20_READ_SCRATCHPAD);

		for(i = 0; i < DS18B20_REGISTERS; i++)
			ReadBytes[i] = DS18B20_ReadByte();

		*Data = ReadBytes[REG_USER_BYTE_1] << 8 | ReadBytes[REG_USER_BYTE_2];
		return true;
	}

	return false;
}
//-------------------

Int16U DS18B20_ReadByte()
{
	Int16U i, Data = 0;

	for(i = 0; i < 8; i++)
		Data |= DS18B20_ReadBit() << i;

	return Data;
}
//-------------------

void DS18B20_WriteByte(Int16U Data)
{
	Int16U i;

	for(i = 0; i < 8; i++)
		DS18B20_WriteBit((Data >> i) & 0x1);
}
//-------------------

void DS18B20_WriteBit(Boolean Bit)
{
	DS18B20_SetDQ(false);
	DELAY_US(Bit ? 5 : 65);
	DS18B20_SetDQ(true);
	DELAY_US(Bit ? 65 : 5);
}
//-------------------

Boolean DS18B20_ReadBit()
{
	Boolean Bit;

	DS18B20_SetDQ(false);
	DELAY_US(5);
	DS18B20_SetDQ(true);
	DELAY_US(10);

	Bit = DS18B20_ReadDQ();

	DELAY_US(55);

	return Bit;
}
//-------------------

void DS18B20_SetDQ(Boolean State)
{
	GPIO_SetState(GPIO_ADAPTER_ID_CTRL, DQ_WRITE_INVERSION ? !State : State);
}
//-------------------

Boolean DS18B20_ReadDQ()
{
	return DQ_READ_INVERSION ? !GPIO_GetState(GPIO_ADAPTER_ID_DATA) : GPIO_GetState(GPIO_ADAPTER_ID_DATA);
}
//-------------------

void DS18B20_StrongPullUpDQ(Boolean State)
{
	GPIO_SetState(GPIO_ADAPTER_ID_PWR, (DQ_STRONG_PULLUP_INVERSION) ? !State : State);
}
//-------------------
