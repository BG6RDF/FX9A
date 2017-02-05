#include "bsp_ad9834.h"

// DDS AD9834控制端  PE7/38/SDATA  PE8/39/SCLK   PE9/40/FSYNC
#define AD9834_FSYNC GPIO_Pin_9
#define AD9834_SCLK GPIO_Pin_8
#define AD9834_SDATA GPIO_Pin_7
#define AD9834_PORT GPIOE


#define FCLK 50 

void ad9834_write(uint16_t data);

void ad9834_init()
{
  GPIO_InitTypeDef GPIO_InitStructure;
  
  GPIO_InitStructure.GPIO_Pin =  AD9834_FSYNC | AD9834_SCLK | AD9834_SDATA;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_Init(AD9834_PORT, &GPIO_InitStructure);
  
  GPIO_SetBits(AD9834_PORT, AD9834_FSYNC); 
  GPIO_SetBits(AD9834_PORT, AD9834_SCLK); 

  ad9834_write(0x2100);
  ad9834_write(0x2308); // 一次性写FREQREG0
}

void ad9834_write(uint16_t data)
{ 
  GPIO_ResetBits(AD9834_PORT, AD9834_FSYNC);

  for(uint8_t i=0; i<16; i++)
  {
    if(data & 0x8000)
    {
      GPIO_SetBits(AD9834_PORT, AD9834_SDATA);  
    }else
    {
      GPIO_ResetBits(AD9834_PORT, AD9834_SDATA);  
    }
    data <<= 1;
    delay_us(5);
    GPIO_ResetBits(AD9834_PORT, AD9834_SCLK);
    delay_us(5);
    GPIO_SetBits(AD9834_PORT, AD9834_SCLK); 
    delay_us(5);
  }
  
  GPIO_SetBits(AD9834_PORT, AD9834_FSYNC); 
}

void ad9834_tx_freq(uint32_t freq)
{ 
  uint32_t temp; 
  freq=freq*16384/(FCLK*1000000)*16384; 

  temp=freq;    
  ad9834_write(temp/0x4000+0x4000); 
  ad9834_write(temp%0x4000+0x4000); 
} 

void ad9834_freq(uint32_t freq, uint32_t refClk)
{ 
  unsigned long temp; 
  temp = (unsigned long)(268435456.0 / refClk * freq);
  //temp = freq*4294967296/16/refClk; //8053063680000000 161061273.6
  
  ad9834_write(temp%0x4000+0x4000); 
  ad9834_write(temp/0x4000+0x4000); 
} 

uint32_t ad9834_freq_old=0;
static uint32_t clk_ref_old1 = 0;
void bfo_update(uint32_t freq, uint32_t ref)
{
  if((freq != ad9834_freq_old) || (clk_ref_old1 != ref))
  {
    ad9834_freq(freq, ref);
    ad9834_freq_old = freq;
    clk_ref_old1 = ref;
  }
}

//发送相位
void ad9834_tx_phase(uint16_t Phase) 
{ 
  Phase %= 360;
  Phase=(uint16_t)((float)Phase*2048/3.1415926+0.5);
  ad9834_write(0xC000+Phase);
}