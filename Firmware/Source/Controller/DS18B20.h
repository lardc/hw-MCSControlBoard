#ifndef DS18B20_H_
#define DS18B20_H_

#include "stdinc.h"

#define DS18B20_FAMILY_CODE			0x28

// DS18B20 ROM commands
#define DS18B20_READ_ROM			0x33
#define DS18B20_MATCH_ROM			0x55
#define DS18B20_SKIP_ROM			0xCC
#define DS18B20_SEARCH_ROM			0xF0
#define DS18B20_ALARM_SEARCH		0xEC

// DS18B20 function commands
#define DS18B20_WRITE_SCRATCHPAD	0x4E
#define DS18B20_READ_SCRATCHPAD		0xBE
#define DS18B20_COPY_SCRATCHPAD		0x48
#define DS18B20_RECALL_E2			0xB8
#define DS18B20_READ_PWR_SUPPLY		0xB4
#define DS18B20_CONVERT_T				0x44

// CONFIG register (resolution)
#define CONFIG_RES_9BIT 			0x1F
#define CONFIG_RES_10BIT 			0x3F
#define CONFIG_RES_11BIT 			0x5F
#define CONFIG_RES_12BIT 			0x7F

// Scratchpad
#define DS18B20_SCRATCHPAD_SIZE		9
#define REG_TEMPERATURE_LSB			0
#define REG_TEMPERATURE_MSB			1
#define REG_USER_BYTE_1				2	// TH (EEPROM)
#define REG_USER_BYTE_2				3	// TL (EEPROM)
#define REG_CONFIGURATION			4
#define REG_CRC						8

void DS18B20_Init();

// Запись / чтение 16-битного слова в EEPROM-байты TH и TL (scratchpad [2..3])
Boolean DS18B20_WriteReg(pInt16U Data);
Boolean DS18B20_ReadReg(pInt16U Data);

// Температура в 0.1°C (например, 253 = 25.3°C)
Boolean DS18B20_ReadTemperatureC10(pInt16S Data);

#endif /* DS18B20_H_ */
