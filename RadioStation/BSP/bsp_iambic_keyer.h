#ifndef __BSP_IAMBIC_KEYER_H__
#define __BSP_IAMBIC_KEYER_H__

#include "bsp_board.h"

#define MORSE_CODE_COUNT 37

typedef enum{
  IAMBIC_A,
  IAMBIC_B,
}KEYER_Mode;

typedef struct{
  KEYER_Mode mode;
  
}KEYER_AttrDef;

void keyer_init();
void keyer_auto_setup();
void keyer_manual_task();
void keyer_auto_task();
void call_enter();
void call_task();
void call_exit();
#endif