#include "bsp_timer.h"

CallBack Tmr0CallBack;
CallBack Tmr2CallBack;
CallBack Tmr4CallBack;

void TMR_NVIC_Config()
{
  NVIC_InitTypeDef NVIC_InitStructure;  
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); 
  NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn; // 按键
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3; 
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;  
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;  
  NVIC_Init(&NVIC_InitStructure);

  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); 
  NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;  // 1ms 定时器
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0; 
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;  
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;  
  NVIC_Init(&NVIC_InitStructure);
  
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); 
  NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;  // 側音
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0; 
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;  
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;  
  NVIC_Init(&NVIC_InitStructure);
  
}
void TMR_Init(TIM_TypeDef *timer, uint32_t hz, uint32_t period, CallBack cBack)
{
  uint32_t prescaler;

  TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure; 
  
  TMR_NVIC_Config();
  
  if( timer == TIM2 )
  {
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2,ENABLE);
    Tmr2CallBack = cBack;
  }
  else if( timer == TIM3)
  {
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3,ENABLE);
  }
  else if( timer == TIM4)
  {
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4,ENABLE);
    Tmr4CallBack = cBack;
  }
  
  TIM_DeInit(timer);  
  
  prescaler = SystemFrequency_AHBClk / (hz * period);
    
  TIM_TimeBaseStructure.TIM_Period = period - 1;  //自动重装载寄存器的值  
  
  TIM_TimeBaseStructure.TIM_Prescaler = (prescaler - 1);     //时钟预分频数  
  
  TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;    //采样分频  
  
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; //向上计数模式 
  
  TIM_TimeBaseInit(timer, &TIM_TimeBaseStructure);  
  
  TIM_ClearFlag(timer, TIM_FLAG_Update);               //清除溢出中断标志  
  TIM_ARRPreloadConfig(timer, ENABLE);
  
}

uint16_t TMR_GetTime(TIM_TypeDef *timer)
{
  return timer->CNT;
}

void TMR_SetTime(TIM_TypeDef *timer, uint16_t cnt)
{
  timer->CNT = cnt;
}

void TMR_IRQEnable(TIM_TypeDef *timer)
{
  TIM_ITConfig(timer, TIM_IT_Update, ENABLE); 
}

void TMR_Start(TIM_TypeDef *timer)
{
  TIM_Cmd(timer,ENABLE); 
}

void TMR_Stop(TIM_TypeDef *timer)
{
  TIM_Cmd(timer,DISABLE); 
}

void TIM2_IRQHandler(void)
{
  if(TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)     //检测是否发生溢出更新事件  
  {
    TIM_ClearITPendingBit(TIM2 , TIM_FLAG_Update);      //清除TIM2的中断待处理位 
    (*Tmr2CallBack)();  
  }
}

void TIM4_IRQHandler(void)
{
  if(TIM_GetITStatus(TIM4, TIM_IT_Update) != RESET)     //检测是否发生溢出更新事件  
  {
    TIM_ClearITPendingBit(TIM4 , TIM_FLAG_Update);      //清除TIM2的中断待处理位 
    (*Tmr4CallBack)();  
  }
}