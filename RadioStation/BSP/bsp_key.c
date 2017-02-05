#include "bsp_key.h"
#include "bsp_timer.h"
#include "bsp_adc.h"
#include "bsp_oled_api.h"

// 老版本
//开关        PD6/87/K1  PD5/86/K2  PD7/88/K3  PD4/85/K4  PD0/81/K5  PD1/82/K6  
//开关        PC12/80/K7  PC11/79/K8  PC10/78/K9  PA15/77/K10 
//编码器      PB4/90/K1-A  PB3/89/K1-B  PA12/71/K9-A  PA11/70/K9-B  PA10/69/K10-A  PA9/68/K10-B
//光电编码器  PD2/83/R1-A  PD3/84/R1-B

// 新版本
//开关        PB4/90/K1   PD6/87/K3 
//编码器      PA12/71/K1-A  PA11/70/K1-B
//声音编码器  PA10/69/K2-A  PA9/68/K2-B   -> 后期 改到 57/PD10 58/PD11脚
//光电编码器  PD2/83/R1-A  PD3/84/R1-B
//PE12/43/KEY1 是点控制。PE13/44/KEY2是划控制
//PB0/35脚    PTT开关检测端（此脚使用电阻上拉到3.3V，在SSB模式下，按下话筒开关，开 
//        关把此脚对地短路，把35脚电平拉低。单片机控制相应的电路发射，此脚只在SSB模式下起作用）
#define PTT GPIO_Pin_0
#define DIT GPIO_Pin_12
#define DAH GPIO_Pin_13

#define K1 GPIO_Pin_4
#define K2 GPIO_Pin_5
#define K3 GPIO_Pin_6
#define K4 GPIO_Pin_4
#define K5 GPIO_Pin_0
#define K6 GPIO_Pin_1
#define K7 GPIO_Pin_12
#define K8 GPIO_Pin_11

#define K1A GPIO_Pin_12
#define K1B GPIO_Pin_11

#define K2A GPIO_Pin_10
#define K2B GPIO_Pin_11

#define R1A GPIO_Pin_2
#define R1B GPIO_Pin_3

#define KEY_1_PORT GPIOB
#define KEY_23456R1A1BK2A2B_PORT GPIOD
#define KEY_78_PORT GPIOC
#define KEY_K1A1B_PORT GPIOA
#define PADDLE_PORT GPIOE

static KEY_AttrDef key;
bool key_availlble = false;
uint8_t KVal = 0;
uint8_t K9_A = 0;
uint8_t K9_B = 0;

uint8_t kVal    = 0xff; //按键标识，键值
uint16_t kTick   = 0;    //去抖时间kTick * 20毫秒
bool scan_state = true;
uint8_t tick1s = 0;
uint8_t ditUp = 0;
uint8_t ditDown = 0;
uint8_t dahUp = 0;
uint8_t dahDown = 0;
uint8_t pttUp = 0;
uint8_t pttDown = 0;

bool DitPaddle;   // DIT键是否被按下
bool DahPaddle;   // DAH键是否被按下
bool PaddleUpdate = false;
bool PttPressed;  // PTT控制键是否被按下
bool PttFinish;   // PTT按下操作是否被执行
void KEY_EXTI_Config()
{
  EXTI_InitTypeDef EXTI_InitStructure;
  //清空中断标志
  EXTI_ClearITPendingBit(EXTI_Line2);
  EXTI_ClearITPendingBit(EXTI_Line10);
  EXTI_ClearITPendingBit(EXTI_Line11);
  
  //选择中断管脚
  GPIO_EXTILineConfig(GPIO_PortSourceGPIOD, GPIO_PinSource2);
  GPIO_EXTILineConfig(GPIO_PortSourceGPIOD, GPIO_PinSource10);
  GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource11);
  
  EXTI_InitStructure.EXTI_Line = EXTI_Line2; //选择中断线路
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt; //设置为中断请求，非事件请求
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling; //设置中断触发方式为上下降沿触发
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;                                          //外部中断使能
  EXTI_Init(&EXTI_InitStructure);
  
  EXTI_InitStructure.EXTI_Line = EXTI_Line10; //选择中断线路
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt; //设置为中断请求，非事件请求
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling; //设置中断触发方式为上下降沿触发
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;                                          //外部中断使能
  EXTI_Init(&EXTI_InitStructure);
  
  EXTI_InitStructure.EXTI_Line = EXTI_Line11; //选择中断线路
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt; //设置为中断请求，非事件请求
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling; //设置中断触发方式为上下降沿触发
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;                                          //外部中断使能
  EXTI_Init(&EXTI_InitStructure);
}

