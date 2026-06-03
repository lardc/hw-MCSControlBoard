#ifndef __BOARD_H
#define __BOARD_H

#include "stm32f30x.h"

#include "ZwRCC.h"
#include "ZwGPIO.h"
#include "ZwNCAN.h"
#include "ZwTIM.h"
#include "ZwDAC.h"
#include "ZwDMA.h"
#include "ZwADC.h"
#include "ZwEXTI.h"
#include "ZwSCI.h"
#include "ZwIWDG.h"
#include "ZwNFLASH.h"
#include "ZwSPI.h"

// Определения для выходных портов
GPIO_PortPinSettingMacro GPIO_LED				= {GPIOB, Pin_10};
GPIO_PortPinSettingMacro GPIO_IND_CSM			= {GPIOB, Pin_5};
GPIO_PortPinSettingMacro GPIO_IND_ADPTR			= {GPIOB, Pin_6};
GPIO_PortPinSettingMacro GPIO_PNEUM_TOP			= {GPIOB, Pin_7};
GPIO_PortPinSettingMacro GPIO_PNEUM_BOT			= {GPIOB, Pin_8};
GPIO_PortPinSettingMacro GPIO_PNEUM_DUT			= {GPIOB, Pin_9};
GPIO_PortPinSettingMacro GPIO_SF_OUT			= {GPIOB, Pin_2};

// Определения для входных портов
GPIO_PortPinSettingMacro GPIO_DUT_1				= {GPIOB, Pin_12};
GPIO_PortPinSettingMacro GPIO_DUT_2				= {GPIOB, Pin_13};
GPIO_PortPinSettingMacro GPIO_DUT_3				= {GPIOB, Pin_14};
GPIO_PortPinSettingMacro GPIO_DUT_4				= {GPIOA, Pin_15};
GPIO_PortPinSettingMacro GPIO_ADPTR_TOP			= {GPIOB, Pin_3};
GPIO_PortPinSettingMacro GPIO_ADPTR_BOT			= {GPIOB, Pin_4};

// Определения для аналоговых портов
GPIO_PortPinSettingMacro GPIO_MEASURE_BOT 		= {GPIOA, Pin_1};
GPIO_PortPinSettingMacro GPIO_MEASURE_TOP 		= {GPIOA, Pin_2};
GPIO_PortPinSettingMacro GPIO_MEASURE_PRESS		= {GPIOA, Pin_4};

// Определения для портов альтернативных функций
GPIO_PortPinSettingMacro GPIO_ALT_UART1_TX		= {GPIOA, Pin_9};
GPIO_PortPinSettingMacro GPIO_ALT_UART1_RX		= {GPIOA, Pin_10};
GPIO_PortPinSettingMacro GPIO_ALT_CAN1_TX		= {GPIOA, Pin_12};
GPIO_PortPinSettingMacro GPIO_ALT_CAN1_RX		= {GPIOA, Pin_11};

// Определения для портов внешних прерываний
//

#endif // __BOARD_H
