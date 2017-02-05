/*
  本软件 没有设置和读取 滤波器状态的命令 (N和W)
*/

#include "bsp_kenwood.h"
#include "bsp_usart.h"
#include "bsp_oled_api.h"
#include "bsp_iambic_keyer.h"
#include "bsp_key.h"

/**
 * itoa() 将整型值转换为字符串
 * itoa() 将长整型值转换为字符串
 * ultoa() 将无符号长整型值转换为字符串 
*/
uint32_t StringToDec(uint8_t *pBuf, uint8_t len)
{
  uint32_t result = 0;
  uint8_t i;
  
  for(i=0; i<len-1; i++)
  {
    result += (pBuf[i] - 0x30); 
    result *= 10;
  }
  result += (pBuf[len-1] - 0x30);;
  
  return result;
}

void DecToString(uint8_t *pBuf, uint8_t len, int32_t dat)
{
  int32_t sign;  
  
  if( (sign=dat) < 0)
  {
    dat = -dat;
  }
  
  for(uint8_t i=0; i<len; i++)
  {
    pBuf[len-1-i] = dat % 10 + '0'; 
    dat /= 10;
  }
   
  if(sign < 0) 
    pBuf[0] = '-'; 
}
//整数转字符串,整数(最大长度 10 位) + (符号位 1 位 [-]) + (字符串结束'\0' [1位])
void Myitoa(int32_t n, uint8_t str[]) 
{ 
    int32_t i, sign; 
    uint8_t s[12]; 
    uint8_t j=0;
    
    if((sign=n) < 0) 
        n=-n; 
    i=0; 
    do 
    { 
        s[i++] = n % 10 + '0'; 
    }while ((n/=10) > 0); 

    if(sign < 0) 
        s[i++] = '-'; 

    s[i] = '\0'; 

    //for (; i>0; i-- )
    do{
        str[j++] = s[i-1];
    }while(--i > 0);
}


