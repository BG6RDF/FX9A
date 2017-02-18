#include "bsp_pwm.h"

//TIM3_CH4/PB1/36脚   输出音频信号，音频频率在500HZ-1KHZ之间可调（信号随KEY1/KEY2长短变化，同步于CW）

//占空比，取值范围为0-100
int dutyfactor = 50;

// F = 128 + 127sin(2*pi*n/N)
int SinTable[] = {255, 254, 246, 234, 219, 199, 177, 153, 128, 103, 79, 57, 37, 22, 10, 2,
1, 2, 10, 22, 37, 57, 79, 103, 128, 153, 177, 199, 219, 234, 246, 255};

//uint16_t SinTable[] = {255, 246, 219, 177, 128, 79, 37, 10,
//1, 10, 37, 79, 128, 177, 219, 246, 255};

uint32_t SinPara[32];

volatile uint8_t SampleTimes = 0;
#define PWM_CLK_FREQ 72000000   //BG6RDF
uint16_t pwm_period = 0;

uint8_t BuzzerIsRunning = 0;
void pwm_init()
{
  // GPIO配置
  GPIO_InitTypeDef GPIO_InitStructure;
  
  GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_1;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
  
  uint32_t prescaler;
  // TIM3配置
  TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
  
  //prescaler = PWM_CLK_FREQ / (hz * period); // 5.859375
  prescaler = 4;
  pwm_period = PWM_CLK_FREQ / 8000 / prescaler;
  //重新将Timer设置为缺省值
  TIM_DeInit(TIM3);
  
  //采用内部时钟给TIM3提供时钟源
  //TIM_InternalClockConfig(TIM3);
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
  
  //预分频系数为0，即不进行预分频，此时TIMER的频率为72MHz
  TIM_TimeBaseStructure.TIM_Prescaler = prescaler - 1;
  
  //设置时钟分割
  TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
  
  //设置计数器模式为向上计数模式
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  
  //设置计数溢出大小，每计7200个数就产生一个更新事件，即PWM的输出频率为10kHz
  TIM_TimeBaseStructure.TIM_Period = pwm_period - 1;
  
  //将配置应用到TIM3中
  TIM_TimeBaseInit(TIM3,&TIM_TimeBaseStructure);

  
  // PWM配置
  TIM_OCInitTypeDef TimOCInitStructure;
  
  //设置缺省值
  TIM_OCStructInit(&TimOCInitStructure);
  
  //PWM模式1输出
  TimOCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
  
  //设置占空比，占空比=(CCRx/ARR)*100%或(TIM_Pulse/TIM_Period)*100%
  TimOCInitStructure.TIM_Pulse = dutyfactor * pwm_period / 100;
  
  //TIM输出比较极性高
  TimOCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
  
  //使能输出状态
  TimOCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
  
  //TIM3的CH4输出
  TIM_OC4Init(TIM3, &TimOCInitStructure);
  
  TIM_OC4PreloadConfig(TIM3,TIM_OCPreload_Enable);			// 将CCR预装载功能打开
  TIM_ARRPreloadConfig(TIM3, ENABLE);	  				// 将APR预装载功能打开
  //设置TIM3的PWM输出为禁止
  //TIM_CtrlPWMOutputs(TIM3,DISABLE);           //只用于TIM1和TIM8  BG6RDF
  
  TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);
}


void pwm_freq_set(uint32_t hz)
{
  uint32_t prescaler;
  // TIM3配置
  TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
  
  hz = hz * 32; // 采样频率 Fsample = Fpwm * 32;
  //prescaler = PWM_CLK_FREQ / (hz * period); // 5.859375
  prescaler = 2;
  pwm_period = PWM_CLK_FREQ / hz / prescaler;
  
  //预分频系数为0，即不进行预分频，此时TIMER的频率为72MHz
  TIM_TimeBaseStructure.TIM_Prescaler = prescaler - 1;
  
  //设置时钟分割
  TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
  
  //设置计数器模式为向上计数模式
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  
  //设置计数溢出大小，每计7200个数就产生一个更新事件，即PWM的输出频率为10kHz
  TIM_TimeBaseStructure.TIM_Period = pwm_period - 1;
  
  //将配置应用到TIM3中
  TIM_TimeBaseInit(TIM3,&TIM_TimeBaseStructure);
}

void sin_para_get()
{
  for(uint8_t i=0; i<32; i++)
  {
    SinPara[i] = SinTable[i] * pwm_period / 256;        //除以256是归一化，
    SinPara[i] = SinPara[i] * MENU.dutyOfbuzzer / 10;
  }
}

void pwm_start()
{
  TIM3->CCER |=0x1000;                  //channel 4打开BG6RDF
  TIM_Cmd(TIM3, ENABLE);	
  //TIM_CtrlPWMOutputs(TIM3,ENABLE);    //只用于TIM1和TIM8  BG6RDF
}

void pwm_stop()
{
  //TIM_CtrlPWMOutputs(TIM3,DISABLE);   //只用于TIM1和TIM8  BG6RDF
  TIM_Cmd(TIM3, DISABLE);
  TIM3->CCER &=0xefff;                  //channel 4关闭 BG6RDF
  TIM_SetCompare4(TIM3,  0);            //BG6RDF
  SampleTimes = 15;                     //BG6RDF
}

void buzzer_freq_set(uint32_t hz)
{
  pwm_freq_set(hz);
}

void buzzer_start()
{
  if (BuzzerIsRunning == 0)
  {
    BuzzerIsRunning = 1;
    pwm_start();
  }
}

void buzzer_stop()
{
  BuzzerIsRunning = 0;
  pwm_stop();
}

void TIM3_IRQHandler(void)
{
  if(TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)     //检测是否发生溢出更新事件  
  {
    TIM_ClearITPendingBit(TIM3 , TIM_FLAG_Update);      //清除TIM3的中断待处理位 
    SampleTimes = (SampleTimes + 1) % 32;
    TIM_SetCompare4(TIM3,  SinPara[SampleTimes]);
  }
}