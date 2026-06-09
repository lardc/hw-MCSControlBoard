#include "InitConfig.h"
#include "Board.h"
#include "SysConfig.h"
#include "BCCIxParams.h"
#include "LowLevel.h"
#include "ZwSPI.h"
#include "ZwDMA.h"

Int16U INITCFG_PressureAdcBuffer[ADC_PRESSURE_BUF_SIZE];

Boolean INITCFG_ConfigSystemClock()
{
	return RCC_PLL_HSE_Config(QUARTZ_FREQUENCY, PREDIV_4, PLL_14);
}
//------------------------------------------------

void INITCFG_ConfigIO()
{
	RCC_GPIO_Clk_EN(PORTA);
	RCC_GPIO_Clk_EN(PORTB);
	RCC_GPIO_Clk_EN(PORTC);

	GPIO_InitAnalog(GPIO_MEASURE_PRESS);

	GPIO_InitAltFunction(GPIO_ALT_UART1_RX, AltFn_7);
	GPIO_InitAltFunction(GPIO_ALT_UART1_TX, AltFn_7);
	GPIO_InitAltFunction(GPIO_ALT_UART3_RX, AltFn_7);
	GPIO_InitAltFunction(GPIO_ALT_UART3_TX, AltFn_7);
	GPIO_InitAltFunction(GPIO_ALT_CAN1_RX, AltFn_9);
	GPIO_InitAltFunction(GPIO_ALT_CAN1_TX, AltFn_9);
	GPIO_InitAltFunction(GPIO_ALT_SPI3_SCK, AltFn_6);
	GPIO_InitAltFunction(GPIO_ALT_SPI3_MISO, AltFn_6);
	GPIO_InitAltFunction(GPIO_ALT_SPI3_MOSI, AltFn_6);

	LL_InitGPIO();
}
//------------------------------------------------

void INITCFG_ConfigUART()
{
	USART_Init(USART1, SYSCLK, USART_BAUDRATE);
	USART_Recieve_Interupt(USART1, 0, true);
}
//------------------------------------------------

void INITCFG_ConfigUSART3()
{
	USART_Init(USART3, SYSCLK, USART_BAUDRATE);
}
//------------------------------------------------

void INITCFG_ConfigCAN()
{
	RCC_CAN_Clk_EN(CAN_1_ClkEN);
	NCAN_Init(SYSCLK, CAN_BAUDRATE, FALSE);
	NCAN_FIFOInterrupt(TRUE);
	NCAN_FilterInit(0, CAN_SLAVE_FILTER_ID, CAN_MASTER_FILTER_ID);
}
//------------------------------------------------

void INITCFG_ConfigSPI()
{
	SPI_Init8b(SPI3, SPI3_BAUDRATE_BITS, SPI_MSB_FIRST);
}
//------------------------------------------------

void INITCFG_ConfigRS485()
{
	GPIO_SetState(GPIO_RS485_CTRL, false);
}
//------------------------------------------------

void INITCFG_ConfigTimer7()
{
	TIM_Clock_En(TIM_7);
	TIM_Config(TIM7, SYSCLK, TIMER7_uS);
	TIM_Interupt(TIM7, 2, true);
	TIM_Start(TIM7);
}
//------------------------------------------------

void INITCFG_ConfigTimer1()
{
	TIM_Clock_En(TIM_1);
	TIM_Config(TIM1, SYSCLK, TIMER1_uS);
	TIM_Interupt(TIM1, 1, true);
	TIM_Start(TIM1);
}
//------------------------------------------------

void INITCFG_ConfigWatchDog()
{
	IWDG_Config();
	IWDG_ConfigureSlowUpdate();
}
//------------------------------------------------

void INITCFG_ConfigTimer15()
{
	TIM_Clock_En(TIM_15);
	TIM_Config(TIM15, SYSCLK, TIMER15_uS);
	TIM_MasterMode(TIM15, MMS_UPDATE);
	TIM_Start(TIM15);
}
//------------------------------------------------

void INITCFG_ConfigADC()
{
	RCC_ADC_Clk_EN(ADC_12_ClkEN);

	ADC_Calibration(ADC2);
	ADC_Enable(ADC2);
	ADC_TrigConfig(ADC2, ADC12_TIM15_TRGO, RISE);

	ADC_ChannelSeqReset(ADC2);
	ADC_ChannelSet_Sequence(ADC2, ADC_PRESSURE_CHANNEL, 1);
	ADC_ChannelSeqLen(ADC2, ADC_PRESSURE_SEQ_LENGTH);

	ADC_ChannelSet_SampleTime(ADC2, ADC_PRESSURE_CHANNEL, ADC_SMPL_TIME_4_5);
	ADC_DMAConfigWithAutDLY(ADC2);
	ADC_SamplingStart(ADC2);
}
//------------------------------------------------

void INITCFG_ConfigDMA()
{
	DMA_Clk_Enable(DMA2_ClkEN);

	DMA_Reset(DMA2_Channel1);
	DMAChannelX_DataConfig(DMA2_Channel1, (uint32_t)INITCFG_PressureAdcBuffer, (uint32_t)(&ADC2->DR),
			ADC_PRESSURE_BUF_SIZE);
	DMAChannelX_Config(DMA2_Channel1, DMA_MEM2MEM_DIS, DMA_LvlPriority_LOW, DMA_MSIZE_16BIT, DMA_PSIZE_16BIT,
			DMA_MINC_EN, DMA_PINC_DIS, DMA_CIRCMODE_EN, DMA_READ_FROM_PERIPH);
	DMA_ChannelEnable(DMA2_Channel1, true);
}
//------------------------------------------------
