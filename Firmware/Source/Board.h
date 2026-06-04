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

// Interface pins
GPIO_PortPinSettingMacro GPIO_ALT_UART1_TX		= {GPIOA, Pin_9};
GPIO_PortPinSettingMacro GPIO_ALT_UART1_RX		= {GPIOA, Pin_10};
GPIO_PortPinSettingMacro GPIO_ALT_CAN1_TX		= {GPIOA, Pin_12};
GPIO_PortPinSettingMacro GPIO_ALT_CAN1_RX		= {GPIOA, Pin_11};

// Stub pool (application GPIO — same physical pin until board pinout is defined)
GPIO_PortPinSettingMacro GPIO_STUB_A			= {GPIOA, Pin_0};

GPIO_PortPinSettingMacro GPIO_STPM_STEP			= {GPIOA, Pin_0};
GPIO_PortPinSettingMacro GPIO_STPM_DIR			= {GPIOA, Pin_0};
GPIO_PortPinSettingMacro GPIO_STPM_EN			= {GPIOA, Pin_0};
GPIO_PortPinSettingMacro GPIO_FAN				= {GPIOA, Pin_0};
GPIO_PortPinSettingMacro GPIO_SAFETY_IN			= {GPIOA, Pin_0};
GPIO_PortPinSettingMacro GPIO_HOME				= {GPIOA, Pin_0};
GPIO_PortPinSettingMacro GPIO_SEN_BUS			= {GPIOA, Pin_0};
GPIO_PortPinSettingMacro GPIO_SEN_ADAPTER		= {GPIOA, Pin_0};
GPIO_PortPinSettingMacro GPIO_OUT_POWER			= {GPIOA, Pin_0};
GPIO_PortPinSettingMacro GPIO_OUT_CONTROL		= {GPIOA, Pin_0};
GPIO_PortPinSettingMacro GPIO_SPIMUX_A			= {GPIOA, Pin_0};
GPIO_PortPinSettingMacro GPIO_SPIMUX_B			= {GPIOA, Pin_0};
GPIO_PortPinSettingMacro GPIO_SPIMUX_C			= {GPIOA, Pin_0};
GPIO_PortPinSettingMacro GPIO_ADAPTER_ID_PWR	= {GPIOA, Pin_0};
GPIO_PortPinSettingMacro GPIO_ADAPTER_ID_CTRL	= {GPIOA, Pin_0};
GPIO_PortPinSettingMacro GPIO_ADAPTER_ID_DATA	= {GPIOA, Pin_0};

GPIO_PortPinSettingMacro GPIO_LED				= {GPIOB, Pin_10};
GPIO_PortPinSettingMacro GPIO_MEASURE_PRESS		= {GPIOA, Pin_4};

#endif // __BOARD_H
