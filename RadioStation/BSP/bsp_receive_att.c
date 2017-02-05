#include "bsp_receive_att.h"

// 接收衰减控制数字电位器  PD10/57/CLK   PD11/58/SDI    PD12/59/CS。 分3档 每档0-60
#define RATT_CS   GPIO_Pin_12
#define RATT_SDI  GPIO_Pin_11
#define RATT_CLK  GPIO_Pin_10
#define RATT_PORT GPIOD

void ratt_init()
{
  GPIO_InitTypeDef GPIO_InitStructure;
  
  GPIO_InitStructure.GPIO_Pin =  RATT_CLK | RATT_SDI | RATT_CS;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_Init(RATT_PORT, &GPIO_InitStructure);
}

void ratt_control(uint8_t data)
{
  if(data > 60)
  {
    return;
  }
  GPIO_ResetBits(RATT_PORT, RATT_CLK);  
  GPIO_SetBits(RATT_PORT, RATT_CLK); 
  
  GPIO_ResetBits(RATT_PORT, RATT_CS);

  for(uint8_t i=0; i<8; i++)
  {
    GPIO_ResetBits(RATT_PORT, RATT_CLK); 
    
    if(data & 0x80)
    {
      GPIO_SetBits(RATT_PORT, RATT_SDI);  
    }else
    {
      GPIO_ResetBits(RATT_PORT, RATT_SDI);  
    }
    data <<= 1;
    
   // delay_us(1);
    GPIO_SetBits(RATT_PORT, RATT_CLK); 
   // delay_us(1);
  }
  
  GPIO_SetBits(RATT_PORT, RATT_CS); 
  GPIO_ResetBits(RATT_PORT, RATT_CLK);  
  GPIO_SetBits(RATT_PORT, RATT_CLK); 
  GPIO_ResetBits(RATT_PORT, RATT_CLK);  
}