#include "bsp_iambic_keyer.h"
#include "bsp_timer.h"
#include "bsp_pwm.h"
#include "bsp_oled_api.h"
#include "bsp_key.h"

/*
0  mcode_tab[0x0201] = "A";  // 01         
1  mcode_tab[0x0408] = "B";  // 1000
2  mcode_tab[0x040A] = "C";  // 1010
3  mcode_tab[0x0304] = "D";  // 100
4  mcode_tab[0x0100] = "E";  // 0
5  mcode_tab[0x0402] = "F";  // 0010
6  mcode_tab[0x0306] = "G";  // 110
7  mcode_tab[0x0400] = "H";  // 0000
8  mcode_tab[0x0200] = "I";  // 00
9  mcode_tab[0x0407] = "J";  // 0111
10  mcode_tab[0x0305] = "K";  // 101
11  mcode_tab[0x0404] = "L";  // 0100
12  mcode_tab[0x0203] = "M";  // 11
13  mcode_tab[0x0202] = "N";  // 10
14  mcode_tab[0x0307] = "O";  // 111
15  mcode_tab[0x0406] = "P";  // 0110
16  mcode_tab[0x040D] = "Q";  // 1101
17  mcode_tab[0x0302] = "R";  // 010
18  mcode_tab[0x0300] = "S";  // 000
19  mcode_tab[0x0101] = "T";  // 1 
20  mcode_tab[0x0301] = "U";  // 001
21  mcode_tab[0x0401] = "V";  // 0001 
22  mcode_tab[0x0303] = "W";  // 011 
23  mcode_tab[0x0409] = "X";  // 1001 
24  mcode_tab[0x040B] = "Y";  // 1011 
25  mcode_tab[0x040C] = "Z";  // 1100 
  
  mcode_tab[0x050F] = "1";  // 01111 
  mcode_tab[0x0507] = "2";  // 00111
  mcode_tab[0x0503] = "3";  // 00011 
  mcode_tab[0x0501] = "4";  // 00001 
  mcode_tab[0x0500] = "5";  // 00000 
  mcode_tab[0x0510] = "6";  // 10000 
  mcode_tab[0x0518] = "7";  // 11000 
  mcode_tab[0x051C] = "8";  // 11100 
  mcode_tab[0x051E] = "9";  // 11110
  mcode_tab[0x051F] = "0";  // 11111 
  
  mcode_tab[0x0511] = "=";  // 10001
  mcode_tab[0x0615] = ".";  // 010101
  mcode_tab[0x0605] = "SK"; // 000101
  mcode_tab[0x0516] = "KN"; // 10110
  mcode_tab[0x0512] = "/";  // 10010
  mcode_tab[0x060C] = "?";  // 001100
*/

uint16_t CallCoder[] = {0x0201, 0x0408, 0x040A, 0x0304, 0x0100, 0x0402, 0x0306, 0x0400, 0x0200, 0x0407, 0x0305, 0x0404,    // A - L
0x0203, 0x0202, 0x0307, 0x0406, 0x040D, 0x0302, 0x0300, 0x0101, 0x0301, 0x0401, 0x0303, 0x0409, 0x040B, 0x040C, 0x0512,    // M - Z /
0x051F, 0x051E, 0x051C, 0x0518, 0x0510, 0x0500, 0x0501, 0x0503, 0x0507, 0x050F }; // 0 9 - 1

typedef enum
{
  TIMER_STATE_IDLE1, 
  TIMER_STATE_IDLE2, 
  TIMER_STATE_IDLE3, 
  TIMER_STATE_ON1, 
  TIMER_STATE_ON2, 
  TIMER_STATE_OFF
} TIMER_STATE;

TIMER_STATE timer_state;

static long dit_time;
static uint8_t duration;
static uint8_t duration_tmp = 0;

static uint16_t element_stop_time;
static uint32_t char_stop_time;
static uint32_t word_stop_time;
//static uint32_t delay_stop_time;      //BG6RDF
uint8_t call_time = 3;

