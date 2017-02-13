#include "bsp_board.h"
#include "bsp_oled_api.h"
#include "bsp_key.h"
#include "bsp_adc.h"
#include "bsp_baud.h"
#include "bsp_iambic_keyer.h"
#include "bsp_24lc16.h"
#include "bsp_transmit_power.h"
#include "bsp_vol.h"
#include "bsp_ad9951.h"
#include "bsp_ad9834.h"
#include "bsp_pwm.h"
#include "bsp_timer.h"
#include "bsp_usart.h"

static KEY_AttrDef key;

extern KEYER_AttrDef KEYER;

DispRefreshMgr DRMgr;
AdjustItemMgr AIMgr;
FREQ_AttrDef FREQ;
RADIO_ModeDef CWMode;
// VFO���ñ���
VFO_SaveDef VFO;
VOL_AttrDef VOL;
ATT_AttrDef ATT;
RADIO_AttrDef RADIO;
TP_AttrDef TP;
DC_AttrDef DC;
FIL_AttrDef FIL;
WPM_AttrDef WPM;
MENU_AttrDef MENU;
MEM_AttrDef MEM[SIZE_OF_MEM];

uint32_t adc_val;
uint8_t tmp = 0;
uint8_t MemSetAll = 0;
RADIO_ModeDef Mode;
RADIO_ModeDef MemSetMode;
TP_LevelDef TPLevel;
uint8_t NumOfMem;

bool isRun = false;

bool RfIsTx = false;

uint16_t hswr_timeout= 0;
FIL_AttrDef CWFilter; 
uint8_t MemSetMaxIndex;

#define BFO_DEFAULT_VAL 8998070

uint32_t BAUD[SIZE_OF_BAUD][3] = {
  {179900, 190500, 200099},//160M:      1.799.00MHZ-2.000 .99MHZ
  {349900, 365000, 400099},//80M:       3.499.00MHZ-4.000.99MHZ
  {524900, 535000, 545099},//60M:       3.499.00MHZ-3.900.99MHZ
  {699900, 705000, 730099},//40M:       6.999.00MHZ-7.300.99MHZ
  {999900, 1012500, 1015099},//30M:       9.999.00MHZ-10.150.99MHZ
  {1399900, 1417500, 1435099},//20M:       13.999.00MHZ-14.350.99MHZ
  {1806700, 1811800, 1816899},//17M:       18.067.00MHZ-18.168.99MHZ
  {2099900, 2122500, 2145099},//15M:       20.999.00MHZ-21.450.99MHZ
  {2488900, 2494000, 2499099},//12M:       24.889.00MHZ-24.990.99MHZ
  {2799900, 2885000, 2970099}//10M:       27.999.00MHZ-29.700.99MHZ
};

uint8_t CALL_BUF[] = {
  'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
  'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '/', '0', '9', '8', '7', '6', '5', 
  '4', '3', '2', '1', '-'};

//�����ź�ƽ��
#define RF_ALPHA 0.9    //����ƽ��ϵ��
#define RX_ALPHA 0.9    //����ƽ��ϵ��
volatile static float last_rf_grid=0;
volatile static float last_rx_grid=0;

void vfo_init()
{
  // ����E2P
  for(uint8_t i=0; i<SIZE_OF_BAUD; i++)
  {
    if( i <= 2 )
    {
      VFO.VFOA[i].mode = LSB;
      VFO.VFOB[i].mode = LSB;
    }
    else
    {
      VFO.VFOA[i].mode = USB;
      VFO.VFOB[i].mode = USB;
    }
    VFO.VFOA[i].freq = BAUD[i][1];
    VFO.VFOB[i].freq = BAUD[i][1]; 
  }
  VFO.RIT = 0;
  VFO.XIT = 0;
  FREQ.numOfbaud = 3;  //7050000
  VFO.CurrBaudOfVfoA = 3;
  VFO.CurrBaudOfVfoB = 3;
}

void gpio_init()
{
      // GPIO enable clock and release Reset ,�ⲿ�ж� ����ʱ�ӱ����
  RCC_APB2PeriphResetCmd(  RCC_APB2Periph_GPIOA, DISABLE);
  RCC_APB2PeriphClockCmd(  RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE);
      // GPIO enable clock and release Reset
  RCC_APB2PeriphResetCmd(  RCC_APB2Periph_GPIOB, DISABLE);
  RCC_APB2PeriphClockCmd(  RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE);
  GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);      /*ʹ��SWD ����JTAG*/
  
      // GPIO enable clock and release Reset
  RCC_APB2PeriphResetCmd(  RCC_APB2Periph_GPIOC, DISABLE);
  RCC_APB2PeriphClockCmd(  RCC_APB2Periph_GPIOC | RCC_APB2Periph_AFIO, ENABLE);
  
      // GPIO enable clock and release Reset
  RCC_APB2PeriphResetCmd(  RCC_APB2Periph_GPIOD, DISABLE);
  RCC_APB2PeriphClockCmd(  RCC_APB2Periph_GPIOD | RCC_APB2Periph_AFIO, ENABLE);
  
    // GPIO enable clock and release Reset
  RCC_APB2PeriphResetCmd(  RCC_APB2Periph_GPIOE, DISABLE);
  RCC_APB2PeriphClockCmd(  RCC_APB2Periph_GPIOE, ENABLE);
}

void power_key_init()
{
  // 34��-PC5  ��Դ���ƽ� ͨ��Ϊ�ߵ�ƽ�� �ϵ�Ϊ�͵�ƽ
  GPIO_InitTypeDef GPIO_InitStructure;
  
  GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_5;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_Init(GPIOC, &GPIO_InitStructure);
  
  // PE15/46 VoiceControl : ����Ϊ��  ֹͣΪ��
  GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_15;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_Init(GPIOE, &GPIO_InitStructure);
  
  // PB10/47 H/L : ����Ϊ��  ֹͣΪ��
  GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_10;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
}

void voice_control_stop()
{
  GPIO_SetBits(GPIOE, GPIO_Pin_15);  
}

void voice_control_start()
{
  GPIO_ResetBits(GPIOE, GPIO_Pin_15);
}

void power_high_on()
{
  GPIO_SetBits(GPIOB, GPIO_Pin_10);  
}

void power_high_off()
{
  GPIO_ResetBits(GPIOB, GPIO_Pin_10);
}

void power_on()
{
  GPIO_SetBits(GPIOC, GPIO_Pin_5);  
}

void power_off()
{
  GPIO_ResetBits(GPIOC, GPIO_Pin_5); 
}

