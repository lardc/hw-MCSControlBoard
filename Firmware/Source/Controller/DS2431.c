// Header
#include "DS2431.h"

// Includes
#include "OneWire.h"
#include "Delay.h"

// Variables
static Int8U SerialNumber[DS2431_ONE_WIRE_MAC_SIZE];
static Boolean SkipRom = true;

static const Int8U EraseRow[DS2431_ROW_SIZE] =
{
	DS2431_ERASE_BYTE, DS2431_ERASE_BYTE, DS2431_ERASE_BYTE, DS2431_ERASE_BYTE,
	DS2431_ERASE_BYTE, DS2431_ERASE_BYTE, DS2431_ERASE_BYTE, DS2431_ERASE_BYTE
};

// Forward functions
static void DS2431_StartTransmission();
static Boolean DS2431_WriteInternal(Int16U address, const Int8U *buf, Int16U count, Boolean verify);

// Functions
void DS2431_Begin(Int8U serialNumber[DS2431_ONE_WIRE_MAC_SIZE])
{
	for (Int8U i = 0; i < DS2431_ONE_WIRE_MAC_SIZE; i++)
		SerialNumber[i] = serialNumber[i];

	SkipRom = false;
}
//-------------------

Int8U DS2431_ReadByte(Int16U address)
{
	Int8U res = 0xFF;

	DS2431_Read(address, &res, 1);
	return res;
}
//-------------------

void DS2431_Read(Int16U address, Int8U *buf, Int16U len)
{
	DS2431_StartTransmission();

	OneWire_Write(DS2431_READ_MEMORY, 1);
	OneWire_Write((Int8U)(address & 0xFF), 1);
	OneWire_Write((Int8U)((address >> 8) & 0xFF), 1);

	for (Int16U i = 0; i < len; i++)
		buf[i] = OneWire_Read();

	OneWire_Depower();
}
//-------------------

Boolean DS2431_Write(Int16U address, const Int8U *buf, Int16U count, Boolean verify)
{
	Boolean ret = DS2431_WriteInternal(address, buf, count, verify);

	OneWire_Depower();
	return ret;
}
//-------------------

Boolean DS2431_EraseAll(Boolean verify)
{
	for (Int16U address = 0; address < DS2431_EEPROM_SIZE; address += DS2431_ROW_SIZE)
	{
		if (!DS2431_Write(address, EraseRow, DS2431_ROW_SIZE, verify))
			return false;
	}

	return true;
}
//-------------------

Boolean DS2431_ReadData(Int8U *buf, Int16U len)
{
	if (len > DS2431_EEPROM_SIZE)
		return false;

	if (len > 0)
		DS2431_Read(0, buf, len);

	return true;
}
//-------------------

Boolean DS2431_WriteData(const Int8U *buf, Int16U len)
{
	Int8U row[DS2431_ROW_SIZE];

	if (len > DS2431_EEPROM_SIZE)
		return false;

	for (Int16U address = 0; address < len; address += DS2431_ROW_SIZE)
	{
		Int16U chunk = len - address;

		if (chunk > DS2431_ROW_SIZE)
			chunk = DS2431_ROW_SIZE;

		if (chunk == DS2431_ROW_SIZE)
		{
			if (!DS2431_Write(address, &buf[address], DS2431_ROW_SIZE, false))
				return false;
		}
		else
		{
			DS2431_Read(address, row, DS2431_ROW_SIZE);

			for (Int16U i = 0; i < chunk; i++)
				row[i] = buf[address + i];

			if (!DS2431_Write(address, row, DS2431_ROW_SIZE, false))
				return false;
		}
	}

	return true;
}
//-------------------

static void DS2431_StartTransmission()
{
	OneWire_Reset();

	if (SkipRom)
		OneWire_Skip();
	else
		OneWire_Select(SerialNumber);
}
//-------------------

static Boolean DS2431_WriteInternal(Int16U address, const Int8U *buf, Int16U count, Boolean verify)
{
	Int8U errorCount = 0;
	Int8U buffer[DS2431_BUFFER_SIZE];
	Int8U crc16[DS2431_CRC_SIZE];

	if (address >= DS2431_EEPROM_SIZE || (address % DS2431_ROW_SIZE) != 0)
		return false;

	buffer[0] = DS2431_WRITE_SCRATCHPAD;
	buffer[1] = (Int8U)(address & 0xFF);
	buffer[2] = (Int8U)((address >> 8) & 0xFF);

	for (Int16U i = 0; i < count; i++)
		buffer[DS2431_CMD_SIZE + i] = buf[i];

	DS2431_StartTransmission();
	OneWire_WriteBytes(buffer, DS2431_CMD_SIZE + count, true);
	OneWire_ReadBytes(crc16, DS2431_CRC_SIZE);

	if (!OneWire_CheckCrc16(buffer, DS2431_CMD_SIZE + count, crc16, 0))
		verify = true;

	buffer[0] = DS2431_READ_SCRATCHPAD;

	do
	{
		DS2431_StartTransmission();
		OneWire_Write(buffer[0], 0);
		OneWire_ReadBytes(&buffer[1], DS2431_CMD_SIZE);

		if (buffer[3] != DS2431_PF_MASK)
			verify = true;

		if (verify)
		{
			OneWire_ReadBytes(&buffer[4], count);
			OneWire_ReadBytes(crc16, DS2431_CRC_SIZE);

			if (!OneWire_CheckCrc16(buffer, 12, crc16, 0))
			{
				errorCount++;
				continue;
			}

			if (address != ((Int16U)buffer[2] << 8 | buffer[1]))
				return false;

			if (buffer[3] != DS2431_PF_MASK)
				return false;

			for (Int16U i = 0; i < DS2431_ROW_SIZE; i++)
			{
				if (buffer[4 + i] != buf[i])
					return false;
			}
		}

		break;
	}
	while (errorCount < DS2431_READ_RETRY);

	buffer[0] = DS2431_COPY_SCRATCHPAD;

	DS2431_StartTransmission();
	OneWire_WriteBytes(buffer, DS2431_CMD_SIZE + 1, true);
	DELAY_US(DS2431_COPY_DELAY_US);

	if (OneWire_Read() != DS2431_WRITE_MASK)
		return false;

	return true;
}
//-------------------
