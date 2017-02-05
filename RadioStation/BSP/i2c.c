#include "i2c.h"

//----- ST24C16 I2C configure -------


#define GET_I2C_SDA_PIN() 	( GPIO_ReadInputDataBit(I2C_PORT, I2C_SDA) )

#define I2C_DELAY_US 15

void SET_E2P_MODE(uint8_t bit)
{
  if( bit )
  {
    GPIO_SetBits(I2C_PORT, E2P_WC);
  }
  else
  {
    GPIO_ResetBits(I2C_PORT, E2P_WC);
  }
}
void SET_I2C_SCK_PIN(uint8_t bit)
{
  if( bit )
  {
    GPIO_SetBits(I2C_PORT, I2C_SCK);
  }
  else
  {
    GPIO_ResetBits(I2C_PORT, I2C_SCK);
  }
}

void SET_I2C_SDA_PIN(uint8_t bit)
{
  if( bit )
  {
    GPIO_SetBits(I2C_PORT, I2C_SDA);
  }
  else
  {
    GPIO_ResetBits(I2C_PORT, I2C_SDA);
  }
}

void SET_I2C_SDA_DIR(uint8_t out)
{
  GPIO_InitTypeDef GPIO_InitStructure;
  
  GPIO_InitStructure.GPIO_Pin =  I2C_SDA;
  if( out )
  {
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
  }
  else
  {
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  }
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_Init(I2C_PORT, &GPIO_InitStructure);
}

void i2c_reset()
{
  SET_E2P_MODE(0);
  SET_I2C_SDA_PIN(1);
  SET_I2C_SCK_PIN(0);
  delay_us(I2C_DELAY_US); // 钳住总线准备发数据
  
  for(uint8_t i=0; i<9; i++)
  {
    SET_I2C_SCK_PIN(1);
    delay_us(I2C_DELAY_US); // 钳住总线准备发数据
    SET_I2C_SCK_PIN(0);
    delay_us(I2C_DELAY_US); // 钳住总线准备发数据
  }
}

void i2c_init(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;
  
  GPIO_InitStructure.GPIO_Pin =  E2P_WC;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_Init(I2C_PORT, &GPIO_InitStructure);
  
  GPIO_InitStructure.GPIO_Pin =  I2C_SDA | I2C_SCK;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_Init(I2C_PORT, &GPIO_InitStructure);
  
  GPIO_SetBits(I2C_PORT, E2P_WC);
  
  /*I2C总线初始为高电平*/ 
  GPIO_SetBits(I2C_PORT, I2C_SDA);
  GPIO_SetBits(I2C_PORT, I2C_SCK);
}

void i2c_start()
{
  SET_I2C_SCK_PIN(1);
  SET_I2C_SDA_PIN(1);
  delay_us(I2C_DELAY_US); // 起始条件建立时间大于4.7us 
  
  SET_I2C_SDA_PIN(0);
  delay_us(I2C_DELAY_US); // 起始条件锁定时间大于4us 
  
  SET_I2C_SCK_PIN(0);
  delay_us(I2C_DELAY_US); // 钳住总线准备发数据
}

void i2c_stop()
{
  SET_I2C_SDA_PIN(0);
  SET_I2C_SCK_PIN(1);     // 发送总线时钟信号
  delay_us(I2C_DELAY_US); // 结束总线时间大于4us 
  
  SET_I2C_SDA_PIN(1);
  delay_us(I2C_DELAY_US); // 结束总线
}

void i2c_ack()
{
  SET_I2C_SDA_PIN(0);
  SET_I2C_SCK_PIN(1);
  delay_us(I2C_DELAY_US);
  
  SET_I2C_SCK_PIN(0);
  SET_I2C_SDA_PIN(1);
  delay_us(I2C_DELAY_US);
}

void i2c_noack()
{

  SET_I2C_SDA_PIN(1);
  SET_I2C_SCK_PIN(1);
  delay_us(I2C_DELAY_US);
  
  SET_I2C_SCK_PIN(0);
  SET_I2C_SDA_PIN(0);
  delay_us(I2C_DELAY_US);
}

uint8_t i2c_getack()
{
  uint8_t val = 0;
  
  /* 为输入做准备，确保从机ACK真正为低 */
  SET_I2C_SDA_PIN(1);
  SET_I2C_SDA_DIR(INPUT);
  SET_I2C_SCK_PIN(1);
  delay_us(I2C_DELAY_US);
  
  if( GET_I2C_SDA_PIN() )
    val = 1;
  SET_I2C_SCK_PIN(0);
  delay_us(I2C_DELAY_US);
  SET_I2C_SDA_DIR(OUTPUT);
  
  return ( (val==1)?NOACK:ACK );
}

void i2c_tx(uint8_t wdata)
{
  uint8_t i;

  for(i=0;i<8;i++)
  {
    if( (wdata & 0x80) == 0x80)
      SET_I2C_SDA_PIN(1);
    else
      SET_I2C_SDA_PIN(0);
    wdata <<= 1;
    SET_I2C_SCK_PIN(1);
    delay_us(I2C_DELAY_US);	//Confirm the time of csk high level over 4.7us
    SET_I2C_SCK_PIN(0);
    delay_us(I2C_DELAY_US);  
  }
  SET_I2C_SDA_PIN(1);
}

uint8_t i2c_rx()
{
  uint8_t i;
  uint8_t tmp;
  

  SET_I2C_SDA_PIN(1);	//Release data bus
  SET_I2C_SDA_DIR(INPUT);
  for(i=0;i<8;i++)
  {
    
    SET_I2C_SCK_PIN(1);
    delay_us(I2C_DELAY_US);
    tmp <<= 1;
    if( GET_I2C_SDA_PIN() )
      tmp |= 0x01;
    SET_I2C_SCK_PIN(0);
    delay_us(I2C_DELAY_US);
  }
  SET_I2C_SDA_DIR(OUTPUT);
  return tmp;
}

