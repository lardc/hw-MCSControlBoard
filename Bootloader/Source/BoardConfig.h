// -----------------------------------------
// Board parameters
// ----------------------------------------

#ifndef __BOARD_CONFIG_H
#define __BOARD_CONFIG_H

// Board includes
//
#include "ZwRCC.h"
#include "ZwGPIO.h"
#include "ZwNCAN.h"
#include "ZwUSART.h"
#include "ZwTIM.h"
#include "ZwIWDG.h"
#include "ZwNFLASH.h"

// Definitions
#define CONFIG_USE_USART2

// Blinking LED settings 
#define LED_BLINK_PORT		GPIOA
#define LED_BLINK_PIN		Pin_4

#endif // __BOARD_CONFIG_H
