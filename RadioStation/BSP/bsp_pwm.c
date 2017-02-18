#include "bsp_pwm.h"

//TIM3_CH4/PB1/36��   �����Ƶ�źţ���ƵƵ����500HZ-1KHZ֮��ɵ����ź���KEY1/KEY2���̱仯��ͬ����CW��

//ռ�ձȣ�ȡֵ��ΧΪ0-100
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
  // GPIO����
  GPIO_InitTypeDef GPIO_InitStructure;
  
  GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_1;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
  
  uint32_t prescaler;
  // TIM3����
  TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
  
  //prescaler = PWM_CLK_FREQ / (hz * period); // 5.859375
  prescaler = 4;
  pwm_period = PWM_CLK_FREQ / 8000 / prescaler;
  //���½�Timer����Ϊȱʡֵ
  TIM_DeInit(TIM3);
  
  //�����ڲ�ʱ�Ӹ�TIM3�ṩʱ��Դ
  //TIM_InternalClockConfig(TIM3);
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
  
  //Ԥ��Ƶϵ��Ϊ0����������Ԥ��Ƶ����ʱTIMER��Ƶ��Ϊ72MHz
  TIM_TimeBaseStructure.TIM_Prescaler = prescaler - 1;
  
  //����ʱ�ӷָ�
  TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
  
  //���ü�����ģʽΪ���ϼ���ģʽ
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  
  //���ü��������С��ÿ��7200�����Ͳ���һ�������¼�����PWM�����Ƶ��Ϊ10kHz
  TIM_TimeBaseStructure.TIM_Period = pwm_period - 1;
  
  //������Ӧ�õ�TIM3��
  TIM_TimeBaseInit(TIM3,&TIM_TimeBaseStructure);

  
  // PWM����
  TIM_OCInitTypeDef TimOCInitStructure;
  
  //����ȱʡֵ
  TIM_OCStructInit(&TimOCInitStructure);
  
  //PWMģʽ1���
  TimOCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
  
  //����ռ�ձȣ�ռ�ձ�=(CCRx/ARR)*100%��(TIM_Pulse/TIM_Period)*100%
  TimOCInitStructure.TIM_Pulse = dutyfactor * pwm_period / 100;
  
  //TIM����Ƚϼ��Ը�
  TimOCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
  
  //ʹ�����״̬
  TimOCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
  
  //TIM3��CH4���
  TIM_OC4Init(TIM3, &TimOCInitStructure);
  
  TIM_OC4PreloadConfig(TIM3,TIM_OCPreload_Enable);			// ��CCRԤװ�ع��ܴ�
  TIM_ARRPreloadConfig(TIM3, ENABLE);	  				// ��APRԤװ�ع��ܴ�
  //����TIM3��PWM���Ϊ��ֹ
  //TIM_CtrlPWMOutputs(TIM3,DISABLE);           //ֻ����TIM1��TIM8  BG6RDF
  
  TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);
}


void pwm_freq_set(uint32_t hz)
{
  uint32_t prescaler;
  // TIM3����
  TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
  
  hz = hz * 32; // ����Ƶ�� Fsample = Fpwm * 32;
  //prescaler = PWM_CLK_FREQ / (hz * period); // 5.859375
  prescaler = 2;
  pwm_period = PWM_CLK_FREQ / hz / prescaler;
  
  //Ԥ��Ƶϵ��Ϊ0����������Ԥ��Ƶ����ʱTIMER��Ƶ��Ϊ72MHz
  TIM_TimeBaseStructure.TIM_Prescaler = prescaler - 1;
  
  //����ʱ�ӷָ�
  TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
  
  //���ü�����ģʽΪ���ϼ���ģʽ
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  
  //���ü��������С��ÿ��7200�����Ͳ���һ�������¼�����PWM�����Ƶ��Ϊ10kHz
  TIM_TimeBaseStructure.TIM_Period = pwm_period - 1;
  
  //������Ӧ�õ�TIM3��
  TIM_TimeBaseInit(TIM3,&TIM_TimeBaseStructure);
}

void sin_para_get()
{
  for(uint8_t i=0; i<32; i++)
  {
    SinPara[i] = SinTable[i] * pwm_period / 256;        //����256�ǹ�һ����
    SinPara[i] = SinPara[i] * MENU.dutyOfbuzzer / 10;
  }
}

void pwm_start()
{
  TIM3->CCER |=0x1000;                  //channel 4��BG6RDF
  TIM_Cmd(TIM3, ENABLE);	
  //TIM_CtrlPWMOutputs(TIM3,ENABLE);    //ֻ����TIM1��TIM8  BG6RDF
}

void pwm_stop()
{
  //TIM_CtrlPWMOutputs(TIM3,DISABLE);   //ֻ����TIM1��TIM8  BG6RDF
  TIM_Cmd(TIM3, DISABLE);
  TIM3->CCER &=0xefff;                  //channel 4�ر� BG6RDF
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
  if(TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)     //����Ƿ�����������¼�  
  {
    TIM_ClearITPendingBit(TIM3 , TIM_FLAG_Update);      //���TIM3���жϴ�����λ 
    SampleTimes = (SampleTimes + 1) % 32;
    TIM_SetCompare4(TIM3,  SinPara[SampleTimes]);
  }
}