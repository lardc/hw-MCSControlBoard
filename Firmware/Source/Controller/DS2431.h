#ifndef DS2431_H_
#define DS2431_H_

#include "stdinc.h"

// Defines
#define DS2431_ONE_WIRE_MAC_SIZE		8
#define DS2431_ONE_WIRE_FAMILY_CODE		0x2D

#define DS2431_EEPROM_SIZE				128
#define DS2431_ROW_SIZE					8

#define DS2431_PF_MASK					0x07
#define DS2431_WRITE_MASK				0xAA

#define DS2431_CMD_SIZE					3
#define DS2431_CRC_SIZE					2
#define DS2431_READ_RETRY				2

#define DS2431_BUFFER_SIZE				(DS2431_ROW_SIZE + DS2431_CMD_SIZE + DS2431_CRC_SIZE)

#define DS2431_WRITE_SCRATCHPAD			0x0F
#define DS2431_READ_SCRATCHPAD			0xAA
#define DS2431_COPY_SCRATCHPAD			0x55
#define DS2431_READ_MEMORY				0xF0

#define DS2431_COPY_DELAY_US			15000	// t_PROG = 12.5 ms worst case

#define DS2431_ERASE_BYTE				0xFF

// Functions
// Установить ROM-адрес устройства (family code + serial + CRC из OneWire_Search).
// После вызова адресация идёт через MATCH ROM вместо SKIP ROM.
void DS2431_Begin(Int8U serialNumber[DS2431_ONE_WIRE_MAC_SIZE]);

// Чтение одного байта из EEPROM
Int8U DS2431_ReadByte(Int16U address);

// Чтение нескольких байт из EEPROM
void DS2431_Read(Int16U address, Int8U *buf, Int16U len);

// Запись до 8 байт; address должен быть кратен 8.
// После вызова шина 1-Wire должна быть обесточена (OneWire_Depower вызывается внутри).
Boolean DS2431_Write(Int16U address, const Int8U *buf, Int16U count, Boolean verify);

// Стирание области данных EEPROM (0x00..0x7F): запись 0xFF во все 16 строк по 8 байт.
// Регистры защиты (0x80..) не затрагиваются. Защищённые страницы не стираются — функция вернёт false.
Boolean DS2431_EraseAll(Boolean verify);

// Чтение массива с адреса 0. len не должен превышать DS2431_EEPROM_SIZE.
Boolean DS2431_ReadData(Int8U *buf, Int16U len);

// Запись массива с адреса 0. len не должен превышать DS2431_EEPROM_SIZE.
Boolean DS2431_WriteData(const Int8U *buf, Int16U len);

#endif /* DS2431_H_ */