void GPIO_NVIC_Config()
{
  NVIC_InitTypeDef NVIC_InitStructure;  
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); 
  
  NVIC_InitStructure.NVIC_IRQChannel = EXTI2_IRQn; 
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0; 
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;  
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;  
  NVIC_Init(&NVIC_InitStructure);
  
  NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn; 
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0; 
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;  
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;  
  NVIC_Init(&NVIC_InitStructure);
  
}
void key_init()
{
  GPIO_InitTypeDef GPIO_InitStructure;
  TMR_Init(TIM2, 200, 2000, key_calback);        //预分频数为TIMXCLK/(50*2000)，每秒产生50个中断
  TMR_IRQEnable(TIM2);
  
  GPIO_InitStructure.GPIO_Pin =  K1;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_Init(KEY_1_PORT, &GPIO_InitStructure);
  GPIO_ResetBits(KEY_1_PORT, K1);
  
  GPIO_InitStructure.GPIO_Pin =  PTT;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_Init(KEY_1_PORT, &GPIO_InitStructure);
  
  GPIO_InitStructure.GPIO_Pin =  K2 | K3 | K4 | K5 | K6 | R1A | R1B | K2A | K2B;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_Init(KEY_23456R1A1BK2A2B_PORT, &GPIO_InitStructure);
  
  GPIO_InitStructure.GPIO_Pin =  K7 | K8;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_Init(KEY_78_PORT, &GPIO_InitStructure);
  
  GPIO_InitStructure.GPIO_Pin =  K1A | K1B;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_Init(KEY_K1A1B_PORT, &GPIO_InitStructure);
  
  GPIO_InitStructure.GPIO_Pin =  DIT | DAH;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_Init(PADDLE_PORT, &GPIO_InitStructure);
  KEY_EXTI_Config();
  GPIO_NVIC_Config();
  
  // CW模式 ：单键 或 双键判断
  if (GPIO_ReadInputDataBit(PADDLE_PORT, DIT) == 0)
  {
    MENU.cwMode = MANUAL;
  }
  else
  {
    MENU.cwMode = AUTO;
  }
}

