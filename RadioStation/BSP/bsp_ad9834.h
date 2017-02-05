#ifndef __BSP_AD9834_H__
#define __BSP_AD9834_H__

#include "bsp_board.h"


void ad9834_init();
void ad9834_tx_freq(uint32_t freq);
void ad9834_tx_phase(uint16_t Phase);
void ad9834_freq(uint32_t freq, uint32_t refClk);
void bfo_update(uint32_t freq, uint32_t ref);
#endif