uint8_t save;
MENU_AttrDef tMem;
uint8_t buf[6];
VFO_SaveDef TVFO;

void ResetToDefault()
{
  e2p_save_write(0x5a);
    
    MENU.delay = 8;
    MENU.freqOfbuzzer = 600;
    MENU.dutyOfbuzzer = 2;
    MENU.key = 0;
    MENU.vswr = 9;
    //MENU.rbfo = 8998724;
    //MENU.rbfo = 8998870-800;
    MENU.rbfo = BFO_DEFAULT_VAL; //8998070;
//    MENU.tbfo = 9000000;
//    MENU.usbbfo = 9001500;
//    MENU.lsbbfo = 8998500;
    MENU.ref = 50000000;
    WPM.val = 15;
    MENU.cwnloOffset = 0;       //CWխ��ģʽ����ƫ�Ƴ�ֵ  BG6RDF

    vfo_init();
    RADIO.mode = LSB;
    TP.level = L;
    for(uint8_t i=0; i< MAX_CALLSIGN_LENGTH; i++)
    {
      MENU.callSign[i] = MORSE_CODE_COUNT;
    }
    for(uint8_t i=0; i<100; i++)
    {
      MEM[i].freq = 705000;
      MEM[i].baud = 3;
      MEM[i].mode = USB;
    }
    
    VOL.val = 2;
    e2p_mem_write(0, MEM, 100);
    e2p_vfo_write(&VFO);
    e2p_fil_write(FIL.func);
    e2p_vol_write(VOL.val);
    e2p_att_write(ATT.val);
    e2p_pwm_write(WPM.val);
    e2p_menu_write(&MENU);
}

void software_init()
{
  //e2p_save_write(0x11);

  if( e2p_save_read() == 0x5a)
  {
    e2p_mem_read(0, MEM, 100);
    VFO = e2p_vfo_read();
    FIL.func = e2p_fil_read();
    VOL.val = e2p_vol_read();
    ATT.val = e2p_att_read();
    WPM.val = e2p_pwm_read();
    MENU = e2p_menu_read();
  }
  else  // ����Ĭ������
  {
    ResetToDefault();
  }
  CWFilter.func = FIL.func;

  
 
  FREQ.item |=  VFOItem + VFOAItem;
  FREQ.numOfbaud = VFO.CurrBaudOfVfoA;
  VFO_Get();
  
  // ȫ��ˢ��
  DRMgr.item |= ALLItem;
}

#define POWER_ON_WAIT_COUNT 3000000
//�ȴ���Դ��ť����һ��ʱ��  BG6RDF
void WaitForPowerOn()
{
  GPIO_InitTypeDef GPIO_InitStructure;
  //�Ȱѵ�Դ��ť��������Ϊ���룬90��-PB4
  GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_4;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
  
  uint32_t cnt=0;
  while(1)
  {
   if (!GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_4))
     cnt++;
   else
     cnt=0;
   if (cnt>POWER_ON_WAIT_COUNT)
     return;
  }
}

void board_init()
{
  gpio_init();          //����GPIO
  tp_init();            //һЩGPIO���ŵ�����
  QuietModeOn();        //����
  
  WaitForPowerOn();     //�ȴ���Դ��ť����һ��ʱ��
  
  vol_init();           //����������������
  
  power_key_init();     //���õ�Դ���ߵ͹��ʿ�������
  power_on();           //PC5���øߵ�ƽ����������Դ
  i2c_init();
  
  delay_ms(50);
  vol_control(0);
  e2p_reset();
  
  software_init();
  key_init();           //��ʼ����ť���������͵����GPIO����ʼ��GPIO�жϺ��ж�����������ּ������Զ���
  
  //���K4,K6�����£���ָ���Ĭ��ֵ
  uint16_t portValue=GPIO_ReadInputData(GPIOD);
  if (!(portValue & GPIO_Pin_4) &&  !(portValue & GPIO_Pin_1))  //K4, K6Ҳ������
    ResetToDefault();

  oled_init();
  adc_init();
  baud_init();
  pwm_init();           //����PWM�����ڲ���
  keyer_init();
  
    
  ad9834_init();
  ad9951_init();
  
  usart_init();
  //oled_h_vswr_disp();
  RF_ToRx();
  isRun = true;
  
  //
//  buzzer_freq_set(500);
//  sin_para_get();
//  buzzer_start();
}

uint8_t FILFunc;
void FIL_Refresh(uint8_t all)
{
  if(FILFunc != FIL.func || all)
  {
    FILFunc = FIL.func;
    if(FILFunc == W)
    {
      oled_filter(" ");
    }
    else
    {
      oled_filter("N");
    }
  }
}
void TP_Refresh(uint8_t all)
{
  if(TPLevel != TP.level || all)
  {
    TPLevel = TP.level;
    if(TPLevel == H)
    {
      oled_tx_power(" ");
    }
    else if(TPLevel == M)
    {
      oled_tx_power("M");
    }
    else
    {
      oled_tx_power("L");
    }
  }
}

void MODE_Refresh(uint8_t all)
{
  if(Mode != RADIO.mode || all)
  {
    Mode = RADIO.mode;
    if(Mode == USB)
    {
      voice_control_start();
      oled_lsb_usb("USB ");
    }
    else if(Mode == LSB)
    {
      voice_control_start();
      oled_lsb_usb("LSB ");
    }
    else if(Mode == CWB)
    {
      voice_control_stop();
      oled_lsb_usb("CW/B");
      keyer_auto_setup();
    }
    else
    {
      voice_control_stop();
      oled_lsb_usb("CW/A");
      keyer_auto_setup();
    }
  }
}

void MEM_SetModeRefresh(uint8_t all)
{
  if(MemSetMode != RADIO.mode || all)
  {
    MemSetMode = RADIO.mode;
    if(MemSetMode == USB)
    {
      voice_control_start();
      oled_lsb_usb_mem("USB ");
    }
    else if(MemSetMode == LSB)
    {
      voice_control_start();
      oled_lsb_usb_mem("LSB ");
    }
    else if(MemSetMode == CWB)
    {
      voice_control_stop();
      oled_lsb_usb_mem("CW/B");
    }
    else
    {
      voice_control_stop();
      oled_lsb_usb_mem("CW/A");
    }
  }
}

