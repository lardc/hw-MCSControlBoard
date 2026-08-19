// Include
#include "Interrupts.h"
//
#include "Controller.h"
#include "LowLevel.h"
#include "Global.h"
#include "StepperMotor.h"
#include "ZwUSART.h"
#include "ZwTIM.h"

extern volatile Int16U SM_LogScaleCoef;
extern volatile Int16U SM_LogScaleCounter;

// Functions
//
static void CONTROL_MotorLogSample()
{
	if(!SM_IsLogging() || CONTROL_ValuesCounter >= VALUES_x_SIZE)
		return;

	if(SM_LogScaleCounter > 1)
	{
		SM_LogScaleCounter--;
		return;
	}

	SM_LogScaleCounter = SM_LogScaleCoef;
	CONTROL_MotorMovement[CONTROL_ValuesCounter] = SM_GetPositionMm();
	CONTROL_MotorSpeed[CONTROL_ValuesCounter] = SM_GetSpeedMmS();
	CONTROL_ValuesCounter++;
}
//-----------------------------------------
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

		CONTROL_MotorLogSample();

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
