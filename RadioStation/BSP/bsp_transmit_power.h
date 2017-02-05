#ifndef __BSP_TRANSMIT_POWER_H__
#define __BSP_TRANSMIT_POWER_H__

#include "bsp_board.h"


void tp_init();
void tx_control_on();
void tx_control_off();
void tp_control(uint8_t data);
void QuietModeOn();
void QuietModeOff();
void MICControlOn();
void MICControlOff();
void CW_FilterControl(uint8_t high);
void CW_BFOControl(uint8_t high);
#endif