void MENU_SetUp(uint8_t pos)
{
  switch(pos)
  {
  case 0:
    if( MENU.key == 0)
    {
      MENU.key = 1;
    }
    break;
  case 1:
    if( MENU.delay >= 15)
    {
      MENU.delay = 15;
    }
    else
    {
      MENU.delay ++;
    }
    break;
  case 2:
    if( MENU.freqOfbuzzer >= 1200)
    {
      MENU.freqOfbuzzer = 1200;
    }
    else
    {
      MENU.freqOfbuzzer ++;
    }
    break;
  case 3:
    if( MENU.cwMode == 0)
    {
      MENU.cwMode = 1;
    }
    break;
  case 4:
    if( MENU.vswr >= 9)
    {
      MENU.vswr = 9;
    }
    else
    {
      MENU.vswr ++;
    }
    break;
 
  case 5:
    if( MENU.rbfo >= 9999999)
    {
      MENU.rbfo = 9999999;
    }
    else
    {
      if (FREQ.item & TSItem)
      {
        MENU.rbfo += 100; // ����100HZ
      }
      else
      {
        MENU.rbfo ++;
      }
    }
    break;
  case 6:
    if( MENU.dutyOfbuzzer >= 29)
    {
      MENU.dutyOfbuzzer = 29;
    }
    else
    {
      MENU.dutyOfbuzzer ++;
    }
    break;
  case 7:
    if( MENU.ref >= 99999999)
    {
      MENU.ref = 99999999;
    }
    else
    {
      if (FREQ.item & TSItem)
      {
        MENU.ref += 100; // ����100HZ
      }
      else
      {
        MENU.ref ++;
      }
    }
    break;
  case 8:       //CW Narrow Band LO Offset
    if (FREQ.item & TSItem)
      MENU.cwnloOffset+=100;
    else
      MENU.cwnloOffset++;
    if (MENU.cwnloOffset>MAX_CWN_LO_OFFSET)
      MENU.cwnloOffset=MAX_CWN_LO_OFFSET;
    break;
  default:
    if(MENU.callSign[pos - 9] == MORSE_CODE_COUNT)
    {
      MENU.numOfcall = 0;
    }
    else if(MENU.numOfcall >= MORSE_CODE_COUNT)
    {
      MENU.numOfcall = 0;
    }
    else
    {
      MENU.numOfcall ++;
    }
    MENU.callSign[pos - 9] = MENU.numOfcall; 
    break;
  }
}

void MENU_SetDown(uint8_t pos)
{
  switch(pos)
  {
  case 0:
    if( MENU.key == 1)
    {
      MENU.key = 0;
    }
    break;
  case 1:
    if( MENU.delay <= 1)
    {
      MENU.delay = 1;
    }
    else
    {
      MENU.delay --;
    }
    break;
  case 2:
    if( MENU.freqOfbuzzer <= 500)
    {
      MENU.freqOfbuzzer = 500;
    }
    else
    {
      MENU.freqOfbuzzer --;
    }
    break;
  case 3:
    if( MENU.cwMode == 1)
    {
      MENU.cwMode = 0;
    }
    break;
  case 4:
    if( MENU.vswr <= 0)
    {
      MENU.vswr = 0;
    }
    else
    {
      MENU.vswr --;
    }
    break;

  case 5:
    if( MENU.rbfo <= 0)
    {
      MENU.rbfo = 0;
    }
    else
    {
      if (FREQ.item & TSItem)
      {
        if( MENU.rbfo >= 100)
        {
          MENU.rbfo -= 100; // ����100HZ
        }
      }
      else
      {
        MENU.rbfo --;
      }
    }
    break;
  case 6:
    if( MENU.dutyOfbuzzer <= 1)
    {
      MENU.dutyOfbuzzer = 1;
    }
    else
    {
      MENU.dutyOfbuzzer --;
    }
    break;
  case 7:
    if( MENU.ref <= 0)
    {
      MENU.ref = 0;
    }
    else
    {
      if (FREQ.item & TSItem)
      {
        if( MENU.ref >= 100)
        {
          MENU.ref -= 100; // ����100HZ
        }
      }
      else
      {
        MENU.ref --;
      }
    }
    break;
  case 8:       //CW Narrow Band LO Offset
    if (FREQ.item & TSItem)
      MENU.cwnloOffset-=100;
    else
      MENU.cwnloOffset--;
    if (MENU.cwnloOffset<(-MAX_CWN_LO_OFFSET))
      MENU.cwnloOffset=-MAX_CWN_LO_OFFSET;
    break;
  default:
    if(MENU.callSign[pos - 9] == MORSE_CODE_COUNT)      // pos�а�����ǰ��Ĳ˵�
    {
      MENU.numOfcall = MORSE_CODE_COUNT - 1;
    }
    else if(MENU.numOfcall <= 0)
    {
      MENU.numOfcall = MORSE_CODE_COUNT;
    }
    else
    {
      MENU.numOfcall --;
    }
    MENU.callSign[pos  -9] = MENU.numOfcall; 
    break;
  }
}

//�����ò˵�ģʽ�°���TS��ť�����ñ�����
void MENU_Ts(uint8_t pos)
{
  switch(pos)
  {
  case 0:
  case 1:
  case 2:
  case 3:
  case 4:
    FREQ.item &= ~TSItem;
    oled_ts("   ");
    break;
  case 5:
    MENU.rbfo /= 100;
    MENU.rbfo *= 100;
    break;
  case 6: //CW-VOL
    FREQ.item &= ~TSItem;
    oled_ts("   ");
    break;
//  case 6:
//    MENU.tbfo /= 100;
//    MENU.tbfo *= 100;
//    break;
//  case 7:
//    MENU.usbbfo /= 100;
//    MENU.usbbfo *= 100;
//    break;
//  case 8:
//    MENU.lsbbfo /= 100;
//    MENU.lsbbfo *= 100;
//    break;
  case 7:
    MENU.ref /= 100;
    MENU.ref *= 100;
    break;
  
  default:
    FREQ.item &= ~TSItem;
    oled_ts("   ");
    break;
  }
}


uint32_t Get_DDSLO(uint32_t freq)
{
  int32_t tmp;
  if( RADIO.mode == USB)
  {
    tmp = freq * 10 + 1500; // ��ƵƵ�� ��10
  }
  else if(RADIO.mode == LSB)
  {
    tmp = freq * 10 - 1500;
  }
  else  //CW
  {
    if( RF_IsTx() )
    {
      tmp = freq * 10;
    }
    else
    {
      if( FIL.func == N )
      {
        //CWխ��ģʽ�¼��ϱ���ƫ���������ڲ��������˲�������Ƶ��ƫ��
        tmp = freq * 10 + MENU.cwnloOffset;     //BG6RDF
      }
      else
      {
        tmp = freq * 10;
      }
    }
  }
  
  return tmp;
}

