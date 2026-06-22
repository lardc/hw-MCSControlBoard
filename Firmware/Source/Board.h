#ifndef __BOARD_H
#define __BOARD_H

#include "stm32f30x.h"

#include "ZwRCC.h"
#include "ZwGPIO.h"
#include "ZwNCAN.h"
#include "ZwTIM.h"
#include "ZwADC.h"
#include "ZwEXTI.h"
#include "ZwSCI.h"
#include "ZwIWDG.h"
#include "ZwNFLASH.h"

// Alternate-function pins
GPIO_PortPinSettingMacro GPIO_ALT_UART2_TX		= {GPIOA, Pin_2};
GPIO_PortPinSettingMacro GPIO_ALT_UART2_RX		= {GPIOA, Pin_3};
GPIO_PortPinSettingMacro GPIO_ALT_UART3_TX		= {GPIOA, Pin_9};
GPIO_PortPinSettingMacro GPIO_ALT_UART3_RX		= {GPIOA, Pin_10};
GPIO_PortPinSettingMacro GPIO_ALT_CAN1_RX 		= {GPIOA, Pin_11};
GPIO_PortPinSettingMacro GPIO_ALT_CAN1_TX 		= {GPIOA, Pin_12};
GPIO_PortPinSettingMacro GPIO_ALT_SPI3_SCK		= {GPIOB, Pin_3};
GPIO_PortPinSettingMacro GPIO_ALT_SPI3_MISO		= {GPIOB, Pin_4};
GPIO_PortPinSettingMacro GPIO_ALT_SPI3_MOSI		= {GPIOB, Pin_5};

// Application GPIO
//Заменены некоторые выходы пинов на другие на время тестов на SVTU;
GPIO_PortPinSettingMacro GPIO_LED				= {GPIOA, Pin_8};
GPIO_PortPinSettingMacro GPIO_RS485_CTRL		= {GPIOA, Pin_1};
// 0 — SVTU: один пин DQ; 1 — MCS: CTRL / IN / PWR
#define ONEWIRE_THREE_PIN_BUS					0
#if ONEWIRE_THREE_PIN_BUS
GPIO_PortPinSettingMacro GPIO_DQ_PWR			= {GPIOA, Pin_5};
GPIO_PortPinSettingMacro GPIO_DQ_IN			= {GPIOA, Pin_6};
GPIO_PortPinSettingMacro GPIO_DQ_CTRL			= {GPIOA, Pin_7};
#else
GPIO_PortPinSettingMacro GPIO_DQ				= {GPIOB, Pin_15};	// parasite power, подтяжка 4.7 кОм
GPIO_PortPinSettingMacro GPIO_DQ_CTRL			= {GPIOB, Pin_15};
#endif
GPIO_PortPinSettingMacro GPIO_SEN_S1			= {GPIOA, Pin_0};
GPIO_PortPinSettingMacro GPIO_SEN_S3			= {GPIOA, Pin_0};
GPIO_PortPinSettingMacro GPIO_SEN_S2			= {GPIOA, Pin_0};
GPIO_PortPinSettingMacro GPIO_SEN_S4			= {GPIOA, Pin_0};
GPIO_PortPinSettingMacro GPIO_SEN_S5			= {GPIOA, Pin_0};
GPIO_PortPinSettingMacro GPIO_SPI_LD			= {GPIOA, Pin_15};
GPIO_PortPinSettingMacro GPIO_STPM_DIR			= {GPIOB, Pin_0};
GPIO_PortPinSettingMacro GPIO_STPM_STEP			= {GPIOB, Pin_1};
GPIO_PortPinSettingMacro GPIO_STPM_EN			= {GPIOB, Pin_2};
GPIO_PortPinSettingMacro GPIO_SPI_OE			= {GPIOB, Pin_6};
GPIO_PortPinSettingMacro GPIO_SPI_SS			= {GPIOB, Pin_7};
GPIO_PortPinSettingMacro GPIO_HOMING			= {GPIOB, Pin_12};
GPIO_PortPinSettingMacro GPIO_TEST				= {GPIOB, Pin_13};
GPIO_PortPinSettingMacro GPIO_MEASURE_PRESS		= {GPIOA, Pin_4};

#endif // __BOARD_H
