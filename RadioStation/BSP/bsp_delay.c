#include "bsp_delay.h"

volatile uint8_t fac_us=0; //us延时倍乘数   

//延时函数初始化
//为确保准确度,请保证时钟频率最好为4的倍数,最低8Mhz
//clk:时钟频率(24/16/12/8等) 
void delay_init(uint8_t clk)
{
  if(clk>16)
  {
    fac_us=(16-4)/4;//24Mhz时,stm8大概19个周期为1us
  }
  else
  {
    if(clk>4)
    {
      fac_us=(clk-4)/4;
    }
    else
    {
    fac_us=1;
    }
  }
}


//延时nus
//延时时间=(fac_us*4+4)*nus*(T)
//其中,T为CPU运行频率(Mhz)的倒数,单位为us.
//准确度:
//92%  @24Mhz
//98%  @16Mhz
//98%  @12Mhz
//86%  @8Mhz
void delay_us(uint32_t nus)
{  
  
  while(nus--)
  {
    for(uint16_t i=0; i<72; i++)
    {
      __asm("nop");
    }
  }
//  __asm(
//  "PUSH A          \n"  //1T,压栈
//  "DELAY_XUS:      \n"   
//  "LD A,fac_us     \n"   //1T,fac_us加载到累加器A
//  "DELAY_US_1:     \n"  
//  "NOP             \n"  //1T,nop延时
//  "DEC A           \n"  //1T,A--
//  "JRNE DELAY_US_1 \n"   //不等于0,则跳转(2T)到DELAY_US_1继续执行,若等于0,则不跳转(1T).
//  "NOP             \n"  //1T,nop延时
//  "DECW X          \n"  //1T,x--
//  "JRNE DELAY_XUS  \n"    //不等于0,则跳转(2T)到DELAY_XUS继续执行,若等于0,则不跳转(1T).
//  "POP A           \n"  //1T,出栈
//  );
} 
//延时nms  
//为保证准确度,nms不要大于16640.
void delay_ms(uint32_t nms)
{
  delay_us(nms*1000);
}