uint32_t Get_DDSBFO()
{
  uint32_t tmp;
  if( RADIO.mode == USB)
  {
    //tmp = MENU.usbbfo; 
    tmp = 9001500;
  }
  else if(RADIO.mode == LSB)
  {
    //tmp = MENU.lsbbfo;
    tmp = 8998500;
  }
  else
  {
    if( RF_IsRx() )
    {
      if( FIL.func == N )
      {
        tmp = MENU.rbfo;
      }
      else
      {
        tmp = 8999200;
      }
    }
    else
    {
      tmp = 0;  // TBFO �̶�Ϊ�� 
    }
  }
  
  return tmp;
}

void board_refresh_manager()
{
  uint8_t grid = 0;
  
  if (DRMgr.item > 0)
  {
    if( RADIO.mode == CWA || RADIO.mode == CWB )
    {
      // SSB��CW�л��󣬴洢�˲���״̬
      if( CWFilter.func == N )  
      {
        FIL.func = N;
      }
      else
      {
        FIL.func = W;
      }
    }
    else
    {
      // ��CW�л���SSBģʽ���ر��˲���
      FIL.func = W;
    } 
      
    if (DRMgr.item & FREQItem || DRMgr.item & ALLItem)
    {
      DRMgr.item &= ~FREQItem;  
      
      // ������״̬�˳�ʱ������VFO��ʾ
      if (FREQ.item & VFOItem && DRMgr.item & ALLItem)
      {
        if(FREQ.item & VFOAItem)
        {
          oled_vfo("VFO/A ");
        }
        else
        {
          oled_vfo("VFO/B ");
        }
      }
      else if (FREQ.item & MEMItem)   // �����������˳�ʱ�������ŵ��л�ʱ��ֻҪ��MEMģʽ�������ŵ���
      {
        oled_update_mem(FREQ.numOfmem);
      }
      
      
      
      if( DRMgr.item & ALLItem )  // �˳����ò˵�������ˢ����Ļ��Ƶ��
      {
        if(FREQ.item & MEMItem)
        {
          MEM_Get();
        }
        else
        {
          if( FREQ.item & VFOAItem )
          {
            FREQ.numOfbaud = VFO.CurrBaudOfVfoA;
          }
          else
          {
            FREQ.numOfbaud = VFO.CurrBaudOfVfoB;
          }
          VFO_Get();  // ��ʱ���α���û�б䶯������Ҫ���¶�ȡ��
        }
      }
      else if(!(FREQ.item & MEMItem)) // MEM��ʾ�˳��󣬲�����Ƶ�ʴ洢
      {
        VFO_Save();
      }
      
      // ������״̬�˳�ʱ������TS��ʾ
      if (FREQ.item & TSItem)
      {
        if( DRMgr.item & ALLItem )
        {
          oled_ts("TS");
        }
        FREQ.val /= 100;
        FREQ.val *= 100;
      }
      
      if (FREQ.item & RITItem && !(FREQ.item & MEMItem))  // RITʹ��ʱ����ʾƵ��=RIT + FREQ
      {
        if (DRMgr.item & ALLItem)
        {
          oled_rit("RIT");
        }
        oled_rit_txfreq(FREQ.val);
        
        oled_update_frequency(FREQ.val+VFO.RIT, (DRMgr.item & ALLItem)?1:0); 
        lo_update(Get_DDSLO(FREQ.val+VFO.RIT), MENU.ref);
        bfo_update(Get_DDSBFO(), MENU.ref);
      }
      else if (FREQ.item & XITItem && !(FREQ.item & MEMItem))   //XIT����ʾƵ��=FREQ+XIT BG6RDF
      {
        if (DRMgr.item & ALLItem)
        {
          oled_rit("XIT");
        }
        oled_rit_txfreq(FREQ.val);
        
        oled_update_frequency(FREQ.val+VFO.XIT, (DRMgr.item & ALLItem)?1:0); 
        //XITģʽҲ����ˢ�±�����ΪVFOA/VFOB�л�ʱ������Ҳ�����л�
        lo_update(Get_DDSLO(FREQ.val), MENU.ref);
        bfo_update(Get_DDSBFO(), MENU.ref);
      }
      else
      {
        oled_update_frequency(FREQ.val, (DRMgr.item & ALLItem)?1:0); 
        lo_update(Get_DDSLO(FREQ.val), MENU.ref);
        bfo_update(Get_DDSBFO(), MENU.ref);
      }
      baud_chn_ctrl(FREQ.numOfbaud);  // Ӳ�����ο���
      
      if (DRMgr.item == 0)
      {
        return;
      }
    }
    
    if (DRMgr.item & RADIOItem || DRMgr.item & ALLItem)
    {
      DRMgr.item &= ~RADIOItem;
      if (AIMgr.item & MEMSETItem)
      {
        MEM_SetModeRefresh(MemSetAll);
        MEM_Save();
      }
      else
      {
        MODE_Refresh((DRMgr.item & ALLItem)?1:0);
      
        // �ŵ�ģʽ�£�ģʽ�ɵ�����������ģʽ
        if(!(FREQ.item & MEMItem))
        {
          VFO_Save();
        }
      }
      
      if (DRMgr.item == 0)
      {
        return;
      }
    }
    
    if (DRMgr.item & FILItem || DRMgr.item & ALLItem)
    {
      DRMgr.item &= ~FILItem;
      
      if (FIL.func == N) // ��ʾʱ��ֻ��ʾN�� W����ʾ�� ������ʾ������ʾL
      {
        CW_FilterControl(1);
      }
      else
      {
        CW_FilterControl(0);
      }
      
      
      FIL_Refresh((DRMgr.item & ALLItem)?1:0);
      
      if (DRMgr.item == 0)
      {
        return;
      }
    }
    
    if (DRMgr.item < MEMSETItem && AIMgr.item <= 0)
    {
      if ( DRMgr.item & DCItem || DRMgr.item & ALLItem)
      {
        DRMgr.item &= ~DCItem;
        //4095-0: 2.5v-0
        // 4095 / adc = 2.5 / x => x = 2.5 * adc / 4096 
        // ��ѹ���� 10K 1K
        adc_val = 2500 * adc_batt();
        adc_val /= 4096;
        adc_val = adc_val * 11;
        DC.val = adc_val / 100;
        
        //DC.val = (uint32_t)((((adc_batt()*2474*61)/1000)/4096)*1.31);
        oled_update_dc(DC.val); 
        
        if (DRMgr.item == 0)
        {
          return;
        }
      }
      
      if ( DRMgr.item & SWRCHKItem || DRMgr.item & ALLItem)
      {
        DRMgr.item &= ~SWRCHKItem;
        
                //4096-0: 2.5v-0
        // 1.5:0.12V(196.608)  3:0.35V(573.44) 2:0.22(360.448)
        adc_val = adc_swr();
        
        if(adc_val > 574)
        {
          grid = 9;
        }
        else if( adc_val > 360)
        {
          grid = 6 + (adc_val-360) / 71; // (574-360) / 3 = 71
        }
        else if( adc_val > 20)
        {
          grid = (adc_val-20) / 56; // (360-20) / 6 = 56
        }
        else
        {
          grid = 0;
        }
        oled_swr_grid(grid);
        if( grid >= MENU.vswr && MENU.vswr > 0)
        {
          RF_ToRx();
          oled_h_vswr_disp();
          hswr_timeout = 1;
        }
        
        if (DRMgr.item == 0)
        {
          return;
        }
      }
      
      if ( DRMgr.item & RFCHKItem || DRMgr.item & ALLItem)
      {
        DRMgr.item &= ~RFCHKItem;
        //4096-0: 2.5v-0
        // 15W:1V(1638.4)  5W:0.6V(983.04)
        adc_val = adc_rf();
        
        /*
        if(adc_val > 1638)
        {
          grid = 13;
        }
        else if( adc_val > 983)
        {
          grid = 5 + (adc_val-983) / 82; // (1638-983) / 8 = 82
        }
        else if( adc_val > 20)
        {
          grid = (adc_val-20) / 192; // (983-20) / 5 = 192
        }
        else
        {
          grid = 0;
        }*/
        
        /**
        *   2016��9��9�� �޸���10W
        *       10W��ѹ0.66V   5�ߵ�ѹ0.47V
        *       4096-0: 2.5v-0
        *       10W:0.66V(1081.3)  5W:0.47V(770.04)
        */
        
        if(adc_val > 1081)
        {
          grid = 11;
        }
        else if( adc_val > 770)
        {
          grid = 6 + (adc_val-770) / 62; // (1081-770) / 5 = 62.2
        }
        else if( adc_val > 20)
        {
          grid = (adc_val-20) / 125; // (770-20) / 6 = 125
        }
        else
        {
          grid = 0;
        }
        
        //ƽ������������       BG6RDF
        float rf_grid=(float)grid;
        if (rf_grid<last_rf_grid)
        {
          rf_grid=last_rf_grid*RF_ALPHA+rf_grid*(1-RF_ALPHA);
          grid=(uint32_t)rf_grid;
        }
        last_rf_grid=rf_grid;
        
        oled_rf_grid(grid);
        if (DRMgr.item == 0)
        {
          return;
        }
      }
      
      if ( DRMgr.item & RXCHKItem || DRMgr.item & ALLItem)
      {
        
        DRMgr.item &= ~RXCHKItem;
        //4096-0: 2.5v-0
        // +40:0.8V(1310.72)  +20:0.67(1097.728) 9:0.54(819.2) 6:0.04(65.536) 3:0.008(13.1072) 
        // 2.4V (3932.16)        2V (3276.8)       1.47V (2408.448)       5:0.08(131.072)        1:0.02(32.768)
        adc_val = adc_rx();
        
        if(adc_val > 3932)
        {
          grid = 25;
        }
        else if( adc_val > 3276)
        {
          grid = 21 + (adc_val-3276) / 164; // (3932-3276) / 4 = 164
        }
        else if( adc_val > 2408 )
        {
          grid = 17 + (adc_val-2408) / 217; // (3276-2408) / 4 = 217
        }
        else if( adc_val > 131 )
        {
          grid = 10 + (adc_val-131) / 325;// (2048-131) / 7 = 325
        }
        else if( adc_val > 32 )
        {
          grid = 0 + (adc_val-32) / 11; // (131-32) / 9 = 11
        }
        else
        {
          grid = 0;
        }

        //ƽ������������
        float rx_grid=(float)grid;
        if (rx_grid<last_rx_grid)
        {
          rx_grid=last_rx_grid*RX_ALPHA+rx_grid*(1-RX_ALPHA);
          grid=(uint32_t)rx_grid;
        }
        last_rx_grid=rx_grid;
        
        oled_rx_grid(grid);
        
        if (DRMgr.item == 0)
        {
          return;
        }
      }
      
      if (DRMgr.item & VOLItem || DRMgr.item & ALLItem)
      {
        DRMgr.item &= ~VOLItem;
        oled_update_vol(VOL.val);
        vol_control(VOL.val);

        
        if (DRMgr.item == 0)
        {
          return;
        }
      }
      
      // ATT����ȥ���� LOCKƵ����ť�������ܴ���
      if (DRMgr.item & ATTItem  || DRMgr.item & ALLItem)
      {
        DRMgr.item &= ~ATTItem;
        //oled_update_att(ATT.val);   // 46-ATT : ����Ϊ��  ֹͣΪ��
        oled_button_lock(ATT.val);   // 1: LOCK����״̬������Ĭ�ϣ� 0: ����״̬
//
//        if (ATT.val >= 1)
//        {
//          rf_att_on();
//        }else{
//          rf_att_off();
//        }
        if (DRMgr.item == 0)
        {
          return;
        }
      }
      
      
      
      if (DRMgr.item & TPItem || DRMgr.item & ALLItem)
      {
        DRMgr.item &= ~TPItem;
        TP_Refresh((DRMgr.item & ALLItem)?1:0); // 47�ţ� H-1 L-0
        if (TP.level == L)
        {
          power_high_off();
        }
        else
        {
          power_high_on();
        }
        if (DRMgr.item == 0)
        {
          return;
        }
      }
      
      if (DRMgr.item & TXRXItem || DRMgr.item & ALLItem)
      {
        DRMgr.item &= ~TXRXItem;
        if( RF_IsRx() )
        {
          oled_txrx("RX");
          if(FREQ.item & RITItem && !(FREQ.item & MEMItem))
          {
            lo_update(Get_DDSLO(FREQ.val+VFO.RIT), MENU.ref);
            bfo_update(Get_DDSBFO(), MENU.ref);
          }
          else if(FREQ.item & XITItem && !(FREQ.item & MEMItem))     //BG6RDF
          {
            lo_update(Get_DDSLO(FREQ.val), MENU.ref);
            bfo_update(Get_DDSBFO(), MENU.ref);
          }
          else                                                       //BG6RDF
          {
            lo_update(Get_DDSLO(FREQ.val), MENU.ref);
            bfo_update(Get_DDSBFO(), MENU.ref);
          }
        }
        else
        {
          oled_txrx("TX");
          if(FREQ.item & RITItem && !(FREQ.item & MEMItem))
          {
            lo_update(Get_DDSLO(FREQ.val), MENU.ref);
            bfo_update(Get_DDSBFO(), MENU.ref);
          }
          else if(FREQ.item & XITItem && !(FREQ.item & MEMItem))     //BG6RDF
          {
            lo_update(Get_DDSLO(FREQ.val+VFO.XIT), MENU.ref);
            bfo_update(Get_DDSBFO(), MENU.ref);
          }
          else                                                       //BG6RDF
          {
            lo_update(Get_DDSLO(FREQ.val), MENU.ref);
            bfo_update(Get_DDSBFO(), MENU.ref);
          }
        }
        oled_txrx_clear_grid();  // TX��RX״̬�л������������ʾ����
        if (DRMgr.item == 0)
        {
          return;
        }
      }
      
      if (DRMgr.item & ALLItem)
      {
        oled_update_rx_scaler();
        oled_update_tx_scaler();
      }
      
     
    }
    else  // ALL SET REFRESH
    {
      if (DRMgr.item & MEMSETItem)
      {
        DRMgr.item &= ~MEMSETItem; 
        if (FREQ.item & TSItem)
        {
          FREQ.val /= 100;
          FREQ.val *= 100;
        }
        oled_mem_set(FREQ.numOfmem, FREQ.val, MemSetAll);
        MemSetAll = 0;
        MEM_Save();
        lo_update(Get_DDSLO(FREQ.val), MENU.ref);       //���±����BFO������������Ƶ��ʱ���տ���ͬ��
        bfo_update(Get_DDSBFO(), MENU.ref);
        if (DRMgr.item == 0)
        {
          return;
        }
      }
      
      if (DRMgr.item & WPMSETItem )
      {
        DRMgr.item &= ~WPMSETItem;
        oled_update_wpm(WPM.val); 
        
        if (DRMgr.item == 0)
        {
          return;
        }
      }
      
      if (DRMgr.item & MENUSETItem )
      {
        DRMgr.item &= ~MENUSETItem;
        if (FREQ.item & TSItem)
        {
          MENU_Ts(MENU.numOfmenu);
        }
        oled_menu_set(MENU.numOfmenu); 
        oled_menu_arrows_set(MENU.numOfmenu);
        lo_update(Get_DDSLO(FREQ.val), MENU.ref);
        bfo_update(Get_DDSBFO(), MENU.ref);
        
        if (DRMgr.item == 0)
        {
          return;
        }
      }
    }
    if (DRMgr.item & ALLItem)
    {
      oled_display_on();
      DRMgr.item &= ~ALLItem;
    }
  }
}

