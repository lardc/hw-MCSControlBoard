#ifndef DS18B20_H_
#define DS18B20_H_

// Includes
//
#include "ZwDSP.h"

// Definitions
//
// DS18B20 ROM commands
#define DS18B20_READ_ROM			0x33
#define DS18B20_MATCH_ROM			0x55
#define DS18B20_SKIP_ROM			0xCC
#define DS18B20_SEARCH_ROM			0xF0
#define DS18B20_ALARM_SEARCH		0xEC

// DS18B20 Memory commands
#define DS18B20_WRITE_SCRATCHPAD	0x4E
#define DS18B20_READ_SCRATCHPAD		0xBE
#define DS18B20_COPY_SCRATCHPAD		0x48
#define DS18B20_RECALL_E2			0xB8
#define DS18B20_READ_PWR_SUPPLY		0xB4

// DS18B20 registers bits
// CONFIG
#define CONFIG_RES_9BIT 			0x1F
#define CONFIG_RES_10BIT 			0x3F
#define CONFIG_RES_11BIT 			0x5F
#define CONFIG_RES_12BIT 			0x7F

// DS18B20 registers
#define REG_TEMPERATURE_LSB			0
#define REG_TEMPERATURE_MSB			1
#define REG_USER_BYTE_1				2
#define REG_USER_BYTE_2				3
#define REG_COUNT_REMAIN			6
#define REG_COUNT_PER_CELSIUS		7
#define REG_CRC						8

// Functions
//
Boolean DS18B20_WriteReg(pInt16U Data);
Boolean DS18B20_ReadReg(pInt16U Data);
Boolean DS18B20_ReadROM(pInt16U Data);

#endif /* DS18B20_H_ */
