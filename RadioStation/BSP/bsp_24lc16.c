#include "bsp_24lc16.h"

// 接24C16储存器 PE2/1-7/WC   PE3/2-6/SCL   PE4/3-5/SDA

void e2p_reset()
{
  i2c_start();
  i2c_reset();
  i2c_start();
  i2c_stop();
}

uint8_t e2p_write_byte(uint16_t addr, uint8_t wdata)
{
	uint8_t ccode;
	uint8_t wordl;
        uint8_t wordh;
	
        wordh = (addr >> 8) & 0x07; 
	ccode = E2P_CTLCODE | (wordh << 1) | E2P_INS_WRITE;
	wordl = addr & 0xff;
        
	i2c_start();
	
	i2c_tx(ccode);	
	if( i2c_getack() == NOACK )
	{
          return false;
        }
	
	i2c_tx(wordl);
	if( i2c_getack() == NOACK )
	{
          return false;
        }
	
	i2c_tx(wdata);
	if( i2c_getack() == NOACK )
        {
          return false;
        }
	  	
	i2c_stop();
	delay_ms(8);//写时序的有效停止信号到内部编程/擦除周期结束的这一段时间
	
	return true;
}

// len : 0~8
uint8_t e2p_write_mulbyte(uint16_t addr, uint8_t *wdata, uint8_t len)
{
	uint8_t ccode;
        uint8_t wordh;
	uint8_t wordl;
	
        //SET_E2P_MODE(1);
        wordh = (addr >> 8) & 0x07;
	ccode = E2P_CTLCODE | wordh << 1 | E2P_INS_WRITE;
	wordl = addr & 0xff;
        
	i2c_start();
	
	i2c_tx(ccode);	
	if( i2c_getack() == NOACK )
	{
          return false;
        }
	
	i2c_tx(wordl);
	if( i2c_getack() == NOACK )
	{
          return false;
        }
	
        for(uint8_t i=0; i<len; i++)
        {
          i2c_tx(wdata[i]);
          if( i2c_getack() == NOACK )
          {
            return false;
          }
        } 	
	i2c_stop();
	delay_ms(8);//写时序的有效停止信号到内部编程/擦除周期结束的这一段时间
	
	return true;
}

bool e2p_write(uint16_t addr, uint8_t *data, uint16_t len)
{
  //uint16_t tlen = (len / 9) + 1;
  uint16_t tlen = (len + 7) / 8;               //BG6RDF
  bool ret = true;
  
  for(uint8_t j=0; j<tlen; j++)
  {
    if( len <= 8)
    {
      ret = e2p_write_mulbyte(addr, &data[j*8], len);
    }
    else
    {
      ret = e2p_write_mulbyte(addr, &data[j*8], 8);
      len -= 8;
      addr += 8;
    }
    
  }
  return ret;
}

uint8_t e2p_read_byte(uint16_t addr)
{
	uint8_t ccode;
	uint8_t wordl;
        uint8_t wordh;
	uint8_t tmp;
	
        wordh = (addr >> 8) & 0x07;
	ccode = E2P_CTLCODE | (wordh << 1) | E2P_INS_WRITE;
        wordl = addr & 0xff;
	
	i2c_start();
	
	i2c_tx(ccode);	
	if( i2c_getack() == NOACK )
	  	return false;
	
	i2c_tx(wordl);
	if( i2c_getack() == NOACK )
	  	return false;
	
	//ccode = E2P_CTLCODE | (wordh << 1) | E2P_INS_READ;
        ccode = E2P_CTLCODE | E2P_INS_READ;
	i2c_start();
	i2c_tx(ccode);	
	if( i2c_getack() == NOACK )
	  	return false;
	
	tmp = i2c_rx();
	i2c_noack();
	i2c_stop();
	
	return tmp;
}

bool e2p_read(uint16_t addr, uint8_t *rdata, uint16_t len)
{
      uint8_t ccode;
      uint8_t wordh;
	uint8_t wordl;
	
        //SET_E2P_MODE(0);
        wordh = (addr >> 8) & 0x07;
	ccode = E2P_CTLCODE | wordh <<1  | E2P_INS_WRITE;
        wordl = addr & 0xff;
	
	i2c_start();
	
	i2c_tx(ccode);	
	if( i2c_getack() == NOACK )
	  	return false;
	
	
	i2c_tx(wordl);
	if( i2c_getack() == NOACK )
	  	return false;
	
	//ccode = E2P_CTLCODE | wordh << 1 | E2P_INS_READ;
        ccode = E2P_CTLCODE | E2P_INS_READ;
	i2c_start();
	i2c_tx(ccode);	
	if( i2c_getack() == NOACK )
	  	return false;
	
        for(uint16_t i=0; i<len-1; i++)
        {
	  rdata[i] = i2c_rx();
          i2c_ack();
        }
        rdata[len-1] = i2c_rx();
	i2c_noack();
	i2c_stop();
        
        return true;
}


void e2p_mem_write(uint8_t numOfmem, MEM_AttrDef *pMem, uint8_t len)
{
  uint8_t step = 8;
  uint8_t base = step * numOfmem;
  for(uint8_t i=0; i<len; i++)
  {
    e2p_write(E2P_ADDR_MEM+base+i*step, (uint8_t *)&pMem[i], sizeof(MEM_AttrDef));
  }
}