uint8_t key_read()
{
  uint16_t port;
  
  port = GPIO_ReadInputData(KEY_1_PORT);
  if( !(port & K1) )
  {
    return 1;
  }
  
  port = GPIO_ReadInputData(KEY_23456R1A1BK2A2B_PORT);
  if ( !(port & K2) )
  {
    return 2; 
  }
  
  if( !(port & K3) )
  {
    return 3;
  }
  
  if( !(port & K4) )
  {
    return 4;
  }
  
  if( !(port & K5) )
  {
    return 5;
  }
  
  if( !(port & K6) )
  {
    return 6;
  }
  
  port = GPIO_ReadInputData(KEY_78_PORT);
  if( !(port & K7) )
  {
    return 7;
  }
  
  if( !(port & K8) )
  {
    return 8;
  }
  uint8_t ptt = GPIO_ReadInputDataBit(KEY_1_PORT, PTT);
  if(ptt && PttPressed)         //以前是按下的但现在松开了
  {
    if(pttDown > 0)
    {
      pttDown = 0;
    }
    pttUp ++;
    if(pttUp >= 1)              //松开防抖，原值1      BG6RDF
    {
      pttUp = 0;
      PttPressed = false;

    }
  }
  else if(!ptt && !PttPressed ) //以前是松开的但现在按下了
  {
    if(pttUp > 0)
    {
      pttUp = 0;
    }
    pttDown ++;
    if(pttDown >= 1)            //按下防抖，原值1      BG6RDF
    {
      pttDown = 0;
      PttPressed = true;
    }
  }
  
  tick1s ++;
  if(tick1s % 40 == 0)          //原值10      BG6RDF
  {
    if( RF_IsRx() )
    {
      DRMgr.item |= RXCHKItem;
    }
    else
    {
      DRMgr.item |= SWRCHKItem;
      DRMgr.item |= RFCHKItem;
    }
  }
  if(tick1s == 200)             //原值50      BG6RDF
  {
    tick1s = 0;
    DRMgr.item |= DCItem;
  }
  
  port = GPIO_ReadInputData(PADDLE_PORT);

  if(port & DIT)
  {
    if(ditDown > 0)
    {
      ditDown = 0;
    }
    ditUp ++;
    if(ditUp >= 1)      //点松开防抖，原值1      BG6RDF
    {
      ditUp = 0;
      DitPaddle = false;
    }
  }
  else
  {
    if(ditUp > 0)
    {
      ditUp = 0;
    }
    ditDown ++;
    if(ditDown >= 1)    //点按下防抖，原值1      BG6RDF
    {
      ditDown = 0;
      DitPaddle = true;
    }
  }
  if(port & DAH)
  {
    if(dahDown > 0)
    {
      dahDown = 0;
    }
    dahUp ++;
    if(dahUp >= 1)      //划松开防抖，原值1      BG6RDF
    {
      dahUp = 0;
      DahPaddle = false;
    }
  }
  else
  {
    if(dahUp > 0)
    {
      dahUp = 0;
    }
    dahDown ++;
    if(dahDown >= 1)    //划按下防抖，原值1      BG6RDF
    {
      dahDown = 0;
      DahPaddle = true;
    }
  }
  
  PaddleUpdate = true;  //用于CW模式的 Paddle更新
  
  return 0xff;
}

void key_readkeyer()
{
  uint16_t port;
  
  port = GPIO_ReadInputData(PADDLE_PORT);
  if(port & DIT)
  {
    DitPaddle = false;
  }
  else
  {
    DitPaddle = true;
  }
  if(port & DAH)
  {
    DahPaddle = false;
  }
  else
  {
    DahPaddle = true;
  }
//  if(port & DIT)
//  {
//    if(ditDown > 0)
//    {
//      ditDown = 0;
//    }
//    ditUp ++;
//    if(ditUp >= 1)
//    {
//      ditUp = 0;
//      DitPaddle = false;
//    }
//  }
//  else
//  {
//    if(ditUp > 0)
//    {
//      ditUp = 0;
//    }
//    ditDown ++;
//    if(ditDown >= 1)
//    {
//      ditDown = 0;
//      DitPaddle = true;
//    }
//  }
//  if(port & DAH)
//  {
//    if(dahDown > 0)
//    {
//      dahDown = 0;
//    }
//    dahUp ++;
//    if(dahUp >= 1)
//    {
//      dahUp = 0;
//      DahPaddle = false;
//    }
//  }
//  else
//  {
//    if(dahUp > 0)
//    {
//      dahUp = 0;
//    }
//    dahDown ++;
//    if(dahDown >= 1)
//    {
//      dahDown = 0;
//      DahPaddle = true;
//    }
//  }
  
  PaddleUpdate = true;  //用于CW模式的 Paddle更新
}

bool key_ispress(KEY_AttrDef *rkey)
{
  *rkey = key;
  return key_availlble; 
}

void key_clear()
{
  key_availlble = false;
}

uint8_t upCnt = 0;
void key_calback( void  )
{
  uint8_t kTmp = key_read();
  
  if(kVal == 0xff)      //之前状态无按键
  {
    kVal = kTmp;
    key.state = keyNull;
    kTick = 0;
    return;
  }
  
  if(kTmp != kVal)      //和前次按键不同
  {
    upCnt ++;
    if (upCnt >= 8)     //防抖    原值是2    BG6RDF
    {
      upCnt = 0;
      if(kTmp == 0xff)    //短按弹起
      {
        if( key.state == keyShot)
        {
          key_availlble = true; 
        } 
      }
      kVal = kTmp;        //保存新按键值
      kTick = 0;
      return;
    }
  }
  else
  {
    kTick ++;
    if( upCnt > 0 )
    {
      upCnt = 0;
    }
    if(kTick == 16)          //20ms * 4 = 80ms，原值是4   BG6RDF
    {
      key.state = keyShot;
      key.val = kVal;
    }  
    
    if(kTick == 200)      //按住2秒后，原值50，为啥是2秒 20mS*50=1S?  BG6RDF
    {
      key.val = kVal;
      key.state = keyLong;
      key_availlble = true; 
    }
  }
}

