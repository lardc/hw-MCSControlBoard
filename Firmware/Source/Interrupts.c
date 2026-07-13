// Include
#include "Interrupts.h"
//
#include "Controller.h"
#include "LowLevel.h"
#include "Global.h"
#include "StepperMotor.h"
#include "ZwUSART.h"
#include "ZwTIM.h"

// Functions
//
void USART2_IRQHandler()
{
	if(USARTx_RecieveCheck(USART2))
	{
		USARTx_RegisterToFIFO(USART2);
		USARTx_OverrunFlagClear(USART2);
	}
}
//-----------------------------------------
//Сделана временная замена на время тестов на SVTU
void USART1_IRQHandler()
{
	if(USARTx_RecieveCheck(USART1))
	{
		USARTx_RegisterToFIFO(USART1);
		USARTx_OverrunFlagClear(USART1);
	}
}
//-----------------------------------------

void USB_LP_CAN_RX0_IRQHandler()
{
	if(NCAN_RecieveCheck())
	{
		NCAN_RecieveData();
		NCAN_RecieveFlagReset();
	}
}
//-----------------------------------------

void TIM7_IRQHandler()
{
	static uint16_t LED_BlinkTimeCounter = 0;

	if(TIM_StatusCheck(TIM7))
	{
		CONTROL_TimeCounter++;
		if(++LED_BlinkTimeCounter > TIME_LED_BLINK)
		{
			LL_ToggleBoardLED();
			LED_BlinkTimeCounter = 0;
		}

		TIM_StatusClear(TIM7);
	}
}

void TIM1_UP_TIM16_IRQHandler(void)
{
	if(TIM_StatusCheck(TIM1))
		TIM_StatusClear(TIM1);
}
//-----------------------------------------

void TIM3_IRQHandler(void)
{
	if(TIM_StatusCheck(TIM3))
	{
		SM_TimerHandler();
		TIM_StatusClear(TIM3);
	}
}
//-----------------------------------------