void key_task()
{
  if( RADIO.mode == USB || RADIO.mode == LSB )
  {
    if( PttPressed && !PttFinish )
    {
      PttFinish = true;
      RF_ToTx();
    }
    else if (!PttPressed && PttFinish)
    {
      PttFinish = false;
      RF_ToRx();
      if (hswr_timeout)
      {
        hswr_timeout = 0;
        oled_h_vswr_clear();
      }
    }
  }
  
  if ( key_ispress( &key ) )
  {
    switch(key.val)
    {
    case 1:   // K1:  ��Դ���ػ�����
      if ((AIMgr.item & MENUSETItem) || RF_IsTx())
      {
        break;
      }
      
      if( key.state == keyShot)
      {
        
      }
      else if( key.state == keyLong )
      {        
        if (isRun)
        {
          isRun = false;
          
          
          e2p_mem_write(0, MEM, MemSetMaxIndex+1);
          e2p_vfo_write(&VFO);
          e2p_fil_write(FIL.func);
          e2p_vol_write(VOL.val);
          e2p_att_write(ATT.val);
          e2p_pwm_write(WPM.val);
          e2p_menu_write(&MENU);
          oled_display_off();
          delay_ms(100);
          
          power_off();
        }
      }
      
      break;
    case 2:   // V/M �ŵ�/VFOת���������������ŵ����ܣ�Ҳ�ǲ˵��˳�����
      if ((AIMgr.item & MENUSETItem) || RF_IsTx())
      {
        
      }
      else if( key.state == keyShot )
      {
        if( AIMgr.item & MEMSETItem)
        {
          AIMgr.item &= ~MEMSETItem;
          DRMgr.item |= ALLItem;
          MEM_Save();
          oled_set_screen(0x00);
        }
        else
        {
          if (FREQ.item & VFOItem)
          {
            FREQ.item &= ~VFOItem;
            FREQ.item |= MEMItem;
            DRMgr.item |= FILItem;
            MEM_Get();
          }
          else
          {
            FREQ.item &= ~MEMItem;
            FREQ.item |= VFOItem;
            
            if(FREQ.item & VFOAItem)
            {
              oled_vfo("VFO/A ");
              FREQ.numOfbaud = VFO.CurrBaudOfVfoA;
            }
            else
            {
              oled_vfo("VFO/B ");
              FREQ.numOfbaud = VFO.CurrBaudOfVfoB;
            }
            VFO_Get();
          }
          DRMgr.item |= RADIOItem;  // ����ģʽ
          DRMgr.item |= FREQItem;   // ������Ƶ�Ͳ���
          DRMgr.item |= FILItem;
        }
      }
      else if( key.state == keyLong )
      {
        if(FREQ.item & MEMItem)
        {
          MEM_Get();
          oled_set_screen(0x00);
          
          AIMgr.item |= MEMSETItem;
          DRMgr.item |= MEMSETItem;   // ����MEM������Ƶ
          DRMgr.item |= RADIOItem;    // ����MEM����ģʽ
          DRMgr.item |= FILItem;
          MemSetAll = 1;              // ǿ�Ƹ�������ģʽ CH: HZ
          if (FREQ.item & TSItem)
          {
            oled_ts("TS");
          }
        }
      }
      
      break;
    case 3:   // MODE����ÿ��һ����LSB,USB,CW֮��ת���������л��˲���W/N
      if ((AIMgr.item & MENUSETItem) || RF_IsTx())
      {
        
      }
      else if( key.state == keyShot )
      {
        if (RADIO.mode == LSB)
        {
          if( CWMode == CWA )
          {
            RADIO.mode = CWA;
          }
          else
          {
            RADIO.mode = CWB;
          }
        }
        else if (RADIO.mode == USB)
        {
          RADIO.mode = LSB;         
        }
        else
        {
          RADIO.mode = USB;
        }
        
        if( AIMgr.item & MEMSETItem)
        {
          DRMgr.item |= MEMSETItem;
        }
        else
        {
          DRMgr.item |= FREQItem;   // ������Ƶ
        }
        DRMgr.item |= RADIOItem;
        DRMgr.item |= FILItem;
        
      }
      else if( key.state == keyLong)
      {
        if (RADIO.mode == CWA || RADIO.mode == CWB)
        {
          if (FIL.func == W)
          {
            FIL.func = N;
            CWFilter.func = N;
          }
          else 
          {
            FIL.func = W;
            CWFilter.func = W;
          }
          DRMgr.item |= FILItem;
          if( !(AIMgr.item & MEMSETItem) )  // MEMSET�°� N ����СCH:09
          {
            DRMgr.item |= FREQItem; // ����BFO��LO
          }
        }
        
      }
      break;
    case 4:   // A/B������VFOģʽ�¶̰���VFOA/VFOB֮��ת����������CWA/CWB֮��ת����ֻ��CWģʽ�£�
      if ((AIMgr.item & MENUSETItem) || RF_IsTx())
      {
        
      }
      else if( key.state == keyShot && AIMgr.item <= 0)
      {
        if(FREQ.item & VFOItem)
        {
          if (FREQ.item & VFOBItem)
          {
            FREQ.item |= VFOAItem; 
            FREQ.item &= ~VFOBItem;
            oled_vfo("VFO/A ");
            FREQ.numOfbaud = VFO.CurrBaudOfVfoA;
          }
          else
          {
            FREQ.item |= VFOBItem; 
            FREQ.item &= ~VFOAItem;
            oled_vfo("VFO/B ");
            FREQ.numOfbaud = VFO.CurrBaudOfVfoB; 
          }
          VFO_Get();
          DRMgr.item |= FREQItem;
          DRMgr.item |= RADIOItem; 
          DRMgr.item |= FILItem;
        }
      }
      else if( key.state == keyLong )
      {
        if( RADIO.mode == CWA || RADIO.mode == CWB)
        {
          if (RADIO.mode == CWA)
          {
            CWMode = CWB;
            RADIO.mode = CWB;
          }else {
            RADIO.mode = CWA;
            CWMode = CWA;
          }
          if( AIMgr.item & MEMSETItem)
          {
            DRMgr.item |= MEMSETItem;
          }
          DRMgr.item |= RADIOItem;  
        }
      }
      break;
    case 5:   // ���ΰ�����ÿ�̰�һ�������в���֮��ת������������˵�
      if ( RF_IsTx())
      {
        
      }
      else if( key.state == keyShot)
      {     
        if( AIMgr.item & MENUSETItem)           //�����������ģʽ�����˳�����ģʽ
        {
          AIMgr.item &= ~MENUSETItem;
          DRMgr.item |= ALLItem;
          buzzer_stop();
          QuietModeOff();
          oled_set_screen(0x00);                //����
        }
        else
        // XIT BG6RDF
        if((FREQ.item & VFOItem) && (AIMgr.item <= 0) && !(FREQ.item & XITItem))
        {
          FREQ.item |= XITItem;         //����XIT
          FREQ.item &= ~RITItem;        //XIT��RIT����
          oled_rit("XIT");
          
          DRMgr.item |= FREQItem;
        }
        else if (FREQ.item & XITItem)
        {
          FREQ.item &= ~XITItem;        //�˳�XIT
          oled_rit("   ");
          oled_rit_txfreq_clear();
          DRMgr.item |= FREQItem;
        } 
        /*      //ԭ�������ǲ����л���ť���ָ�ΪXIT��ť  BG6RDF
        else if (!(FREQ.item & MEMItem) || AIMgr.item & MEMSETItem)
        {
          if (FREQ.numOfbaud >= SIZE_OF_BAUD-1)
          {
            FREQ.numOfbaud = 0;
          }
          else
          {
            FREQ.numOfbaud ++; 
          }

          if( AIMgr.item & MEMSETItem)
          {
            FREQ.val = BAUD[FREQ.numOfbaud][1];
            DRMgr.item |= MEMSETItem;
          }
          else
          {
            VFO_Get();
            DRMgr.item |= FREQItem;
            DRMgr.item |= RADIOItem; 
            DRMgr.item |= FILItem;
          }
        }
        */
        
      }
      else if( key.state == keyLong )
      {
        oled_set_screen(0x00);
        
        AIMgr.item |= MENUSETItem;
        DRMgr.item |= MENUSETItem;
        MENU.numOfmenu = 0; // �ò˵�ȫ����ʾ����
        if (FREQ.item & TSItem)
        {
          oled_ts("TS");
        }
      }
      break;
    case 6:   // RIT�������̰��ǿ��ٵ�TS������������RIT���˳�Ҳ����
      if (RF_IsTx())
      {
        
      }
      else if( key.state == keyShot )
      {
        if (FREQ.item & TSItem)
        {
          FREQ.item &= ~TSItem;
          
          oled_ts("   ");
        }
        else  // 3��״̬�£��ŵ�����,�˵�����,VFO/MEM
        {     // ���ã����㼰ˢ��TS��ʾ
          FREQ.item |= TSItem;          
          oled_ts("TS");
          
          if( AIMgr.item & MEMSETItem)
          {
            DRMgr.item |= MEMSETItem;
          }
          else if( AIMgr.item & MENUSETItem)
          {            
            DRMgr.item |= MENUSETItem;
          }
          else
          {
            DRMgr.item |= FREQItem;
          }
        }
      }
      else if( key.state == keyLong)
      {
        // ֻ��VFOģʽ��������
        if((FREQ.item & VFOItem) && (AIMgr.item <= 0) && !(FREQ.item & RITItem))
        {
          FREQ.item |= RITItem;
          FREQ.item &= ~XITItem;        //XIT��RIT����     BG6RDF
          oled_rit("RIT");
          
          DRMgr.item |= FREQItem;
        }
        else if (FREQ.item & RITItem)
        {
          FREQ.item &= ~RITItem;
          oled_rit("   ");
          oled_rit_txfreq_clear();
          DRMgr.item |= FREQItem;
        }
      }
      
      break;

    case 7:   // CH�ŵ���ť��	�̰�ATT���ã���������CALL���У��ں��й����ж̰�һ��ֹͣ���жϣ�����
      if (AIMgr.item & MENUSETItem)
      {
        
      }
      else if( key.state == keyShot )
      {
        if(FREQ.item & CALLItem)
        {
          FREQ.item &= ~CALLItem;
          call_exit();
          // ת��RXģʽ
          RF_ToRx();
        }
        else
        {
          if (RADIO.mode == CWA || RADIO.mode == CWB)
          {
            FREQ.item |= CALLItem;
            call_enter();
          }
        }
        
      }
      else if( key.state == keyLong )
      {
        if (ATT.val >= 1)
        {
          ATT.val = 0;
        }else{
          ATT.val = 1;
        }
        DRMgr.item |= ATTItem;
      }
      break;
    case 8:   // VOL�����������̰���������H/M/L��������CW�ٶȵ�����ת����������������WPM
      if ((AIMgr.item & MENUSETItem) || (AIMgr.item & MEMSETItem) || RF_IsTx())
      {
        
      }
      else if( key.state == keyShot )
      {
        if (AIMgr.item > 0)
        {
          if( AIMgr.item == WPMSETItem )
          {
            AIMgr.item &= ~WPMSETItem;
            oled_close_wpm();  
          }
        }
        else
        {
          //if (RADIO.mode == CWA || RADIO.mode == CWB) // ���ʵ��� ֻ����CWģʽ�²���Ч
          {
            if (TP.level == L)
            {
              TP.level = H;
              TP.val = " ";
            }
            else
            {
              TP.level = L;
              TP.val = "L";
            }
            DRMgr.item |= TPItem;
          }
        }
      }
      else if(key.state == keyLong)
      {
        AIMgr.item |= WPMSETItem;
        DRMgr.item |= WPMSETItem;
      }
      break;
    }
    
    key_clear();
  }
}

