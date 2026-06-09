#ifndef __LOWLEVEL_H
#define __LOWLEVEL_H

#include "Board.h"
#include "stdinc.h"

// --- Выход (1SPI), индекс = бит_ТТ - 1 ---
#define SPI_OUT_ADAPTER			0
#define SPI_OUT_BUS				1
#define SPI_OUT_FAN1			6
#define SPI_OUT_FAN2			7

#define SPI_OUT_MASK_ADAPTER	(1u << SPI_OUT_ADAPTER)
#define SPI_OUT_MASK_BUS		(1u << SPI_OUT_BUS)
#define SPI_OUT_MASK_FANS		((1u << SPI_OUT_FAN1) | (1u << SPI_OUT_FAN2))

// --- Вход (2SPI), индекс = бит_ТТ - 1 ---
#define SPI_IN_ADAPTER_COIL_24V	0
#define SPI_IN_BUS_COIL_24V		1
#define SPI_IN_ADAPTER_RELEASED	2
#define SPI_IN_ADAPTER_HELD		3
#define SPI_IN_BUS_RELEASED		4
#define SPI_IN_BUS_HELD			5

#define SPI_IN_MASK_COIL_24V	((1u << SPI_IN_ADAPTER_COIL_24V) | (1u << SPI_IN_BUS_COIL_24V))

void LL_InitGPIO();

Boolean LL_FilterSafetyCircuit(Boolean NewState);

void LL_SPI_SetOutBit(Int8U Bit, Boolean State);
void LL_SPI_FlushOut();
Boolean LL_SPI_GetInBit(Int8U Bit);
Boolean LL_SPI_IsCoil24VOk();
Int8U LL_SPI_ReadInRaw();

Boolean LL_IsTableSensorOk();
Boolean LL_IsSafetyS3Ok();
Boolean LL_IsSafetyS5Ok();
Boolean LL_HomeSensorActuate();

void LL_SetTestLine(Boolean State);
void LL_RS485_SetTxMode(Boolean State);

void LL_SwitchStep(Boolean State);
void LL_ToggleStep();
void LL_SwitchUpDir(Boolean State);
Boolean LL_IsDirUp();
void LL_SwitchEnable(Boolean State);

void LL_ToggleBoardLED();

#endif // __LOWLEVEL_H
