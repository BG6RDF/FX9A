#include "bsp_vol.h"

//数字音量电位器控制接口 PA6/31/CLK  PA7/32/SDI  PC4/33/CS
#define VOL_CS   GPIO_Pin_4
#define VOL_SDI  GPIO_Pin_7
#define VOL_CLK  GPIO_Pin_6
#define VOL_CLK_SDI_PORT GPIOA
#define VOL_CS_PORT GPIOC

void vol_init()
{
  GPIO_InitTypeDef GPIO_InitStructure;
  
  GPIO_InitStructure.GPIO_Pin =  VOL_CLK | VOL_SDI;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_Init(VOL_CLK_SDI_PORT, &GPIO_InitStructure);
  
  GPIO_InitStructure.GPIO_Pin =  VOL_CS;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_Init(VOL_CS_PORT, &GPIO_InitStructure);
}

void vol_control(uint8_t data)
{
  uint8_t wdat = 0;
  if(data > 60)
  {
    return;
  }
  
  wdat = data * 4;
  if (data == 60) {
    wdat = 255;
  }
  
  GPIO_ResetBits(VOL_CLK_SDI_PORT, VOL_CLK);  
  GPIO_SetBits(VOL_CLK_SDI_PORT, VOL_CLK); 
  
  GPIO_ResetBits(VOL_CS_PORT, VOL_CS);

  for(uint8_t i=0; i<8; i++)
  {
    GPIO_ResetBits(VOL_CLK_SDI_PORT, VOL_CLK); 
    
    if(wdat & 0x80)
    {
      GPIO_SetBits(VOL_CLK_SDI_PORT, VOL_SDI);  
    }else
    {
      GPIO_ResetBits(VOL_CLK_SDI_PORT, VOL_SDI);  
    }
    wdat <<= 1;
    
    delay_us(1);
    GPIO_SetBits(VOL_CLK_SDI_PORT, VOL_CLK); 
    delay_us(1);
  }
  
  GPIO_SetBits(VOL_CS_PORT, VOL_CS); 
  GPIO_ResetBits(VOL_CLK_SDI_PORT, VOL_CLK);  
  GPIO_SetBits(VOL_CLK_SDI_PORT, VOL_CLK); 
  GPIO_ResetBits(VOL_CLK_SDI_PORT, VOL_CLK);  
}