void UART_Task()
{
  uint8_t size = 0;
  if (DMA_IsTxComplete())
  {
    size = UART_TxBufSize(); 
    if(size > 0){
      // ���ڷ��ʹ���
      CopyTBufToDMA();
      UART_DMAStart( size );	
    }
  }
}

void board_task()
{
  key_task();
  
  board_refresh_manager();
  
  if( RADIO.mode == CWA || RADIO.mode == CWB)
  {
    if(FREQ.item & CALLItem)
    {
      call_task();
    }
    else if(MENU.cwMode == MANUAL && AIMgr.item <= 0)
    {
      keyer_manual_task();
    }
    else if(AIMgr.item <= 0)
    {
      keyer_auto_task();
    }
  }
  
  UART_Task();
}

// ��W/V�л���VFOA/B�л��������л����˵��˳�����£�������
void VFO_Get()
{
  if(FREQ.item & VFOBItem)
  {
    FREQ.val = VFO.VFOB[FREQ.numOfbaud].freq;
    RADIO.mode = VFO.VFOB[FREQ.numOfbaud].mode;
  }
  else
  {
    FREQ.item |= VFOAItem;
    FREQ.val = VFO.VFOA[FREQ.numOfbaud].freq;
    RADIO.mode = VFO.VFOA[FREQ.numOfbaud].mode;
  }
}