void KEY_SoftwareSet(uint8_t kval, KEY_State state)
{
  if( key_availlble == false)
  {
    key.val = kval;
    key.state = state;
    key_availlble = true; 
  }
}

// 主编码器，没有防抖
// R1 - line_2
void EXTI2_IRQHandler(void)
{
  if (EXTI_GetITStatus(EXTI_Line2) == SET)
  {
    // 锁定状态
    if ( ATT.val )
    {
      EXTI_ClearITPendingBit(EXTI_Line2);
      return;
    }
    
    if (GPIO_ReadInputDataBit(KEY_23456R1A1BK2A2B_PORT, R1B) == 0) // A-下降沿 B-高电平 为正
    {
      // VFO和MEM模式下，调频旋钮都可用，但在MEM模式下 不记忆
      //if (AIMgr.item > 0)
      {     
        if (FREQ.val >= BAUD[ FREQ.numOfbaud ][2])
        {
          FREQ.val = BAUD[ FREQ.numOfbaud ][2];
          if (FREQ.item & TSItem)
          {
            FREQ.val /= 100;
            FREQ.val *= 100;
          }        
        }
        else
        {
          if(AIMgr.item & MENUSETItem)
          {
            MENU_SetUp(MENU.numOfmenu);
          }
          else if ((FREQ.item & RITItem) && !(AIMgr.item))
          {
            if (VFO.RIT >= UPPER_OF_RIT)
            {
              VFO.RIT = UPPER_OF_RIT;
            }
            else
            {              
              if (FREQ.item & TSItem)
              {
                if ((VFO.RIT + 100) < UPPER_OF_RIT)
                {
                  VFO.RIT += 100;
                }
              }
              else
              {
                VFO.RIT ++;
              }
            }
          }
          else if ((FREQ.item & XITItem) && !(AIMgr.item))      //XIT BG6RDF
          {
            if (VFO.XIT >= UPPER_OF_XIT)
            {
              VFO.XIT = UPPER_OF_XIT;
            }
            else
            {              
              if (FREQ.item & TSItem)
              {
                if ((VFO.XIT + 100) < UPPER_OF_XIT)
                {
                  VFO.XIT += 100;
                }
              }
              else
              {
                VFO.XIT ++;
              }
            }
          }
          else
          {
            if (FREQ.item & TSItem)
            {
              FREQ.val += 100;
            }
            else
            {
              FREQ.val ++;
            }
          }
        }
        if (AIMgr.item & MENUSETItem)
        {
          DRMgr.item |= MENUSETItem;
        }
        else if (AIMgr.item & MEMSETItem)
        {
          DRMgr.item |= MEMSETItem;
        }
        else
        {
          DRMgr.item |= FREQItem;
        }
      }
    }
    else
    {
      //if (AIMgr.item > 0)
      {
        if (FREQ.val <= BAUD[ FREQ.numOfbaud ][0])
        {
          FREQ.val = BAUD[ FREQ.numOfbaud ][0];
          if (FREQ.item & TSItem)
          {
            FREQ.val /= 100;
            FREQ.val *= 100;
          }
        }
        else
        {
          if(AIMgr.item & MENUSETItem)
          {
            MENU_SetDown(MENU.numOfmenu);
          }
          else if ((FREQ.item & RITItem) && !(AIMgr.item))  // RIT选中时，且在非设置状态下 起作用
          {
            if (VFO.RIT <= LOWER_OF_RIT)
            {
              VFO.RIT = LOWER_OF_RIT;
            }
            else
            {
              if (FREQ.item & TSItem)
              {
                if((VFO.RIT - 100) >= LOWER_OF_RIT)
                {
                  VFO.RIT -= 100;
                }
              }
              else
              {
                VFO.RIT --;
              }
            }
          }
          else if ((FREQ.item & XITItem) && !(AIMgr.item)) //XIT BG6RDF
          {
             if (VFO.XIT <= LOWER_OF_XIT)
            {
              VFO.XIT = LOWER_OF_XIT;
            }
            else
            {
              if (FREQ.item & TSItem)
              {
                if((VFO.XIT - 100) >= LOWER_OF_XIT)
                {
                  VFO.XIT -= 100;
                }
              }
              else
              {
                VFO.XIT --;
              }
            }
          }
          else
          {
            if (FREQ.item & TSItem)
            {
              if( FREQ.val >= 100)
              {
                FREQ.val -= 100;
              }
            }
            else
            {
              FREQ.val --;
            }
          }
        }
        
        if (AIMgr.item & MENUSETItem)
        {
          DRMgr.item |= MENUSETItem;
        }
        else if (AIMgr.item & MEMSETItem)
        {
          DRMgr.item |= MEMSETItem;
        }
        else
        {
          DRMgr.item |= FREQItem;
        }
      }
    }
    EXTI_ClearITPendingBit(EXTI_Line2);
  }
}