void e2p_mem_read(uint8_t numOfmem, MEM_AttrDef *pMem, uint8_t len)
{
  uint8_t step = 8;
  uint8_t base = step * numOfmem;
  for(uint8_t i=0; i<len; i++)
  {
    e2p_read(E2P_ADDR_MEM+base+i*step, (uint8_t *)&pMem[i], sizeof(MEM_AttrDef));
  }
}

void e2p_vfo_write(VFO_SaveDef *vfo)
{
  uint8_t step = 8;
  for(uint8_t i=0; i<SIZE_OF_BAUD; i++)
  {
    e2p_write(E2P_ADDR_VFOA+step*i, (uint8_t *)&vfo->VFOA[i], sizeof(VFO_SaveInfo));
    e2p_write(E2P_ADDR_VFOB+step*i, (uint8_t *)&vfo->VFOB[i], sizeof(VFO_SaveInfo));
  }
  e2p_write(E2P_ADDR_A_BAUD, &vfo->CurrBaudOfVfoA, sizeof(int8_t));
  e2p_write(E2P_ADDR_B_BAUD, &vfo->CurrBaudOfVfoB, sizeof(int8_t));
  e2p_write(E2P_ADDR_RIT, (uint8_t *)&(vfo->RIT), 4);
  e2p_write(E2P_ADDR_XIT, (uint8_t *)&(vfo->XIT), 4);   //BG6RDF
}

VFO_SaveDef e2p_vfo_read()
{
  VFO_SaveDef vfo;
  uint8_t step = 8;
  for(uint8_t i=0; i<SIZE_OF_BAUD; i++)
  {
    e2p_read(E2P_ADDR_VFOA+step*i, (uint8_t *)&vfo.VFOA[i], sizeof(VFO_SaveInfo));
    e2p_read(E2P_ADDR_VFOB+step*i, (uint8_t *)&vfo.VFOB[i], sizeof(VFO_SaveInfo));
  }
  e2p_read(E2P_ADDR_A_BAUD, &vfo.CurrBaudOfVfoA, sizeof(int8_t));
  e2p_read(E2P_ADDR_B_BAUD, &vfo.CurrBaudOfVfoB, sizeof(int8_t));
  e2p_read(E2P_ADDR_RIT, (uint8_t *)&(vfo.RIT), 4);
  e2p_read(E2P_ADDR_XIT, (uint8_t *)&(vfo.XIT), 4);     //BG6RDF
  return vfo;
}

void e2p_rit_write(int16_t rit)
{
  e2p_write(E2P_ADDR_RIT, (uint8_t *)&rit, sizeof(int16_t));
}

int16_t e2p_rit_read()
{
  int16_t rit;
  e2p_read(E2P_ADDR_RIT, (uint8_t *)&rit, sizeof(int16_t));
  return rit;
}

void e2p_fil_write(FIL_FuncDef filter)
{
  e2p_write(E2P_ADDR_FIL, (uint8_t *)&filter, sizeof(uint8_t));
}

FIL_FuncDef e2p_fil_read()
{
  FIL_FuncDef filter;
  e2p_read(E2P_ADDR_FIL, (uint8_t *)&filter, sizeof(uint8_t));
  return filter;
}

void e2p_vol_write(uint8_t vol)
{
  e2p_write(E2P_ADDR_VOL, (uint8_t *)&vol, sizeof(uint8_t));
}

uint8_t e2p_vol_read()
{
  uint8_t vol;
  e2p_read(E2P_ADDR_VOL, (uint8_t *)&vol, sizeof(uint8_t));
  return vol;
}

void e2p_att_write(uint8_t att)
{
  e2p_write(E2P_ADDR_ATT, (uint8_t *)&att, sizeof(uint8_t));
}

uint8_t e2p_att_read()
{
  uint8_t att;
  e2p_read(E2P_ADDR_ATT, (uint8_t *)&att, sizeof(uint8_t));
  return att;
}

void e2p_pwm_write(uint8_t pwm)
{
  e2p_write(E2P_ADDR_PWM, (uint8_t *)&pwm, sizeof(uint8_t));
}

uint8_t e2p_pwm_read()
{
  uint8_t pwm;
  e2p_read(E2P_ADDR_PWM, (uint8_t *)&pwm, sizeof(uint8_t));
  return pwm;
}

//void e2p_call_write(uint8_t *call)
//{
//  e2p_write(E2P_ADDR_CALL, (uint8_t *)&call, 8);
//}
//
//void e2p_call_read(uint8_t *call)
//{
//  e2p_read(E2P_ADDR_CALL, (uint8_t *)&call, 8);
//}

void e2p_menu_write(MENU_AttrDef *menu)
{
  e2p_write(E2P_ADDR_MENU, (uint8_t *)&(*menu), sizeof(MENU_AttrDef));
}

MENU_AttrDef e2p_menu_read()
{
  MENU_AttrDef menu;
  e2p_read(E2P_ADDR_MENU, (uint8_t *)&menu, sizeof(MENU_AttrDef));
  return menu;
}

void e2p_save_write(uint8_t save)
{
  e2p_write(E2P_ADDR_SAVE, (uint8_t *)&save, 1);
}

uint8_t e2p_save_read()
{
  uint8_t save;
  e2p_read(E2P_ADDR_SAVE, (uint8_t *)&save, 1);
  return save;
}