// ��Ƶ�ʺ�ģʽ���µ�����£�������
void VFO_Save()
{
  if(FREQ.item & VFOAItem)
  {
    VFO.VFOA[FREQ.numOfbaud].freq = FREQ.val;
    VFO.VFOA[FREQ.numOfbaud].mode = RADIO.mode;
    VFO.CurrBaudOfVfoA = FREQ.numOfbaud;
  }
  else if(FREQ.item & VFOBItem)
  {
    VFO.VFOB[FREQ.numOfbaud].freq = FREQ.val;
    VFO.VFOB[FREQ.numOfbaud].mode = RADIO.mode;
    VFO.CurrBaudOfVfoB = FREQ.numOfbaud;
  }
}

void MEM_Get()
{
  FREQ.val = MEM[FREQ.numOfmem].freq;
  FREQ.numOfbaud = MEM[FREQ.numOfmem].baud;
  RADIO.mode = MEM[FREQ.numOfmem].mode;
}

void MEM_Save()
{
  MEM[FREQ.numOfmem].freq = FREQ.val;
  MEM[FREQ.numOfmem].baud = FREQ.numOfbaud;
  MEM[FREQ.numOfmem].mode = RADIO.mode;
  if( MemSetMaxIndex < FREQ.numOfmem )
  {
    MemSetMaxIndex = FREQ.numOfmem;
  }
}

