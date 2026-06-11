#ifndef __ONEWIRE_H
#define __ONEWIRE_H

#include "stdinc.h"
#include "ZwGPIO.h"

#ifndef ONEWIRE_CRC8_TABLE
#define ONEWIRE_CRC8_TABLE 1
#endif

#ifndef ONEWIRE_CRC16
#define ONEWIRE_CRC16 1
#endif

// Инициализация шины: writePin, readPin, powerPin, usePowerPin — использовать powerPin для parasite power
void OneWire_Init(GPIO_PortPinSetting writePin, GPIO_PortPinSetting readPin, GPIO_PortPinSetting powerPin,
		Boolean usePowerPin);

// Обмен данными
Int8U OneWire_Reset();
void OneWire_Select(const Int8U *rom);
void OneWire_Skip();
void OneWire_Write(Int8U value, Int8U power);
void OneWire_WriteBytes(const Int8U *buf, Int16U count, Boolean power);
Int8U OneWire_Read();
void OneWire_ReadBytes(Int8U *buf, Int16U count);
void OneWire_WriteBit(Int8U value);
Int8U OneWire_ReadBit();
void OneWire_Depower();

// Поиск устройств на шине
void OneWire_ResetSearch();
void OneWire_TargetSearch(Int8U familyCode);
Boolean OneWire_Search(Int8U *newAddr, Boolean searchMode);

// Контрольные суммы Dallas
Int8U OneWire_Crc8(const Int8U *addr, Int8U len);

#if ONEWIRE_CRC16
Boolean OneWire_CheckCrc16(const Int8U *input, Int16U len, const Int8U *invertedCrc, Int16U crc);
Int16U OneWire_Crc16(const Int8U *input, Int16U len, Int16U crc);
#endif

#endif // __ONEWIRE_H
