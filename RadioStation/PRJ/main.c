#include "bsp_board.h"
#include "bsp_timer.h"

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