static bool both_paddles_old;
static bool both_paddles_released;    
static uint16_t milliseconds;

bool keySwitch = false;
extern uint8_t hswr_timeout;
extern bool DitPaddle;
extern bool DahPaddle;
extern bool PaddleUpdate;
KEYER_AttrDef KEYER;

//PE10/41脚  CW/T控制脚，输出高低电平（同步于KEY1/KEY2，也就是点和划。输出高电平时间长短同步于CW）
#define CW_T GPIO_Pin_10
#define CW_T_PORT GPIOE

#define MAX_SIZE_OF_QUEUE 10
typedef struct{
  uint8_t get_pos;
  uint8_t save_pos;
  uint8_t size;
}Queue_AttrDef;
uint8_t Q_BUF[MAX_SIZE_OF_QUEUE];
Queue_AttrDef Queue;

uint8_t call_num = 0;
uint8_t call_sending;
void queue_init()
{
  Queue.get_pos = 0;
  Queue.save_pos = 0;
  Queue.size = 0;
}

void queue_save(uint8_t data)
{
  if(Queue.size < MAX_SIZE_OF_QUEUE)
  {
    Queue.size ++;
    Q_BUF[Queue.save_pos ++] = data;
    if(Queue.save_pos >= MAX_SIZE_OF_QUEUE)
    {
      Queue.save_pos = 0;
    }
  }
}

uint8_t queue_get()
{
  uint8_t ret = 0;
  if(Queue.size > 0)
  {
    Queue.size -- ;
    ret = Q_BUF[Queue.get_pos ++];
    if(Queue.get_pos >= MAX_SIZE_OF_QUEUE)
    {
      Queue.get_pos = 0;
    }
  }
  return ret;
}

void queue_clear()
{
  Queue.size = 0;
  Queue.save_pos=0;     //BG6RDF
  Queue.get_pos=0;      //BG6RDF
}

volatile uint16_t sTick = 0;    //改为volatile BG6RDF
void SysTickTask()
{
  sTick ++;
}

uint16_t SysTickGet()
{
  return sTick;
}

void SysTickSet(uint16_t tick)
{
  sTick = tick;
}

void keyer_init()
{
  GPIO_InitTypeDef GPIO_InitStructure;
  
  GPIO_InitStructure.GPIO_Pin =  CW_T;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_Init(CW_T_PORT, &GPIO_InitStructure);
  
  queue_init();
  
  TMR_Init(TIM4, 1000, 2, SysTickTask); //1ms 定时器3
  TMR_Stop(TIM4);
  TMR_IRQEnable(TIM4);
}

void CWT_On()
{
  GPIO_SetBits(CW_T_PORT, CW_T); 
}

void CWT_Off()
{
  GPIO_ResetBits(CW_T_PORT, CW_T); 
}

void keyer_auto_setup()
{
  both_paddles_old = false;
  both_paddles_released = false;
  queue_clear();
  
  buzzer_freq_set(MENU.freqOfbuzzer);
  sin_para_get();
  timer_state = TIMER_STATE_IDLE1;
}

// 单键模式
uint16_t TX_DELAY;
void keyer_manual_task()
{
  milliseconds = SysTickGet();
  
  //key_readkeyer();
  
  if (hswr_timeout)
  {
    if(!DahPaddle)
    {
      hswr_timeout = 0;
      oled_h_vswr_clear();
    }
    else
    {
      return;
    }
  }
  
  if (DahPaddle && !keySwitch)
  {
    keySwitch = true;
    RF_ToTx();
    buzzer_start(MENU.freqOfbuzzer); // Buzzer on
    milliseconds = 0;
    SysTickSet(0);
    CWT_On();  
  }
  else if(!DahPaddle && keySwitch)
  {
    keySwitch = false;
    buzzer_stop(); // Buzzer off
    CWT_Off();
    TX_DELAY = milliseconds;
  }
  
  if (!RF_IsRx() && !DahPaddle)
  {
    if (milliseconds - TX_DELAY>= MENU.delay*100)
    {
      // 转到RX模式
      RF_ToRx();
    }
  }
  
  
}

