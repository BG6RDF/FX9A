#include "bsp_board.h"
#include "bsp_timer.h"

void RCC_Configuration(void)
{
  RCC_DeInit();
 
  RCC_HSICmd(ENABLE);
 
  while(RCC_GetFlagStatus(RCC_FLAG_HSIRDY) == RESET)
  {
 
  } 
 
  if(1)
  { 
   //FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);
  
   //FLASH_SetLatency(FLASH_Latency_2);
  
   RCC_HCLKConfig(RCC_SYSCLK_Div1); //AHB时钟为系统时钟SYSCLK
  
   RCC_PCLK2Config(RCC_HCLK_Div1);  //APB1时钟为HCLK/2，其中HCLK为AHB时钟
  
   RCC_PCLK1Config(RCC_HCLK_Div2);  //APB2时钟为HCLK
  
   //设置 PLL 时钟源及倍频系数
  
   RCC_PLLConfig(RCC_PLLSource_HSI_Div2, RCC_PLLMul_8);
  
   //使能或者失能 PLL,这个参数可以取：ENABLE或者DISABLE
  
   RCC_PLLCmd(ENABLE);//如果PLL被用于系统时钟,那么它不能被失能
  
   //等待指定的 RCC 标志位设置成功 等待PLL初始化成功
  
   while(RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET)
   {
  
   }
   //设置系统时钟（SYSCLK） 设置PLL为系统时钟源  
   RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);
   //等待PLL成功用作于系统时钟的时钟源
     
   // 0x00：HSI 作为系统时钟
  
   // 0x04：HSE作为系统时钟
  
   // 0x08：PLL作为系统时钟
  
   while(RCC_GetSYSCLKSource() != 0x08)
   {
  
   }
  
   }
//   RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB |
//        RCC_APB2Periph_GPIOC | RCC_APB2Periph_AFIO, ENABLE);
  
//   RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1 | RCC_APB1Periph_I2C2, ENABLE);
  
//   RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
}

void main()
{ 
  /* Setup STM32 system (clock, PLL and Flash configuration) */
  SystemInit(); // 24MHz，在system_stm32f10x.c中作者定义了SYSCLK_FREQ_24MHZ宏，BG6RDF
  //RCC_Configuration();
  board_init();
  
  TMR_Start(TIM2);
  while(1)
  {
    board_task();
  }
}
