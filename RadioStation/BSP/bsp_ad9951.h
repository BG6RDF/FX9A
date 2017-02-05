#ifndef __BSP_AD9951_H__
#define __BSP_AD9951_H__

#include "bsp_board.h"

#define CFR1_REG_ADDR 0x00  // Control Function Register No.1
#define CFR2_REG_ADDR 0x01  
#define ASF_REG_ADDR 0x02   // Amplitude Scale Factor
#define ARR_REG_ADDR 0x03   // Amplitude Ramp Rate
#define FTW0_REG_ADDR 0x04  // Frequency Tuning Word
#define FOW0_REG_ADDR 0x05  // Phase Offset Word


void ad9951_init();
void lo_update(uint32_t freq, uint32_t ref);
#endif