void call_enter()
{
  call_num = 0;
  call_sending = 8;
  call_time = 3;
  queue_clear();
  
  buzzer_freq_set(MENU.freqOfbuzzer);
  sin_para_get();
    
  timer_state = TIMER_STATE_IDLE2;
}

void call_exit()
{
  call_num = 0;
  call_sending = 0;
  queue_clear();
  buzzer_stop(); // Buzzer off
  CWT_Off();
  timer_state = TIMER_STATE_IDLE1;
}

void ascIndexToIambic(uint8_t index)
{
  uint8_t len; 
  uint16_t coder;
  coder = CallCoder[ index ];
  len = coder >> 8; // 呼号码 高8位为长度 低8位
  for(uint8_t i=0; i<len; i++)  // 呼号 最长MAX_CALLSIGN_LENGTH个,Morse码最长8个，BG6RDF
  {     
    if( coder & (1 << (len-1-i)) )
    {
      queue_save(3);    //哒
    }
    else
    {
      queue_save(1);    //嘀
    }
  }
}

void call_task()
{
  milliseconds = SysTickGet();
  dit_time = 1200 / WPM.val; // dit duration in milliseconds
  
  switch( call_sending )
  {
  case 0:
    // 发送完毕， 退出呼叫模式
    FREQ.item &= ~CALLItem;
    call_exit();
    RF_ToRx();
    break;
  case 1: // TEST
    if(call_num <= 3 && timer_state == TIMER_STATE_IDLE2)
    {
      if( call_num == 0)
      {
        ascIndexToIambic( 19 );  // 发送 T
      }
      else if( call_num == 1)
      {
        ascIndexToIambic( 4 );  // 发送 E
      }
      else if( call_num == 2)
      {
        ascIndexToIambic( 18 );  // 发送 S
      }
      else
      {
        ascIndexToIambic( 19 );  // 发送 T
      }
      call_num ++;
      timer_state = TIMER_STATE_IDLE1;
    }
    else if(timer_state == TIMER_STATE_IDLE3)
    {
      call_sending --;
      call_num = 0;
      timer_state = TIMER_STATE_IDLE2;
    }
    break;
  case 2: // K 
    if(call_num < 1 && timer_state == TIMER_STATE_IDLE2)
    {
      if( call_num == 0)
      {
        ascIndexToIambic( 10 );  // 发送 K
      }

      call_num ++;
      timer_state = TIMER_STATE_IDLE1;
    }
    else if(timer_state == TIMER_STATE_IDLE3)
    {
      call_sending = 0;
      call_num = 0;
      timer_state = TIMER_STATE_IDLE2;
    }
    break;
  case 3: // PSE
    if(call_num <= 2 && timer_state == TIMER_STATE_IDLE2)
    {
      if( call_num == 0)
      {
        ascIndexToIambic( 15 );  // 发送 P
      }
      else if( call_num == 1)
      {
        ascIndexToIambic( 18 );  // 发送 S
      }
      else
      {
        ascIndexToIambic( 4 );  // 发送 E
      }
      call_num ++;
      timer_state = TIMER_STATE_IDLE1;
    }
    else if(timer_state == TIMER_STATE_IDLE3)
    {
      call_sending --;
      call_num = 0;
      timer_state = TIMER_STATE_IDLE2;
    }
    break;
  case 4:
    if( call_num < MAX_CALLSIGN_LENGTH && MENU.callSign[call_num] != MORSE_CODE_COUNT && timer_state == TIMER_STATE_IDLE2)
    {
      ascIndexToIambic( MENU.callSign[call_num] );
      call_num ++;
      timer_state = TIMER_STATE_IDLE1;
    }
    else if( (call_num >= MAX_CALLSIGN_LENGTH || MENU.callSign[call_num] == MORSE_CODE_COUNT) && timer_state == TIMER_STATE_IDLE3)
    {
      if (--call_time == 0)
      {
        call_sending --;
      }
      timer_state = TIMER_STATE_IDLE2;
      call_num = 0;
    }
    break;
  case 5:   // 发送 DE  = 3,4
    if(call_num <= 1 && timer_state == TIMER_STATE_IDLE2)
    {
      if( call_num == 0)
      {
        ascIndexToIambic( 3 );  // 发送 D
      }
      else
      {
        ascIndexToIambic( 4 );  // 发送 E
      }
      call_num ++;
      timer_state = TIMER_STATE_IDLE1;
    }
    else if(timer_state == TIMER_STATE_IDLE3)
    {
      call_sending --;
      call_num = 0;
      timer_state = TIMER_STATE_IDLE2;
    }
    break;
  case 6:   // 发送 CQ  = 2,16
  case 7:   // 发送 CQ  = 2,16
  case 8:   // 发送 CQ  = 2,16
    if(call_num <= 1 && timer_state == TIMER_STATE_IDLE2)
    {
      if( call_num == 0)
      {
        ascIndexToIambic( 2 );  // 发送 C
      }
      else
      {
        ascIndexToIambic( 16 );  // 发送 Q
      }
      call_num ++;
      timer_state = TIMER_STATE_IDLE1;
    }
    else if(timer_state == TIMER_STATE_IDLE3)   //词结束且词结束后的GAP已完成
    {
      call_sending --;
      call_num = 0;
      timer_state = TIMER_STATE_IDLE2;
    }
    break;
  }
  
  if(timer_state == TIMER_STATE_IDLE1)
  {
    if( Queue.size > 0)
    {
      duration = queue_get();
      timer_state = TIMER_STATE_ON1;
      milliseconds = 0;
      SysTickSet(0);
    }
  }
  
  switch (timer_state)
  {  
  case TIMER_STATE_IDLE1:               //到这说明一个字符的点划一定结束了
    if (milliseconds >= char_stop_time && call_sending)
    {
      timer_state = TIMER_STATE_IDLE2;
    }
    break;
  case TIMER_STATE_IDLE2:               //词结束
    if (milliseconds >= word_stop_time && call_sending)
    {
      milliseconds = 0;
      SysTickSet(0);
      timer_state = TIMER_STATE_IDLE3;
    }      
    break;
  case TIMER_STATE_ON1:
    // 转到TX模式
    if (RF_IsRx())
    {
      RF_ToTx();
    }
    //delay_stop_time = 0;      BG6RDF
    element_stop_time = milliseconds + dit_time * duration; // dit or dah
    buzzer_start(); // Buzzer on
    CWT_On();
    timer_state = TIMER_STATE_ON2;
    break;
    
  case TIMER_STATE_ON2:
    if (milliseconds >= element_stop_time)
    {
      element_stop_time = milliseconds + dit_time; // space after dit or dah
      buzzer_stop(); // Buzzer off
      CWT_Off();
      timer_state = TIMER_STATE_OFF;
    }
    break;
    
  case TIMER_STATE_OFF:
    if (milliseconds >= element_stop_time)
    {
      char_stop_time = milliseconds + dit_time * 3; // gap between letters
      word_stop_time = milliseconds + dit_time * 7; // gap between words
      
      timer_state = TIMER_STATE_IDLE1;
      TX_DELAY = milliseconds;
    }
    break;
  }

}

