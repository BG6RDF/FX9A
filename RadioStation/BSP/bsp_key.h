#ifndef __BSP_KEY__
#define __BSP_KEY__

#include "bsp_board.h"

typedef enum {
  keyNull,
  keyShot,
  keyLong,
}KEY_State;

typedef struct{
  KEY_State state;
  uint8_t val;
}KEY_AttrDef;

void key_init();
bool key_ispress(KEY_AttrDef *rkey);
void key_clear();
void key_calback(void);
void rotary_encoder();
void KEY_SoftwareSet(uint8_t kval, KEY_State state);
void key_readkeyer();

extern bool PttPressed;
extern bool PttFinish;

#endif