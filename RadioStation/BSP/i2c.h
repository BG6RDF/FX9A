#ifndef I2C_H
#define I2C_H

#include "bsp_board.h"
#include <stdint.h>


#define st(x)      do { x } while (__LINE__ == -1)

#define ACK	0
#define NOACK   1
#define OUTPUT  1
#define INPUT   0


// ½Ó24C16´¢´æÆ÷ PE2-1-7-WC   PE3-2-6-SCL   PE4-3-5-SDA
#define I2C_SDA GPIO_Pin_4	
#define I2C_SCK GPIO_Pin_3	
#define E2P_WC  GPIO_Pin_2
#define I2C_PORT GPIOE
/*********************************************
*	Global function
**/
void i2c_init(void);
void i2c_start();
void i2c_stop();
void i2c_tx(uint8_t wdata);
uint8_t i2c_rx();
void i2c_ack();
void i2c_noack();
uint8_t i2c_getack();
void SET_E2P_MODE(uint8_t bit);

void i2c_reset();
#endif