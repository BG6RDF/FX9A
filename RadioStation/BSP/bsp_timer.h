#ifndef __BSP_TIMER_H__
#define __BSP_TIMER_H__

#include "bsp_board.h"

void TMR_Init(TIM_TypeDef *timer, uint32_t hz, uint32_t period, CallBack cBack);
void TMR_Start(TIM_TypeDef *timer);
void TMR_Stop(TIM_TypeDef *timer);
uint16_t TMR_GetTime(TIM_TypeDef *timer);
void TMR_SetTime(TIM_TypeDef *timer, uint16_t cnt);
void TMR_IRQEnable(TIM_TypeDef *timer);
#endif