void RF_Tx()
{
  // TX������ƶˣ�������SSBģʽ����CWģʽ���ڷ���ʱ����ߵ�ƽ��
  tx_control_on();
  
  // �������ƣ��ڷ���ʱ����ߵ�ƽ
  QuietModeOn();
  
  //  CW�˲����л����ƣ���SSBģʽ��Ĭ�ϵ͵�ƽ����CWģʽ�� RXʱĬ�ϸߵ�ƽ��N խ������
  //         ��CMģʽ ����ʱ Ϊ�͵�ƽ��W �����
  CW_FilterControl(0);
  
  // MIC���ƣ���CWģʽ�·���ʱ����͵�ƽ��RXʱҲ����͵�ƽ��ֻ����SSBģʽ������ߵ�ƽ
  if( RADIO.mode == CWA || RADIO.mode == CWB )
  {
    MICControlOff();
    // BFO�л� CW����״̬�£�����ߵ�ƽ; �������ȫΪ�͵�ƽ
    CW_BFOControl(1);
    
    TMR_Start(TIM4); // ����1ms ʱ�Ӷ�ʱ����
  }
  else
  {
    MICControlOn();
    CW_BFOControl(0);
    TMR_Stop(TIM4);
  }
}

void RF_Rx()
{
  tx_control_off();
  
  QuietModeOff();
  
  MICControlOff();

  CW_BFOControl(0);
  
  if( RADIO.mode == CWA || RADIO.mode == CWB )
  {
    if (FIL.func == N) // ��ʾʱ��ֻ��ʾN�� W����ʾ�� ������ʾ������ʾL
    {
      CW_FilterControl(1);
    }
    else
    {
      CW_FilterControl(0);
    }
  }
}

//�л����պ�Ƶ����board_refresh_manager�д���TXRXItem�¼�����ʱ���� BG6RDF
void RF_ToRx()
{
  RfIsTx = false;
  RF_Rx();
  DRMgr.item |= TXRXItem;       //�շ�ת���¼�
}

//�л�����ǰӦ���÷���Ƶ�ʣ���Ӧ�ȵ�board_refresh_manager�д��� BG6RDF
void RF_ToTx()
{
  RfIsTx = true;
  RF_Tx();
  DRMgr.item |= TXRXItem;       //�շ�ת���¼�
}

bool RF_IsTx()
{
  return RfIsTx;
}

bool RF_IsRx()
{
  return !RfIsTx;
}




