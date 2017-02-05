#ifndef __BSP_24LC16_H__
#define __BSP_24LC16_H__

#include "bsp_board.h"
#include "i2c.h"

// 16K / 8 = 2KBytes
#define E2P_CTLCODE		0xA0
#define E2P_INS_WRITE	        0x00
#define E2P_INS_READ	        0x01

#define E2P_WORDH_ADDR_BM	0x0f
#define E2P_WORDL_ADDR_BM	0xff

#define E2P_ADDR_BASE   0
#define E2P_ADDR_MEM    E2P_ADDR_BASE + 0     // 100 * 8
#define E2P_ADDR_VFOA   E2P_ADDR_BASE + 800   // 8 * 10 = 80
#define E2P_ADDR_VFOB   E2P_ADDR_BASE + 880   // 8 * 10 = 80
#define E2P_ADDR_A_BAUD E2P_ADDR_BASE + 960  
#define E2P_ADDR_B_BAUD E2P_ADDR_BASE + 968
#define E2P_ADDR_RIT    E2P_ADDR_BASE + 976   // 4字节    BG6RDF
#define E2P_ADDR_XIT    E2P_ADDR_BASE + 980   // 4字节    BG6RDF

#define E2P_ADDR_FIL    E2P_ADDR_BASE + 984   // 1
#define E2P_ADDR_VOL    E2P_ADDR_BASE + 992   // 1
#define E2P_ADDR_ATT    E2P_ADDR_BASE + 1000   // 1
#define E2P_ADDR_PWM    E2P_ADDR_BASE + 1008   // 1

#define E2P_ADDR_MENU   E2P_ADDR_BASE + 1024   // 28 -不包括CALL号, BG6RDF:24+8(RESERVED)+10(CALLSIGN)

#define E2P_ADDR_SAVE   E2P_ADDR_BASE + 1600

/**********************************************
*	Global function
**/
uint8_t e2p_write_byte(uint16_t addr, uint8_t wdata);
uint8_t e2p_read_byte(uint16_t addr);

bool e2p_read(uint16_t addr, uint8_t *rdata, uint16_t len);
bool e2p_write(uint16_t addr, uint8_t *data, uint16_t len);

void e2p_mem_write(uint8_t numOfmem, MEM_AttrDef *pMem, uint8_t len);
void e2p_mem_read(uint8_t numOfmem, MEM_AttrDef *pMem, uint8_t len);

void e2p_vfo_write(VFO_SaveDef *vfo);
VFO_SaveDef e2p_vfo_read();

void e2p_rit_write(int16_t rit);
int16_t e2p_rit_read();

void e2p_fil_write(FIL_FuncDef filter);
FIL_FuncDef e2p_fil_read();

void e2p_vol_write(uint8_t vol);
uint8_t e2p_vol_read();

void e2p_att_write(uint8_t att);
uint8_t e2p_att_read();

void e2p_pwm_write(uint8_t pwm);
uint8_t e2p_pwm_read();

void e2p_call_write(uint8_t *call);
void e2p_call_read(uint8_t *call);

void e2p_menu_write(MENU_AttrDef *menu);
MENU_AttrDef e2p_menu_read();

void e2p_save_write(uint8_t save);
uint8_t e2p_save_read();

void e2p_reset();
#endif
