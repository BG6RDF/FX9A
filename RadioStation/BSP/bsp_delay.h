#ifndef __BSP_DELAY_H__
#define __BSP_DELAY_H__

#include "bsp_board.h"

void delay_init(uint8_t clk); //延时函数初始化
void delay_us(uint32_t nus);  //us级延时函数,最大65536us.
void delay_ms(uint32_t nms);  //ms级延时函数

#endif