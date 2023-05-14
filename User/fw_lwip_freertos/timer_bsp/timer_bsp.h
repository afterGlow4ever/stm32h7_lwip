//=======================================================================================


//=======================================================================================

#include "stm32h7xx.h"

#ifndef _TIMER_BSP_
#define _TIMER_BSP_

void TIM3_BSP_Init(uint16_t arr, uint16_t psc);
void ConfigureTimerForRunTimeStatus(void);
extern volatile unsigned long long FreeRTOSRunTimeTicks;

#endif
