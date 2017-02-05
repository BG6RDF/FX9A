#ifndef __BSP_ADC_H__
#define __BSP_ADC_H__

#include "bsp_board.h"

void adc_init();
void adc_start();

uint16_t adc_batt();
uint16_t adc_swr();
uint16_t adc_rf();
uint16_t adc_rx();
#endif