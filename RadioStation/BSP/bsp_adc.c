#include "bsp_adc.h"

/*
  15脚  PC0-ADC12_IN10  供电电压检测输入端（显示供电电压，由5K和1K电阻分压所得）
  17脚  PC1-ADC12_IN11  反射功率电压检测端（电压值在0-3V，仅作参考）
  16脚  PC2-ADC12_IN12  正向功率电压检测段（电压值在0-3V，仅作参考）
  18脚  PC3-ADC12_IN13  接收电平电压检测段（电压值在0-3V，仅作参考）
*/
#define BATT_CHK GPIO_Pin_0
#define SWR_CHK GPIO_Pin_1
#define RF_CHK GPIO_Pin_2
#define RX_CHK GPIO_Pin_3

#define ADC1_DR_Address    ((uint32_t)0x4001244C)

uint16_t ADC_Value[16];

void ADC_Config(void)
{
  DMA_InitTypeDef DMA_InitStructure;
  DMA_DeInit(DMA1_Channel1);
  DMA_InitStructure.DMA_PeripheralBaseAddr = ADC1_DR_Address;   //定义 DMA通道外设基地址=ADC1_DR_Address
  DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)&ADC_Value;       //定义DMA通道存储器地址
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;            //指定外设为源地址
  DMA_InitStructure.DMA_BufferSize = 16;                        //定义DMA缓冲区大小
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;    //当前外设寄存器地址不变
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;       //当前存储器地址不变
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord; //定义外设数据宽度16位
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord; //定义存储器数据宽度16位
  DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;               //DMA通道操作模式为环形缓冲模式
  DMA_InitStructure.DMA_Priority = DMA_Priority_High;           //DMA通道优先级高
  DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;                  //禁止DMA通道存储器到存储器传输
  DMA_Init(DMA1_Channel1, &DMA_InitStructure);
  DMA_Cmd(DMA1_Channel1, ENABLE);
  
  ADC_InitTypeDef ADC_InitStructure;                  //定义ADC初始化结构体变量
  ADC_DeInit(ADC1);
  ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;  //ADC1和ADC2工作在独立模式
  ADC_InitStructure.ADC_ScanConvMode = ENABLE;        //使能扫描
  ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;  //ADC转换工作在单次模式
  ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None; //软件控制转换
  ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;//转换数据右对齐
  ADC_InitStructure.ADC_NbrOfChannel = 4;//扫描通道数 
  ADC_Init(ADC1, &ADC_InitStructure); //初始化ADC
  
  ADC_RegularChannelConfig(ADC1, ADC_Channel_10,  1, ADC_SampleTime_239Cycles5); //BATT
  ADC_RegularChannelConfig(ADC1, ADC_Channel_11, 2, ADC_SampleTime_239Cycles5); 
  ADC_RegularChannelConfig(ADC1, ADC_Channel_12, 3, ADC_SampleTime_239Cycles5);
  ADC_RegularChannelConfig(ADC1, ADC_Channel_13, 4, ADC_SampleTime_239Cycles5);
  //ADC1选择信道14,音序器等级1,采样时间239.5个周期
  ADC_DMACmd(ADC1, ENABLE);//使能ADC1模块DMA
  ADC_Cmd(ADC1, ENABLE);//使能ADC1
  
  ADC_ResetCalibration(ADC1); //重置ADC1校准寄存器
  while(ADC_GetResetCalibrationStatus(ADC1));//等待ADC1校准重置完成
  ADC_StartCalibration(ADC1);//开始ADC1校准
  while(ADC_GetCalibrationStatus(ADC1));//等待ADC1校准完成
  ADC_SoftwareStartConvCmd(ADC1, ENABLE); //使能ADC1软件开始转换
}

//PC0/15/ADC12_IN10
void adc_init()
{
  GPIO_InitTypeDef GPIO_InitStructure; //定义GPIO初始化结构体
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1 | RCC_APB2Periph_GPIOC, ENABLE);
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);	//使能DMA传输
  
  //配置PC0为模拟输入
  GPIO_InitStructure.GPIO_Pin = BATT_CHK | SWR_CHK | RF_CHK | RX_CHK;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
  GPIO_Init(GPIOC, &GPIO_InitStructure);
  
//  NVIC_InitTypeDef NVIC_InitStructure;  
//  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); 
//  
//  NVIC_InitStructure.NVIC_IRQChannel = ADC1_2_IRQn; 
//  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0; 
//  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;  
//  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;  
//  NVIC_Init(&NVIC_InitStructure);
  
  ADC_Config(); 
  
//  ADC_ITConfig(ADC1, ADC_IT_EOC, ENABLE);
}

uint16_t adc_batt()
{
  return (ADC_Value[0] + ADC_Value[4] + ADC_Value[8] + ADC_Value[12]) / 4;
}

uint16_t adc_rf()
{
  return (ADC_Value[1] + ADC_Value[5] + ADC_Value[9] + ADC_Value[13]) / 4;
}

uint16_t adc_swr()
{
  return (ADC_Value[2] + ADC_Value[6] + ADC_Value[10] + ADC_Value[14]) / 4;
}

uint16_t adc_rx()
{
  return (ADC_Value[3] + ADC_Value[7] + ADC_Value[11] + ADC_Value[15]) / 4;
}

void adc_start()
{
  ADC_SoftwareStartConvCmd(ADC1, ENABLE); //使能ADC1软件开始转换
}
void ADC1_2_IRQHandler()
{
  if (ADC_GetITStatus(ADC1, ADC_IT_EOC) == SET)
  {
    DC.val = ADC_GetConversionValue(ADC1);
    DRMgr.item |= DCItem;
    ADC_ClearITPendingBit(ADC1, ADC_IT_EOC);
  }
}