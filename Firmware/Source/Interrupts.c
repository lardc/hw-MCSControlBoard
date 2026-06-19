// Include
#include "Interrupts.h"
//
#include "Controller.h"
#include "LowLevel.h"
#include "Global.h"
#include "StepperMotor.h"
#include "ZwUSART.h"

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

void USART3_IRQHandler()
{
	if(USARTx_RecieveCheck(USART3))
	{
		USARTx_RegisterToFIFO(USART3);
		USARTx_OverrunFlagClear(USART3);
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
	{
		SM_TimerHandler();
		TIM_StatusClear(TIM1);
	}
}
//-----------------------------------------
