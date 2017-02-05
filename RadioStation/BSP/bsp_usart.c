#include "bsp_usart.h"
#include "bsp_kenwood.h"

#define IC_706 0x48
#define CIV_ADDR IC_706

unsigned char UartRxLen = 0;
unsigned char UartRxBuf[32];

static UartTxBufDef UTBuffer;
static bool DMA_IsReady = true;

static uint8_t Uart_Send_Buffer[64];

//uint8_t viewBuf[64];
//uint8_t viewLen = 0;

USART_InitTypeDef RadioCom = {
  9600,
  USART_WordLength_8b,
  USART_StopBits_1,
  USART_Parity_No,
  USART_Mode_Rx | USART_Mode_Tx,
  USART_HardwareFlowControl_None
};

void UartBufInit()
{
  UTBuffer.head = 0;
  UTBuffer.rear = 0;
  UTBuffer.size	= 0;
  memset(UTBuffer.dat, 0, 32);
}

void dma_init()
{
  NVIC_InitTypeDef NVIC_InitStructure;
  DMA_InitTypeDef DMA_InitStructure;
  //启动DMA时钟  
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE); 
  
  //DMA发送中断设置  
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
  NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel4_IRQn;  
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;  
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;  
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;  
  NVIC_Init(&NVIC_InitStructure);  

  //DMA1通道4配置  
  DMA_DeInit(DMA1_Channel4);  
  
  //外设地址  
  DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)(&USART1->DR);
  
  //内存地址  
  DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)Uart_Send_Buffer;

  //dma传输方向单向  
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;  
  
  //设置DMA在传输时缓冲区的长度  
  DMA_InitStructure.DMA_BufferSize = 100;  
  
  //设置DMA的外设递增模式，一个外设  
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;  
  
  //设置DMA的内存递增模式  
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;  
  
  //外设数据字长  
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;  
  
  //内存数据字长  
  DMA_InitStructure.DMA_MemoryDataSize = DMA_PeripheralDataSize_Byte;  
  
  //设置DMA的传输模式  
  DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;  
  
  //设置DMA的优先级别  
  DMA_InitStructure.DMA_Priority = DMA_Priority_High;  
  
  //设置DMA的2个memory中的变量互相访问  
  DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;  
  DMA_Init(DMA1_Channel4, &DMA_InitStructure);  
  DMA_ITConfig(DMA1_Channel4, DMA_IT_TC,ENABLE);  
    
  //使能通道4  
  //DMA_Cmd(DMA1_Channel4, ENABLE); 
}
void usart_init()
{  
  NVIC_InitTypeDef NVIC_InitStructure;
  GPIO_InitTypeDef GPIO_InitStructure;
  USART_InitTypeDef USART_InitStructure;                   //串口初始化结构体声明
  USART_ClockInitTypeDef USART_ClockInitStruct;
  
  UartBufInit();
  
  dma_init();

  // Release reset and enable clock
  USART_DeInit(USART1);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
  
  // GPIO Init
  // Enable GPIO clock and release reset
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE);
  RCC_APB2PeriphResetCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, DISABLE);
  
  // Assign PA9 to UART1 (Tx)
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
  
  // Assign PA10 to UART1 (Rx)GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  //GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
  
  
  //USART_ClockInitTypeDef USART_ClockInitStruct;
  USART_InitStructure.USART_BaudRate = 115200;      //设置波特率为115200bps
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;  //数据位8位
  USART_InitStructure.USART_StopBits = USART_StopBits_1;   //停止位1位
  USART_InitStructure.USART_Parity = USART_Parity_No;    //无校验位
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;   //无硬件流控
  USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;     //接受和发送模式都打开
  
  USART_ClockInitStruct.USART_Clock=USART_Clock_Disable;      //串口时钟禁止
  USART_ClockInitStruct.USART_CPOL=USART_CPOL_Low;        //数据低电平有效
  USART_ClockInitStruct.USART_CPHA=USART_CPHA_2Edge;    //配置CPHA使数据在第2个边沿的时候被捕获
  USART_ClockInitStruct.USART_LastBit=USART_LastBit_Disable;  // 禁用最后一位,使对应的时钟脉冲不会再输出到SCLK引脚
  USART_ClockInit(USART1, &USART_ClockInitStruct);      //配置USART与时钟相关的设置
  
  // Init UART1
  USART_Init(USART1, &USART_InitStructure);
  
  // Enable and configure the priority of the UART1 Update IRQ Channel
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
  NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
  
  USART_ClearFlag(USART1, USART_FLAG_TC  | USART_FLAG_RXNE );
  
  // Enable UART1 interrupts
  USART_ITConfig(USART1,USART_IT_RXNE,ENABLE);
  
  //采用DMA方式发送  
  USART_DMACmd(USART1,USART_DMAReq_Tx,ENABLE);  

  // Enable the UART1
  USART_Cmd(USART1, ENABLE);
}

void UartTxBufWrite(unsigned char wdat)
{
  if(UTBuffer.size < UART_TX_BUF_MAX_SIZE)
  {
    UTBuffer.dat[ UTBuffer.rear++ ] = wdat;
    if(UTBuffer.rear >= UART_TX_BUF_MAX_SIZE)
    {
      UTBuffer.rear = 0;
    }
    UTBuffer.size ++;	
  }
}

void UartTx(uint8_t *wdat, uint8_t len)
{
  for(uint8_t i=0; i<len; i++)
  {
    UartTxBufWrite(wdat[i]);  
  }
}

unsigned char UartTxBufRead()
{
  unsigned char rval = 0;
  if(UTBuffer.size > 0)
  {
    rval = UTBuffer.dat[ UTBuffer.head++ ];
    if(UTBuffer.head >= UART_TX_BUF_MAX_SIZE)
    {
      UTBuffer.head = 0;
    }							
    UTBuffer.size --; 
  }
  
  return rval;
}

unsigned char UART_TxBufSize()
{
  return UTBuffer.size;
}

void usart_rx_analysis(uint8_t wdat)
{
  if(wdat == ';')
  {
    Kenwood_CmdParser(UartRxBuf, UartRxLen);
    UartRxLen = 0;
  }
  else
  {
    UartRxBuf[ UartRxLen++ ] = wdat;
  }
}

bool DMA_IsTxComplete()
{
  return DMA_IsReady;
}

void CopyTBufToDMA()
{
  uint8_t len = UTBuffer.size;
  for(uint8_t i=0; i<len; i++)
  {
    Uart_Send_Buffer[i] = UartTxBufRead();
  }
}

void UART_DMAStart(uint8_t length)
{
  DMA_IsReady = 0;
  DMA_ClearFlag(DMA1_FLAG_TC4);      //清DMA发送完成标志
  DMA_Cmd(DMA1_Channel4, DISABLE);   //停止DMA
  
  //设置传输数据长度  
  DMA1_Channel4->CNDTR = length;      //重设传输长度
  //打开DMA  
  DMA_Cmd(DMA1_Channel4,ENABLE);  
}

void USART1_IRQHandler( void )
{
  uint8_t dat;
  
  if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
  {   
    dat = USART_ReceiveData(USART1);
    //USART_ClearITPendingBit(USART1, USART_IT_RXNE);

    usart_rx_analysis(dat);  
  }
  

}

  //串口1DMA方式发送中断  
void DMA1_Channel4_IRQHandler(void)  
{  
  //清除标志位  
  DMA_ClearFlag(DMA1_FLAG_TC4);  
  
  //关闭DMA  
  DMA_Cmd(DMA1_Channel4,DISABLE);  

  //允许再次发送  
  DMA_IsReady = 1;  
} 