// 双键模式
void keyer_auto_task()
{
  milliseconds = SysTickGet();
  dit_time = 1200 / WPM.val; // dit duration in milliseconds
  
  //key_readkeyer();
  
  if (PaddleUpdate)
  {
    PaddleUpdate = false;
    bool both_paddles = DitPaddle && DahPaddle;
    both_paddles_released = both_paddles_released || (both_paddles_old && !both_paddles);
    both_paddles_old = both_paddles;
    
    if (hswr_timeout)
    {
      if (!DitPaddle && !DahPaddle)
      {
        hswr_timeout = 0;
        oled_h_vswr_clear();
      }
      else
      {
        return;
      }
    }
    if (RADIO.mode == CWB && both_paddles_released)
    {
      both_paddles_released = false;
      duration_tmp = duration_tmp == 1 ? 3 : 1;
      queue_save(duration_tmp);
    }
    else if (DitPaddle && !DahPaddle)
    {
      //duration = 1; // dit
      if( MENU.key == 0)  // CW KEY:MODE0
      {
        if (timer_state == TIMER_STATE_IDLE1)
        {
          duration_tmp = 1;
          queue_save(duration_tmp);
        }
        else if (duration_tmp == 3 || duration_tmp == 0)
        {
          duration_tmp = 1;
          queue_save(duration_tmp);
        }
      }
      else  //CW KEY:MODE1
      {
        if (timer_state == TIMER_STATE_IDLE1)
        {
          duration_tmp = 3;
          queue_save(duration_tmp);
        }
        else if (duration_tmp == 1 || duration_tmp == 0)
        {
          duration_tmp = 3;
          queue_save(duration_tmp);
        }
      }
    }
    else if (!DitPaddle && DahPaddle)
    {
      //duration = 3; // dah
      if( MENU.key == 0)  // CW KEY:MODE0
      {
        if (timer_state == TIMER_STATE_IDLE1)
        {
          duration_tmp = 3;
          queue_save(duration_tmp);
        }
        else if (duration_tmp == 1 || duration_tmp == 0)
        {
          duration_tmp = 3;
          queue_save(duration_tmp);
        }
      }
      else  //CW KEY:MODE1
      {
        if (timer_state == TIMER_STATE_IDLE1)
        {
          duration_tmp = 1;
          queue_save(duration_tmp);
        }
        else if (duration_tmp == 3 || duration_tmp == 0)
        {
          duration_tmp = 1;
          queue_save(duration_tmp);
        }
      }
    }
    else if (DitPaddle && DahPaddle)
    {
      if(timer_state == TIMER_STATE_IDLE1)
      {
        duration_tmp = duration_tmp == 1 ? 3 : 1;
        queue_save(duration_tmp);
      }
    }
  }
  if(timer_state == TIMER_STATE_IDLE1)
  {
    if( Queue.size > 0)
    {
      duration = queue_get();
      timer_state = TIMER_STATE_ON1;
      milliseconds = 0;
      
      SysTickSet(0);
    }
  }
  
  switch (timer_state)
  {  

  case TIMER_STATE_ON1:
    // 转到TX模式
    if (RF_IsRx())
    {
      RF_ToTx();
    }
    //delay_stop_time = 0;      BG6RDF
    element_stop_time = milliseconds + dit_time * duration; // dit or dah
    buzzer_start(); // Buzzer on
    CWT_On();
    timer_state = TIMER_STATE_ON2;
    break;
    
  case TIMER_STATE_ON2:
    if (milliseconds >= element_stop_time)
    {
      element_stop_time = milliseconds + dit_time; // space after dit or dah
      buzzer_stop(); // Buzzer off
      CWT_Off();
      timer_state = TIMER_STATE_OFF;
    }
    break;
    
  case TIMER_STATE_OFF:
    if (milliseconds >= element_stop_time)
    {
      char_stop_time = milliseconds + dit_time * 3; // gap between letters
      word_stop_time = milliseconds + dit_time * 7; // gap between words
      
      timer_state = TIMER_STATE_IDLE1;
      TX_DELAY = milliseconds;
    }
    break;
  }
  
  if (!RF_IsRx())
  {
    if (milliseconds - TX_DELAY >= MENU.delay*100)
    {
      // 转到RX模式
      RF_ToRx();
    }
  }
}