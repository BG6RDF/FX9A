#include "bsp_transmit_power.h"

//发射功率控制，分3档，大功率，中功率，小功率（在菜单里分3项：H/M/L   H/0-60  M/0-60  L/0-60) 
//PE14/45/CS   PE15/46/SDI   PB10/47/CLK

#define TP_CS   GPIO_Pin_14
#define TP_SDI  GPIO_Pin_15
#define TP_CLK  GPIO_Pin_10
#define TP_CS_SDI_PORT GPIOE
#define TP_CLK_PORT GPIOB

// PE11/42脚  TX发射控制端（不管是SSB模式还是CW模式，在发射时输出高电平）
#define TX_CONTROL   GPIO_Pin_11
// PB11/48脚    静音控制，在发射时输出高电平
// PB12/51脚    MIC控制，在CW模式下发射时输出低电平，RX时也输出低电平，只有在SSB模式，发射时输出高电平
#define Quiet GPIO_Pin_11
#define MIC GPIO_Pin_12

//PB13/52脚    CW滤波器切换控制，在SSB模式下默认低电平，在CW模式下 RX时默认高电平（N 窄带）。
//         在CM模式 发射时 为低电平（W 宽带）
//PB14/53脚    BFO切换 CW发射状态下，输出高电平; 其他情况全为低电平
#define CW_FIL_CONTROL GPIO_Pin_13
#define CW_BFO_CONTROL GPIO_Pin_14

void tp_init()
{
  GPIO_InitTypeDef GPIO_InitStructure;
  
  GPIO_InitStructure.GPIO_Pin =  TP_CS | TP_SDI | TX_CONTROL;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_Init(TP_CS_SDI_PORT, &GPIO_InitStructure);
  
  GPIO_InitStructure.GPIO_Pin =  TP_CLK | Quiet | MIC | CW_FIL_CONTROL | CW_BFO_CONTROL;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_Init(TP_CLK_PORT, &GPIO_InitStructure);
  
}

void CW_FilterControl(uint8_t high)
{
  if( high )
  {
    GPIO_SetBits(TP_CLK_PORT, CW_FIL_CONTROL);
  }
  else
  {
    GPIO_ResetBits(TP_CLK_PORT, CW_FIL_CONTROL); 
  }
}

void CW_BFOControl(uint8_t high)
{
  if( high )
  {
    GPIO_SetBits(TP_CLK_PORT, CW_BFO_CONTROL);
  }
  else
  {
    GPIO_ResetBits(TP_CLK_PORT, CW_BFO_CONTROL); 
  }
}
void QuietModeOn()
{
  GPIO_SetBits(TP_CLK_PORT, Quiet); 
}

void QuietModeOff()
{
  GPIO_ResetBits(TP_CLK_PORT, Quiet); 
}

void MICControlOn()
{
  GPIO_SetBits(TP_CLK_PORT, MIC); 
}

void MICControlOff()
{
  GPIO_ResetBits(TP_CLK_PORT, MIC); 
}

void tx_control_on()
{
  GPIO_SetBits(TP_CS_SDI_PORT, TX_CONTROL); 
}

void tx_control_off()
{
  GPIO_ResetBits(TP_CS_SDI_PORT, TX_CONTROL); 
}

void tp_control(uint8_t data)
{
  GPIO_ResetBits(TP_CLK_PORT, TP_CLK);  
  GPIO_SetBits(TP_CLK_PORT, TP_CLK); 
  
  GPIO_ResetBits(TP_CS_SDI_PORT, TP_CS);

  for(uint8_t i=0; i<8; i++)
  {
    GPIO_ResetBits(TP_CLK_PORT, TP_CLK); 
    
    if(data & 0x80)
    {
      GPIO_SetBits(TP_CS_SDI_PORT, TP_SDI);  
    }else
    {
      GPIO_ResetBits(TP_CS_SDI_PORT, TP_SDI);  
    }
    data <<= 1;
    
   // delay_us(1);
    GPIO_SetBits(TP_CLK_PORT, TP_CLK); 
   // delay_us(1);
  }
  
  GPIO_SetBits(TP_CS_SDI_PORT, TP_CS); 
  GPIO_ResetBits(TP_CLK_PORT, TP_CLK);  
  GPIO_SetBits(TP_CLK_PORT, TP_CLK); 
  GPIO_ResetBits(TP_CLK_PORT, TP_CLK);  
}
