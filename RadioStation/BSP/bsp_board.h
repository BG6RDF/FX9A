#ifndef __BSP_BOARD_H__
#define __BSP_BOARD_H__

#include <stdint.h>
#include <stdbool.h>
#include <string.h> 
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include "stm32f10x_conf.h"

#include "bsp_delay.h"

#define SIZE_OF_BAUD 10
#define SIZE_OF_MEM 100
#define SIZE_OF_MENU 19         //BG6RDF 这个歌值是菜单数量+呼号字符数数量

#define UPPER_OF_RIT 9990       //取值范围参照Kenwood控制指令     BG6RDF
#define LOWER_OF_RIT -9990
#define UPPER_OF_XIT 9990      //BG6RDF
#define LOWER_OF_XIT -9990


typedef void (*CallBack)(void);
typedef void (*CallBackArg)(uint32_t arg);

typedef enum{
  VFOAItem  = 0x01,
  VFOBItem  = 0x02,
  MEMItem   = 0x04,
  RITItem   = 0x08,
  TSItem    = 0x10,
  VFOItem   = 0x20,
  CALLItem  = 0x40,
  XITItem   = 0x80,             //BG6RDF
  NULLItem  = 0xff
}FREQ_ItemDef;

typedef struct{
  uint32_t val;
  uint8_t *str;
  FREQ_ItemDef item;
  uint8_t numOfbaud;
  uint8_t numOfmem;
}FREQ_AttrDef;

typedef enum{
  LSB = 1,
  USB = 2,
  CW = 3,
  CWA = 4,
  CWB = 5
}RADIO_ModeDef;

typedef struct{
  RADIO_ModeDef mode;
}RADIO_AttrDef;


#pragma pack(1)//设定为1字节对齐
typedef struct{
  uint32_t freq;
  RADIO_ModeDef mode;
}VFO_SaveInfo;
#pragma pack()

//#pragma pack(push) //保存对齐状态
//#pragma pack(1)//设定为1字节对齐
typedef struct{
  int32_t RIT;
  int32_t XIT;
  VFO_SaveInfo VFOA[SIZE_OF_BAUD];
  VFO_SaveInfo VFOB[SIZE_OF_BAUD];
  uint8_t CurrBaudOfVfoA;
  uint8_t CurrBaudOfVfoB;
  
}VFO_SaveDef;
//#pragma pack()
//#pragma pack(pop)//恢复对齐状态


typedef enum{
  H,
  M,
  L,
}TP_LevelDef;
typedef struct{
  uint8_t *val;
  TP_LevelDef level;
}TP_AttrDef;

typedef enum{
  W,
  N,
}FIL_FuncDef;
typedef struct{
  uint8_t *val;
  FIL_FuncDef func;
}FIL_AttrDef;

typedef struct{
  uint8_t val;
  
}VOL_AttrDef;

typedef struct{
  uint8_t val;
  
}ATT_AttrDef;


typedef struct{
  uint16_t val;
  
}DC_AttrDef;

typedef struct{
  uint16_t val;
}WPM_AttrDef;

//#pragma pack(push) //保存对齐状态
#pragma pack(1)//设定为1字节对齐
typedef struct{
  uint32_t freq;
  uint8_t baud;
  RADIO_ModeDef mode;
}MEM_AttrDef;
#pragma pack()
//#pragma pack(pop)//恢复对齐状态

typedef enum{
  AUTO = 0,
  MANUAL = 1,
}CW_ModeDef;
typedef struct{
  uint8_t ascii;
  uint8_t index;
}CALL_Def;

//最大呼号长度
#define MAX_CALLSIGN_LENGTH 10
//CW窄带模式下本振偏移量，用于补偿晶体滤波器中心频率偏移
#define MAX_CWN_LO_OFFSET 9999

//#pragma pack(push) //保存对齐状态
#pragma pack(1)//设定为1字节对齐
typedef struct{
  uint8_t numOfmenu;
  uint8_t key;  // 0: MODE0  1:MODE1
  uint8_t delay;  // 0.1~1.5S 
  uint8_t cwMode; // 0:AUTO  1:MANUAL
  uint8_t vswr; //
  uint8_t reserved[8];
  uint8_t null;
  uint8_t numOfcall;
  uint8_t dutyOfbuzzer; // 0-29         侧音音量
  uint16_t freqOfbuzzer; // 500-1000Hz
  
  uint16_t null1;
  uint32_t rbfo;
//  uint32_t tbfo;
//  uint32_t usbbfo;
//  uint32_t lsbbfo;
  uint32_t ref;
  uint32_t null2;
  uint8_t  callSign[MAX_CALLSIGN_LENGTH];
  int16_t cwnloOffset;   //CW窄带模式下本振偏移量，范围±9999Hz        BG6RDF
}MENU_AttrDef;
#pragma pack()
//#pragma pack(pop)//恢复对齐状态

typedef enum{
  RADIOItem = 0x00000001,
  VOLItem   = 0x00000002,
  ATTItem   = 0x00000004,
  TPItem    = 0x00000010,
  FILItem   = 0x00000020,
  DCItem    = 0x00000080,
  FREQItem  = 0x00000100,
  TXRXItem  = 0x00000200,
  SWRCHKItem = 0x00000400,
  RFCHKItem  = 0x00000800,
  RXCHKItem  = 0x00001000,
  ALLItem   = 0x00800000,
  MEMSETItem = 0x01000000,
  WPMSETItem = 0x02000000,
  MENUSETItem = 0x04000000,
}DispRefreshItem;
typedef struct{
  DispRefreshItem item;
}DispRefreshMgr;

// 处于设置菜单项时，需要控制其他按键失效，又为退出设置状态提供依据
typedef struct{
  uint32_t item;
}AdjustItemMgr;

extern DispRefreshMgr DRMgr;
extern AdjustItemMgr AIMgr;
extern FREQ_AttrDef FREQ;
extern VFO_SaveDef VFO;
extern VOL_AttrDef VOL;
extern WPM_AttrDef WPM;
extern DC_AttrDef DC;
extern MENU_AttrDef MENU;
extern ATT_AttrDef ATT;

extern uint32_t BAUD[SIZE_OF_BAUD][3];
extern uint8_t CALL_BUF[];
extern MEM_AttrDef MEM[SIZE_OF_MEM];
extern RADIO_AttrDef RADIO;
extern RADIO_ModeDef CWMode;

extern uint8_t rx_grid_old;
extern uint8_t swr_grid_old;
extern uint8_t rf_grid_old;

void board_init();
void board_task();

void VFO_Get();
void VFO_Save();
void MEM_Get();
void MEM_Save();
void MENU_SetUp(uint8_t pos);
void MENU_SetDown(uint8_t pos);
void RF_Tx();
void RF_Rx();
bool RF_IsTx();
bool RF_IsRx();
void RF_ToRx();
void RF_ToTx();
#endif