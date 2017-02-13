#include "bsp_board.h"
#include "bsp_timer.h"

void main()
{ 
  /* Setup STM32 system (clock, PLL and Flash configuration) */
  SystemInit(); // 24MHz����system_stm32f10x.c�����߶�����SYSCLK_FREQ_24MHZ�꣬BG6RDF
  //RCC_Configuration();
  board_init();
  
  TMR_Start(TIM2);
  while(1)
  {
    board_task();
  }
}
