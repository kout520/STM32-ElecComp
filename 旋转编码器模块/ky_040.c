#include "headfile.h"

uint32_t TimerEncoder;
int angle;

void ky_Init(void)
{
    HAL_TIM_Encoder_Start(&htim3,TIM_CHANNEL_ALL);
    //¿ªÆôÖÐ¶Ï
    HAL_TIM_Base_Start_IT(&htim2);
    TIM3->CNT = 100;
}


int getTimerEncoder(void)
{
	TimerEncoder=(short)(__HAL_TIM_GET_COUNTER(&htim3));
    //angle = TimerEncoder *18;
	__HAL_TIM_SET_COUNTER(&htim3,0);
    
	return TimerEncoder;
}