//#pragma directive only affects the function that follows immediately after the directive
#pragma optimize=none
void IntDelay()
{
  for(int i=0; i<20000; i++);
}

// 小编码器无防抖
// K2A - line 10
// K1B - line_11
uint8_t k1ForwardFlag = false;
uint8_t k1BackwardFlag= false;
uint8_t k2ForwardFlag = false;
uint8_t k2BackwardFlag= false;
void EXTI15_10_IRQHandler(void)
{ 
  if (EXTI_GetITStatus(EXTI_Line10) == SET)             //K2A
  {
    if ((AIMgr.item & MENUSETItem) || RF_IsTx())
    {
    }
    else if(GPIO_ReadInputDataBit(KEY_23456R1A1BK2A2B_PORT, K2A) == 0)                          //下降沿
    {
      if (GPIO_ReadInputDataBit(KEY_23456R1A1BK2A2B_PORT, K2B) == 0)
        k2ForwardFlag=true;
      else 
        k2BackwardFlag=true;
      IntDelay();
    }
    else if (k2ForwardFlag && GPIO_ReadInputDataBit(KEY_23456R1A1BK2A2B_PORT, K2B) == 1)        //上升沿，正转
    {
      k2ForwardFlag=false;
      if (AIMgr.item == WPMSETItem)
      {
        if (WPM.val >= 60)
        {
          WPM.val = 60;
        }
        else
        {
          WPM.val ++;
        }
        DRMgr.item |= WPMSETItem;
      }
      else if (AIMgr.item & MEMSETItem)         //信道模式下，K2用于选择波段
      {
        if (FREQ.numOfbaud >= SIZE_OF_BAUD-1)
        {
          FREQ.numOfbaud = 0;
        }
        else
        {
          FREQ.numOfbaud ++; 
        }
        FREQ.val=BAUD[FREQ.numOfbaud][1];
        DRMgr.item |= MEMSETItem;
      }
      else 
      {
        if(VOL.val >= 60)
        {
          VOL.val = 60;
        }else{
          VOL.val ++;
        }
        DRMgr.item |= VOLItem;
      }
    }
    else if (k2BackwardFlag && GPIO_ReadInputDataBit(KEY_23456R1A1BK2A2B_PORT, K2B) == 0)        //上升沿，反转
    {
      k2BackwardFlag=false;
      if (AIMgr.item == WPMSETItem)
      {
        if (WPM.val <= 4)
        {
          WPM.val = 4;
        }
        else
        {
          WPM.val --;
        }
        DRMgr.item |= WPMSETItem;
      }
      else if (AIMgr.item & MEMSETItem)         //信道模式下，K2用于选择波段
      {
        if (FREQ.numOfbaud == 0)
        {
          FREQ.numOfbaud = SIZE_OF_BAUD-1;
        }
        else
        {
          FREQ.numOfbaud --; 
        }
        FREQ.val=BAUD[FREQ.numOfbaud][1];
        DRMgr.item |= MEMSETItem;
      }
      else 
      {
        if(VOL.val <= 0)
        {
          VOL.val = 0;
        }else{
          VOL.val --;
        }
        DRMgr.item |= VOLItem;
      }
    }
    EXTI_ClearITPendingBit(EXTI_Line10);
  }
  
  if (EXTI_GetITStatus(EXTI_Line11) == SET)             //K1B
  {
    if (RF_IsTx())
    {
      
    }
    else if (!(AIMgr.item & MENUSETItem) && !(FREQ.item & MEMItem) && !(FREQ.item & VFOItem))
    {
    }
    else if(GPIO_ReadInputDataBit(KEY_K1A1B_PORT, K1B) == 0)                          //下降沿
    {
      if (GPIO_ReadInputDataBit(KEY_K1A1B_PORT, K1A) == 0)
         k1ForwardFlag=true;
      else 
         k1BackwardFlag=true;
      IntDelay();
    }
    else if (k1ForwardFlag && GPIO_ReadInputDataBit(KEY_K1A1B_PORT, K1A) == 1)        //上升沿，正转
    {
      k1ForwardFlag=false;
      if(AIMgr.item == MENUSETItem)
      {
        if (MENU.numOfmenu >= SIZE_OF_MENU-1)
        {
          MENU.numOfmenu = SIZE_OF_MENU-1;
        }
        else
        {
          MENU.numOfmenu ++; 
        }
        DRMgr.item |= MENUSETItem; 
      }
      else if (AIMgr.item == MEMSETItem || FREQ.item & MEMItem)
      {
        if (FREQ.numOfmem >= SIZE_OF_MEM-1)
        {
          FREQ.numOfmem = SIZE_OF_MEM-1;
        }
        else
        {
          FREQ.numOfmem ++; 
        }
        //FREQ.val =MEM[FREQ.numOfmem].freq;
        MEM_Get();
        if(AIMgr.item == MEMSETItem)
        {
          DRMgr.item |= MEMSETItem; 
          DRMgr.item |= RADIOItem;
        }
        else
        {
          DRMgr.item |= FREQItem;
          DRMgr.item |= RADIOItem;
        }
        DRMgr.item |= FILItem;
      }
      else if (FREQ.item & VFOItem)
      {
        if (FREQ.numOfbaud >= SIZE_OF_BAUD-1)
        {
          FREQ.numOfbaud = 0;
        }
        else
        {
          FREQ.numOfbaud ++; 
        }
        VFO_Get();
        DRMgr.item |= FREQItem;
        DRMgr.item |= RADIOItem; 
        DRMgr.item |= FILItem;
      }
    }
    else if (k1BackwardFlag && GPIO_ReadInputDataBit(KEY_K1A1B_PORT, K1A) == 0)        //上升沿，反转
    {
      k1BackwardFlag=false;
      if(AIMgr.item == MENUSETItem)
      {
        if (MENU.numOfmenu <= 0)
        {
          MENU.numOfmenu = 0;
        }
        else
        {
          MENU.numOfmenu --; 
        }
        DRMgr.item |= MENUSETItem; 
      }
      else if (AIMgr.item == MEMSETItem || FREQ.item & MEMItem)
      {
        if (FREQ.numOfmem <= 0)
        {
          FREQ.numOfmem = 0;
        }
        else
        {
          FREQ.numOfmem --; 
        }
        //FREQ.val = MEM[FREQ.numOfmem].freq;
        MEM_Get();
        if(AIMgr.item == MEMSETItem)
        {
          DRMgr.item |= MEMSETItem;  
          DRMgr.item |= RADIOItem;
        }
        else
        {
          DRMgr.item |= FREQItem;
          DRMgr.item |= RADIOItem;
        }
        DRMgr.item |= FILItem;
      }
      else if (FREQ.item & VFOItem)
      {
        if (FREQ.numOfbaud == 0)
        {
          FREQ.numOfbaud = SIZE_OF_BAUD-1;
        }
        else
        {
          FREQ.numOfbaud --; 
        }
        VFO_Get();
        DRMgr.item |= FREQItem;
        DRMgr.item |= RADIOItem; 
        DRMgr.item |= FILItem;
      }
    }
    EXTI_ClearITPendingBit(EXTI_Line11);
  }
}