void Kenwood_BaudSet(uint8_t baud)
{
  if ( RF_IsTx() || !(FREQ.item & MEMItem) || AIMgr.item & MEMSETItem)
  {
    return;
  }
  FREQ.numOfbaud = baud;   
  
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

void Kenwood_BuildTransceiverStatus()
{
  uint8_t buf[40];
  uint8_t len = 0;
  
  buf[len ++] = 'I';
  buf[len ++] = 'F';
  
  // P1 显示频率
  buf[len ++] = '0';
  buf[len ++] = '0';
  buf[len ++] = '0';
  DecToString(&buf[len], 7, (int32_t)FREQ.val);
  len += 7;
  buf[len ++] = '0';
  
  // P2 Spaces(5)
  buf[len ++] = ' ';
  buf[len ++] = ' ';
  buf[len ++] = ' ';
  buf[len ++] = ' ';
  buf[len ++] = ' ';
  
  // P3 RIT/XIT frequency (5) BG6RDF
  if (FREQ.item & RITItem)
    DecToString(&buf[len], 5, (int32_t)VFO.RIT);
  if (FREQ.item & XITItem)
    DecToString(&buf[len], 5, (int32_t)VFO.XIT);
  len += 5;
  
  // P4 RIT 0:OFF / 1:ON
  if (FREQ.item & RITItem)
  {
    buf[len ++] = '1';
  }
  else
  {
    buf[len ++] = '0';
  }
  
  // P5 XIT
  if (FREQ.item & XITItem)
    buf[len ++] = '1';          //BG6RDF
  else
    buf[len ++] = '0';
  
  // P6 P7 Memory channel number
  buf[len ++] = '1';
  buf[len ++] = (FREQ.numOfmem % 100 / 10) + 0x30;
  buf[len ++] = (FREQ.numOfmem % 10) + 0x30;
  
  // P8 0:RX  1:TX
  if( RF_IsRx() )
  {
    buf[len ++] = '0';
  }
  else
  {
    buf[len ++] = '1';
  }
  
  // P9 Operating mode 1:LSB 2:USB 3:CW
  if( RADIO.mode == LSB)
  {
    buf[len ++] = '1';
  }
  else if(RADIO.mode == USB)
  {
    buf[len ++] = '2';
  }
  else
  {
    buf[len ++] = '3';
  }
  
  // P10 Function (refer to the FR/FT commands) 0:VFOA 1:VFOB 2:Mem
  if(FREQ.item & VFOItem)
  {
    if( FREQ.item & VFOAItem )
    {
      buf[len ++] = '0';
    }
    else
    {
      buf[len ++] = '1';
    }
  }
  else
  {
    buf[len ++] = '2';
  }
  
  // P11 Scan status (refer to the SC command) 0: Scan OFF
  buf[len ++] = '0';
  
  // P12 0: Simplex operation 1: Split operation
  buf[len ++] = '0';
  
  // P13 
  buf[len ++] = '0';
  
  // P14 
  buf[len ++] = '0';
  buf[len ++] = '0';
  
  // P15
  buf[len ++] = '0';
  
  buf[len ++] = ';';
  
  UartTx(buf, len);
}

void Kenwood_MemoryDataAnswer()
{
  uint8_t buf[40];
  uint8_t len = 0;
  
  buf[len ++] = 'M';
  buf[len ++] = '4';
  
  // P1 0: Simplex  1: Split
  buf[len ++] = '0';
  
  // P2 P3 Channel number (refer to the MC command)
  buf[len ++] = '1';
  buf[len ++] = (FREQ.numOfmem % 100 / 10) + 0x30;
  buf[len ++] = (FREQ.numOfmem % 10) + 0x30;
  
  // P4 frequency 
  buf[len ++] = '0';
  buf[len ++] = '0';
  buf[len ++] = '0';
  DecToString(&buf[len], 7, (int32_t)FREQ.val);
  len += 7;
  buf[len ++] = '0';
  
  // P5 Operating mode 1:LSB 2:USB 3:CW
  if( RADIO.mode == LSB)
  {
    buf[len ++] = '1';
  }
  else if(RADIO.mode == USB)
  {
    buf[len ++] = '2';
  }
  else
  {
    buf[len ++] = '3';
  }
  
  // P6 Data mode
  buf[len ++] = '0';
  
  // P7
  buf[len ++] = '0';
  
  // P8  Tone frequency
  buf[len ++] = '0';
  buf[len ++] = '0';
 
  // P9  CTCSS frequency
  buf[len ++] = '0';
  buf[len ++] = '0';
  
  // P10
  buf[len ++] = '0';
  buf[len ++] = '0';
  buf[len ++] = '0';
  
  // P11
  buf[len ++] = '0';
  
  // P12
  buf[len ++] = '0';
  
  // P13
  for(uint8_t i=0; i<9; i++)
  {
    buf[len ++] = '0';
  }
  
  // P14
  buf[len ++] = '0';
  buf[len ++] = '0';
  
  // P15
  buf[len ++] = '0';
  
  // P16
  for(uint8_t i=0; i<8; i++)
  {
    buf[len ++] = '0';
  }
  
  buf[len ++] = ';';
  
  UartTx(buf, len);
}

void Kenwood_VMSwithced(FREQ_ItemDef item)
{
  if ( RF_IsTx() || AIMgr.item & MENUSETItem || AIMgr.item & MEMSETItem)
  {
    return;
  }
  
  if (item & VFOAItem)
  {
    FREQ.item &= ~MEMItem;
    FREQ.item |= VFOItem;
    FREQ.item &= ~VFOBItem;
    FREQ.item |= VFOAItem;
    oled_vfo("VFO/A ");
    FREQ.numOfbaud = VFO.CurrBaudOfVfoA;
    VFO_Get();
  }
  else if(item & VFOBItem)
  {
    FREQ.item &= ~MEMItem;
    FREQ.item |= VFOItem;
    FREQ.item &= ~VFOAItem;
    FREQ.item |= VFOBItem;
    oled_vfo("VFO/B ");
    FREQ.numOfbaud = VFO.CurrBaudOfVfoB;
    VFO_Get();
  }
  else
  {
    FREQ.item &= ~VFOItem;
    FREQ.item |= MEMItem;
    MEM_Get();
  }

  DRMgr.item |= RADIOItem;  // 更新模式
  DRMgr.item |= FREQItem;   // 更新主频和波段
  DRMgr.item |= FILItem;
}

void Kenwood_VMAnswer()
{
  uint8_t buf[15];
  uint8_t len = 0;
  
  buf[ len++ ] = 'F';
  buf[ len++ ] = 'R';
  if (FREQ.item & VFOAItem)
  {
    buf[ len++ ] = '0';
  }
  else if(FREQ.item & VFOBItem)
  { 
    buf[ len++ ] = '1';
  }
  else
  {
    buf[ len++ ] = '2';
  }
  buf[ len++ ] = ';';
  UartTx(buf, len);
}

bool Kenwood_ModeSwithced(RADIO_ModeDef mode)
{
  if ((AIMgr.item & MENUSETItem) || RF_IsTx())
  {
    return false;
  }
  
  if( mode == CW )
  {
    if( CWMode == CWA )
    {
      RADIO.mode = CWA;
    }
    else
    {
      RADIO.mode = CWB;
      keyer_auto_setup();
    }
  }
  else
  {
    RADIO.mode = mode;
  }
  
  if( AIMgr.item & MEMSETItem)
  {
    DRMgr.item |= MEMSETItem;
  }
  else
  {
    DRMgr.item |= FREQItem;   // 更新主频
  }
  DRMgr.item |= RADIOItem;
  DRMgr.item |= FILItem;
  
  return true;
}

//RIT frequency down      BG6RDF
void Kenwood_RITSetDown(int32_t val)
{
  if ( RF_IsTx() || AIMgr.item & MENUSETItem || AIMgr.item & MEMSETItem)
  {
    return;
  }
  
  if( FREQ.item & RITItem )
  {
    VFO.RIT -= val;
    DRMgr.item |= FREQItem;
  }
}

//RIT frequency up      BG6RDF
void Kenwood_RITSetUp(int32_t val)
{
  if ( RF_IsTx() || AIMgr.item & MENUSETItem || AIMgr.item & MEMSETItem)
  {
    return;
  }
  
  if( FREQ.item & RITItem )
  {
    VFO.RIT += val;
    DRMgr.item |= FREQItem;
  }
}

//XIT frequency down      BG6RDF
void Kenwood_XITSetDown(int32_t val)
{
  if ( RF_IsTx() || AIMgr.item & MENUSETItem || AIMgr.item & MEMSETItem)
  {
    return;
  }
  
  if( FREQ.item & XITItem )
  {
    VFO.XIT -= val;
    DRMgr.item |= FREQItem;
  }
}

//XIT frequency up      BG6RDF
void Kenwood_XITSetUp(int32_t val)
{
  if ( RF_IsTx() || AIMgr.item & MENUSETItem || AIMgr.item & MEMSETItem)
  {
    return;
  }
  
  if( FREQ.item & XITItem )
  {
    VFO.XIT += val;
    DRMgr.item |= FREQItem;
  }
}

void Kenwodd_MemSet(uint8_t mem)
{
  if ((AIMgr.item & MENUSETItem) || (FREQ.item & MEMItem))
  {
    FREQ.numOfmem = mem; 
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
}

void Kenwood_VFOAAnswer()
{
  uint8_t buf[15];
  uint8_t len = 0;
  
  buf[ len++ ] = 'F';
  buf[ len++ ] = 'A';
  buf[ len++ ] = '0';
  buf[ len++ ] = '0';
  buf[ len++ ] = '0';
  DecToString(&buf[len], 7, (int32_t)VFO.VFOA[FREQ.numOfbaud].freq);
  len += 7;    
  buf[ len++ ] = '0';
  buf[ len++ ] = ';';
  UartTx(buf, len);
}

void Kenwood_VFOBAnswer()
{
  uint8_t buf[15];
  uint8_t len = 0;
  
  buf[ len++ ] = 'F';
  buf[ len++ ] = 'B';
  buf[ len++ ] = '0';
  buf[ len++ ] = '0';
  buf[ len++ ] = '0';
  DecToString(&buf[len], 7, (int32_t)VFO.VFOB[FREQ.numOfbaud].freq);
  len += 7;    
  buf[ len++ ] = '0';
  buf[ len++ ] = ';';
  UartTx(buf, len);
}

bool Kenwood_VFOSet(int32_t freq)
{
  uint8_t i;
  
  if ( RF_IsTx() || AIMgr.item & MENUSETItem || AIMgr.item & MEMSETItem)
  {
    return false;
  }
  
  for(i=0; i<SIZE_OF_BAUD; i++)
  {
    if ((freq <= BAUD[ i ][2]) && (freq >= BAUD[ i ][0]))
    {
      FREQ.numOfbaud = i;
      FREQ.val = freq;
      DRMgr.item |= FREQItem;
      return true;
    }
  }

  return false;
}

void Kenwood_AFGainSet(uint8_t vol)
{
  if ( RF_IsTx() || AIMgr.item & MENUSETItem || AIMgr.item & MEMSETItem)
  {
    return;
  }
  // 软件界面0-100：发送数据0-255
  // 音量0-60
  uint16_t tmp = vol * 100 / 255;
  if(tmp >= 60)
  {
    tmp = 60;
  }
  VOL.val = tmp;
  DRMgr.item |= VOLItem;
}

void Kenwood_AFGainAnswer()
{
  uint8_t buf[15];
  uint8_t len = 0;
  
  buf[ len++ ] = 'A';
  buf[ len++ ] = 'G';
  buf[ len++ ] = '0';
  uint16_t tmp = VOL.val * 255 / 100;
  DecToString(&buf[len], 3, (int32_t)tmp);
  len += 3;    
  buf[ len++ ] = ';';
  UartTx(buf, len);
}

void Kenwood_MemoryChannelAnswer()
{
  uint8_t buf[15];
  uint8_t len = 0;
  
  buf[ len++ ] = 'M';
  buf[ len++ ] = 'C';
  buf[ len++ ] = '1';
  DecToString(&buf[len], 2, (int32_t)FREQ.numOfmem);
  len += 2;
  buf[ len++ ] = ';';
  UartTx(buf, len);
}

void Kenwood_ModeStauteAnswer()
{
  uint8_t buf[15];
  uint8_t len = 0;
  
  buf[ len++ ] = 'M';
  buf[ len++ ] = 'D';
  DecToString(&buf[len], 1, (int32_t)RADIO.mode);
  len += 1;
  buf[ len++ ] = ';';
  UartTx(buf, len);
}

void Kenwodd_SMeterAnswer()
{
  uint8_t buf[15];
  uint8_t len = 0;
  
  buf[ len++ ] = 'S';
  buf[ len++ ] = 'M';
  buf[ len++ ] = '0';
  if (RF_IsRx())
  {
    DecToString(&buf[len], 4, (int32_t)rx_grid_old);
  }
  else
  {
    DecToString(&buf[len], 4, (int32_t)rf_grid_old);
  }
  len += 4;
  buf[ len++ ] = ';';
  UartTx(buf, len);
}

void Kenwood_SWRMeterAnswer()
{
  uint8_t buf[15];
  uint8_t len = 0;
  
  buf[ len++ ] = 'R';
  buf[ len++ ] = 'M';
  buf[ len++ ] = '1'; // 1: SWR
  DecToString(&buf[len], 4, (int32_t)swr_grid_old);
  len += 4;
  buf[ len++ ] = ';';
  UartTx(buf, len);
}

void Kenwodd_KeyingSpeedAnswer()
{
  uint8_t buf[15];
  uint8_t len = 0;
  
  buf[ len++ ] = 'K';
  buf[ len++ ] = 'S';
  DecToString(&buf[len], 3, (int32_t)WPM.val);
  len += 3;
  buf[ len++ ] = ';';
  UartTx(buf, len);
}
//len : 指令及数据, 末尾的分号不计算在len内    BG6RDF
void Kenwood_CmdParser(uint8_t *pBuf, uint8_t len)
{ 
  uint32_t tmp = 0;
  uint8_t buf[15];
  
  // 设置和读取电源开关状态
  if(pBuf[0] == 'P' && pBuf[1] == 'S')
  {
    if( len > 2)
    {
      // 0: Power OFF ; 1: Power ON ; 9: Power OFF (low current mode)
      UartTx("PS1;", 4);        //BG6RDF
    }
    else
    {
      UartTx("PS1;", 4);
    }
  }
  // 
  else if(pBuf[0] == 'A' && pBuf[1] == 'I')
  {
    if( len > 2)
    {
    
    }
    else
    {
      UartTx("AI0;", 4); // AI OFF
    }
  }
  // 读取发送器的状态
  else if(pBuf[0] == 'I' && pBuf[1] == 'F')
  {
    Kenwood_BuildTransceiverStatus();
  }
  // 设置频率波段
  else if((pBuf[0] == 'B' && pBuf[1] == 'D') || (pBuf[0] == 'B' && pBuf[1] == 'U'))
  {
    //Sets a frequency band.
    if (len > 2)
    {
      tmp = StringToDec(&pBuf[2], 2);
      Kenwood_BaudSet(tmp);
    }
  }
  else if(pBuf[0] == 'B' && pBuf[1] == 'Y')
  {
    // Reads the busy signal status.
    // 未使用
    if( len > 2)
    {
    
    }
    else
    {
      UartTx("BY10;", 5);       //BG6RDF
    }
  }
  else if(pBuf[0] == 'C' && pBuf[1] == 'H')
  {
   // 0: Move the MULTI/CH encoder 1 step up ;1: Move the MULTI/CH encoder 1 step down
   if (len > 2)
   {
     if(pBuf[2] == '1')
     {
     
     }
     else
     {
      
     }
   }
  }
  // 设置或读取VFOA/VFOB的频率
  else if(pBuf[0] == 'F' && pBuf[1] == 'A')
  {
    // Sets or reads the VFO A/ VFO B frequency
    if( len > 2)
    {
      tmp = StringToDec(&pBuf[2], 11);// Frequency (11 digits in Hz); eg.00014195000 for 14.195 MHz
      if (Kenwood_VFOSet((tmp / 10)) )
      {
         Kenwood_VFOAAnswer();
      }  
    }
    else
    {
      Kenwood_VFOAAnswer();
    }
  }
  else if(pBuf[0] == 'F' && pBuf[1] == 'B')
  {
    // Sets or reads the VFO A/ VFO B frequency
    if( len > 2)
    {
      tmp = StringToDec(&pBuf[2], 11);// Frequency (11 digits in Hz); eg.00014195000 for 14.195 MHz
      if (Kenwood_VFOSet((tmp / 10)) )
      {
         Kenwood_VFOBAnswer();
      } 
    }
    else
    {
      Kenwood_VFOBAnswer();
    }
  }
  // 选择或读取VFO 或 寄存器通道
  else if(pBuf[0] == 'F' && pBuf[1] == 'R')
  {
    // Selects or reads the VFO or Memory channel
    if (len > 2)
    {
      if (pBuf[2] == '0') // VFOA
      {
        Kenwood_VMSwithced(VFOAItem);
      }
      else if (pBuf[2] == '1')  // VFOB
      {
        Kenwood_VMSwithced(VFOBItem);
      }
      else  // MEM
      {
        Kenwood_VMSwithced(MEMItem);
      }
      
    }
    else
    {
      Kenwood_VMAnswer();
    }
  }
  else if(pBuf[0] == 'F' && pBuf[1] == 'T')
  {
    // Selects or reads the VFO or Memory channel
    // 未使用
    if( len > 2)
    {
    
    }
    else
    {
      UartTx("FT0;", 4); // VFO A
    }
  }
  else if(pBuf[0] == 'F' && pBuf[1] == 'L')
  {
    // 未使用
    if( len > 2)
    {
    
    }
    else
    {
      UartTx("FL1;", 4); // 1: IF Filter A
    }
  }
  else if(pBuf[0] == 'D' && pBuf[1] == 'A')
  {
    // 未使用
    if( len > 2)
    {
    
    }
    else
    {
      UartTx("DA0;", 4); // 0: DATA mode OFF
    }
  }
  else if(pBuf[0] == 'F' && pBuf[1] == 'S')
  {
    // 未使用
    if( len > 2)
    {
    
    }
    else
    {
      UartTx("FS0;", 4); // 0: Fine Tuning Funciton Off
    }
  }
  else if(pBuf[0] == 'N' && pBuf[1] == 'B')
  {
    // 未使用
    if( len > 2)
    {
    
    }
    else
    {
      UartTx("NB0;", 4); // 0: NB OFF
    }
  }
  else if(pBuf[0] == 'N' && pBuf[1] == 'R')
  {
    // 未使用
    if( len > 2)
    {
    
    }
    else
    {
      UartTx("NR0;", 4); // 0: NR OFF
    }
  }
  else if(pBuf[0] == 'P' && pBuf[1] == 'A')
  {
    // 未使用
    if( len > 2)
    {
    
    }
    else
    {
      UartTx("PA00;", 5); // 0: Pre-amp Off
    }
  }
  else if(pBuf[0] == 'P' && pBuf[1] == 'R')
  {
    // 未使用
    if( len > 2)
    {
    
    }
    else
    {
      UartTx("PR0;", 4); // 0: Speech OFF
    }
  }
  else if(pBuf[0] == 'R' && pBuf[1] == 'A')
  {
    // Sets and reads the RF Attenuator status. 未使用
    if( len > 2)
    {
    
    }
    else
    {
      UartTx("RA0000", 7);  // 00: ATT OFF
    }
  }
  else if(pBuf[0] == 'V' && pBuf[1] == 'X')
  {
    // 未使用
    if( len > 2)
    {
    
    }
    else
    {
      UartTx("VX0;", 4); // 0: VOX OFF
    }
  }
  else if(pBuf[0] == 'B' && pBuf[1] == 'C')
  {
    // 未使用
    if( len > 2)
    {
    
    }
    else
    {
      UartTx("BC0;", 4); // 0: Beat Cancel OFF
    }
  }
  else if(pBuf[0] == 'C' && pBuf[1] == 'A')
  {
    // 未使用
    if( len > 2)
    {
    
    }
    else
    {
      UartTx("CA0;", 4); // 0: Cancels CW TUNE/ Inactive
    }
  }
  else if(pBuf[0] == 'T' && pBuf[1] == 'O')
  {
    // 未使用
    if( len > 2)
    {
    
    }
    else
    {
      UartTx("TO0;", 4); // 0: Tone OFF
    }
  }
  else if(pBuf[0] == 'T' && pBuf[1] == 'N')
  {
    // Sets and reads the Tone frequency. 未使用
    if( len > 2)
    {
    
    }
    else
    {
      UartTx("TN00;", 5); // 
    }
  }
  else if(pBuf[0] == 'C' && pBuf[1] == 'T')
  {
    // 未使用
    if( len > 2)
    {
    
    }
    else
    {
      UartTx("CT0;", 4); // 0: CTCSS OFF
    }
  }
  else if(pBuf[0] == 'C' && pBuf[1] == 'N')
  {
    // Sets and reads the CTCSS frequency 未使用
    if( len > 2)
    {
    
    }
    else
    {
      UartTx("CN00;", 5); // 0: CTCSS OFF
    }
  }
  else if(pBuf[0] == 'T' && pBuf[1] == 'S') 
  {
    // Sets and reads the TF-Set status
    if( len > 2 )
    {
//      if( pBuf[2] == '1') // 0: TF-Set OFF ; 1: TF-Set ON
//      {
//        KEY_SoftwareSet(5, keyLong);
//      }
//      else
//      {
//        KEY_SoftwareSet(5, keyShot);
//      } 
    }
    else
    {
      buf[0] = 'T';
      buf[1] = 'S';
//      if( FREQ.item & TSItem)
//      {
//        buf[2] = '1';
//      }
//      else
//      {
//        buf[2] = '0';
//      }
      buf[2] = '0';
      buf[3] = ';';
      UartTx(buf, 4);
    }
  }
  else if(pBuf[0] == 'M' && pBuf[1] == 'F')
  {
    // 未使用
    if( len > 2)
    {
    
    }
    else
    {
      UartTx("MF0;", 4); // 0: Menu A
    }
  }
  else if(pBuf[0] == 'A' && pBuf[1] == 'C')
  {
    // 未使用
    if( len > 2)
    {
    
    }
    else
    {
      UartTx("AC000;", 6); // 0:
    }
  }
  else if(pBuf[0] == 'N' && pBuf[1] == 'T')
  {
    // 未使用
    if( len > 2)
    {
    
    }
    else
    {
      UartTx("NT00;", 5); // 0: Notch OFF  0: Normal
    }
  }
  // 音量控制
  else if(pBuf[0] == 'A' && pBuf[1] == 'G')
  {
    // Sets or reads the AF gain
    if( len > 3)
    {
      tmp = StringToDec(&pBuf[3], 3);
      Kenwood_AFGainSet(tmp);
    }
    else
    {
      Kenwood_AFGainAnswer(); // 
    }
  }
  else if(pBuf[0] == 'R' && pBuf[1] == 'G')
  {
    // 未使用
    if( len > 2)
    {
    
    }
    else
    {
      UartTx("RG000;", 6); // 
    }
  }
  else if(pBuf[0] == 'G' && pBuf[1] == 'C')
  {
    // Sets or reads the AGC 未使用
    if( len > 2)
    {
    
    }
    else
    {
      UartTx("GC0;", 4); // 
    }
  }
  else if(pBuf[0] == 'P' && pBuf[1] == 'C')
  {
    // 未使用
    if( len > 2)
    {
    
    }
    else
    {
      UartTx("PC005;", 6); // 005 ~ 100: SSB/ CW/ FM/ FSK 
    }
  }
  else if(pBuf[0] == 'M' && pBuf[1] == 'G')
  {
    // 未使用
    if( len > 2)
    {
    
    }
    else
    {
      UartTx("MG000;", 6); // The microphone gain
    }
  }
  // 读取寄存器通道的数据
  else if(pBuf[0] == 'M' && pBuf[1] == 'R')
  {
    // Reads the Memory channel data.
    if( pBuf[2] = '0' && pBuf[3] == '1')
    {
      Kenwood_MemoryDataAnswer();
    }
  }
  else if(pBuf[0] == 'N' && pBuf[1] == 'L')
  {
    // 未使用
    if( len > 2)
    {
    
    }
    else
    {
      UartTx("NL001;", 6); // 001 ~ 010
    }
  }
  else if(pBuf[0] == 'V' && pBuf[1] == 'G')
  {
    // 未使用
    if( len > 2)
    {
    
    }
    else
    {
      UartTx("VG000;", 6); // the VOX Gain 
    }
  }
  else if(pBuf[0] == 'V' && pBuf[1] == 'D')
  {
    // 未使用
    if( len > 2)
    {
    
    }
    else
    {
      UartTx("VD0000;", 7); // the VOX Gain 
    }
  }
   // 读取RX和TX的电平指示格子
  else if(pBuf[0] == 'S' && pBuf[1] == 'M')
  {
    // Reads the S-meter value. 
    // The SM command reads the S-meter during reception and the RF (power) meter during transmission.
    if( pBuf[2] == '0' )
    {
      Kenwodd_SMeterAnswer(); // 0000 ~ 0030: S-meter value
    }
  }
  else if(pBuf[0] == 'S' && pBuf[1] == 'Q')
  {
    // 未使用
    if( len > 3)
    {
    
    }
    else
    {
      UartTx("SQ0000;", 7); // the squelch value
    }
  }
  else if(pBuf[0] == 'Q' && pBuf[1] == 'R')
  {
    // 未使用
    if( len > 3)
    {
    
    }
    else
    {
      UartTx("QR00;", 5); // Quick Memory channel data
    }
  }
  else if(pBuf[0] == 'I' && pBuf[1] == 'S')
  {
    // Sets and reads the DSP Filter Shift 未使用
    if( len > 2)
    {
    
    }
    else
    {
      UartTx("IS 0000;", 8); // 
    }
  }
  // 设置和读取电键键速
  else if(pBuf[0] == 'K' && pBuf[1] == 'S')
  {
    // Sets and reads the Keying speed
    if( len > 2 )
    {
      tmp = StringToDec(&pBuf[2], 3); // 004 ~ 060 (in steps of 1)
      WPM.val = tmp;
    }
    else
    {
      Kenwodd_KeyingSpeedAnswer();
    }
  }
  else if(pBuf[0] == 'S' && pBuf[1] == 'D')
  {
    // Sets and reads the CW break-in time delay. 未使用
    if( len > 2)
    {
    
    }
    else
    {
      UartTx("SD0000;", 7); // 
    }
  }
  else if(pBuf[0] == 'A' && pBuf[1] == 'N')
  {
    // Selects the antenna connector ANT1/ ANT2 未使用
    if( len > 2)
    {
    
    }
    else
    {
      UartTx("AN000;", 6); // 
    }
  }
  else if(pBuf[0] == 'K' && pBuf[1] == 'Y')
  {
    // Converts the entered characters into morse code while keying.
    if( len == 2 )
    {
      buf[0] = 'K';
      buf[1] = 'Y';
      buf[1] = '0'; //0: Character buffer space
      buf[5] = ';';
      UartTx(buf, 4);
    }
    else if( len > 3 )
    {
      
    }
  }
  // 设置和读取寄存器通道号
  else if(pBuf[0] == 'M' && pBuf[1] == 'C')
  {
    // Sets and reads the Memory Channel number
    if( len > 2 )
    {
      if( pBuf[1] == '1' )
      {
        tmp = StringToDec(&pBuf[2], 2); //00 ~ 99: Two digit channel number
        Kenwodd_MemSet(tmp);
      }
    }
    else
    {
      Kenwood_MemoryChannelAnswer();
    }
  }
  // 设置和读取操作模式
  else if(pBuf[0] == 'M' && pBuf[1] == 'D')
  {
    // Sets and reads the operating mode status.
    if( len > 2 )
    {
      if (Kenwood_ModeSwithced((RADIO_ModeDef)(pBuf[2] - '0' )))
      {
        Kenwood_ModeStauteAnswer();
      }
    }
    else
    {
      Kenwood_ModeStauteAnswer();
    }
  }
  else if(pBuf[0] == 'R' && pBuf[1] == 'C')
  {
    // Clears the RIT/XIT frequency. -注：处于RIT/XIT模式时，起作用
    if (FREQ.item & RITItem)    //BG6RDF
      VFO.RIT=0;
    if (FREQ.item & XITItem)
      VFO.XIT=0;
    DRMgr.item |= FREQItem;
  }
  // 读取
  else if(pBuf[0] == 'R' && pBuf[1] == 'M')
  {
    // Sets and reads the Meter function.
    if( len > 2 )
    {
    }
    else
    {
      Kenwood_SWRMeterAnswer(); // 0000 ~ 0030: Meter value in dots
    }
  }
  else if(pBuf[0] == 'R' && pBuf[1] == 'D')
  {
    // Sets and reads the RIT/XIT frequency Down
    if( len > 2 )
    {
      tmp = StringToDec(&pBuf[2], 5); // 00000 ~ 99999: Frequency (in Hz)
      if (FREQ.item & RITItem)
        Kenwood_RITSetDown(tmp);      //BG6RDF
      if (FREQ.item & XITItem)
        Kenwood_XITSetDown(tmp);
    }
    else
    {
      buf[0] = 'R';
      buf[1] = 'D';
      buf[2] = '1';
      buf[3] = ';';
      UartTx(buf, 4);           //BG6RDF
    }
  }
  else if(pBuf[0] == 'R' && pBuf[1] == 'U')
  {
    // Sets and reads the RIT/XIT frequency Up
    if( len > 2 )
    {
      tmp = StringToDec(&pBuf[2], 5); // 00000 ~ 99999: Frequency (in Hz)
      if (FREQ.item & RITItem)
        Kenwood_RITSetUp(tmp);      //BG6RDF
      if (FREQ.item & XITItem)
        Kenwood_XITSetUp(tmp);
    }
    else
    {
      buf[0] = 'R';
      buf[1] = 'U';
      buf[2] = '1';
      buf[3] = ';';
      UartTx(buf, 4);           //BG6RDF
    }
  }
  else if(pBuf[0] == 'R' && pBuf[1] == 'T') 
  {
    // Sets and reads the RIT function status.
    if( len > 2 )
    {
      if( pBuf[2] == '1') // 0: RIT OFF ; 1: RIT ON
      {
        KEY_SoftwareSet(6, keyLong);
      }
      else
      {
        KEY_SoftwareSet(6, keyShot);
      }
    }
    else
    {
      buf[0] = 'R';
      buf[1] = 'T';
      if( FREQ.item & RITItem)
      {
        buf[2] = '1';
      }
      else
      {
        buf[2] = '0';
      }
      buf[3] = ';';
      UartTx(buf, 4);
    }
  }
  else if(pBuf[0] == 'R' && pBuf[1] == 'X') 
  {
    // Sets the receiver function status.
    if( RADIO.mode == USB || RADIO.mode == LSB )
    {
      RF_ToRx();
    }
  }
  else if(pBuf[0] == 'T' && pBuf[1] == 'X') 
  {
    // Sets the transmission mode.
    if (pBuf[2] == '0')
    {
      if( RADIO.mode == USB || RADIO.mode == LSB )
      {
        RF_ToTx();
      }
    }
  }
  else if(pBuf[0] == 'X' && pBuf[1] == 'T')     //BG6RDF
  {
    // Sets and reads the XIT function status.
    if( len > 2 )
    {
      if( pBuf[2] == '1') // 0: XIT OFF ; 1: XIT ON
      {
        if((FREQ.item & VFOItem) && (AIMgr.item <= 0) && !(FREQ.item & XITItem))
          KEY_SoftwareSet(5, keyShot);          //进入XIT
      }
      else
      {
        if((FREQ.item & VFOItem) && (AIMgr.item <= 0) && (FREQ.item & XITItem))
          KEY_SoftwareSet(5, keyShot);          //退出XIT
      }
    }
    else
    {
      buf[0] = 'X';
      buf[1] = 'T';
      if( FREQ.item & XITItem)
      {
        buf[2] = '1';
      }
      else
      {
        buf[2] = '0';
      }
      buf[3] = ';';
      UartTx(buf, 4);
    }
  }
  
}