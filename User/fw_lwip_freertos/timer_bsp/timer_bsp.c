//=======================================================================================


//=======================================================================================

#include "timer_bsp.h"

TIM_HandleTypeDef TIM3_Handle;
volatile unsigned long long FreeRTOSRunTimeTicks;

//=========================================
// timer cfg for freertos
//=========================================

void TIM3_BSP_Init(uint16_t arr, uint16_t psc)
{
	TIM3_Handle.Instance = TIM3;
	TIM3_Handle.Init.Prescaler = psc;
	TIM3_Handle.Init.CounterMode = TIM_COUNTERMODE_UP;
	TIM3_Handle.Init.Period = arr;
	TIM3_Handle.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	
	HAL_TIM_Base_Init(&TIM3_Handle);
	
	HAL_NVIC_SetPriority(TIM3_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(TIM3_IRQn); 
	
	HAL_TIM_Base_Start_IT(&TIM3_Handle);
}

//=========================================
// tick for freertos
// 100us
//=========================================

void ConfigureTimerForRunTimeStatus(void)
{
	FreeRTOSRunTimeTicks = 0;
	TIM3_BSP_Init(10-1, 200-1);
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *TIM_Handle)
{
    if(TIM_Handle->Instance == TIM3)
		{
				FreeRTOSRunTimeTicks++;
		}
}

void TIM3_IRQHandler(void)
{
    HAL_TIM_IRQHandler(&TIM3_Handle);
}
