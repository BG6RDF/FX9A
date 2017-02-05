#ifndef __BSP_PWM_H__
#define __BSP_PWM_H__

#include "bsp_board.h"

#define BSP_PWM_PERIOD_VAL 256
void pwm_init();

void buzzer_freq_set(uint32_t hz);
void buzzer_start();
void sin_para_get();

void buzzer_stop();
#endif