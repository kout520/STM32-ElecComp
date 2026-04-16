//使用说明
//配置tim的pwm，不要忘记打开中断
//一般步进电机接收频率为1kHz-20kHz
//将ENA+，DIR+，PUL+，接单片机可接收电压（一般3.3V）
//ENA使能引脚，DIR反向引脚，PUL脉冲接收引脚
//先要调用BJMotor_Speed确定速度，一般arr根据psc来设置
//在调用BJMotor_distant();确定圈数和方向
//使用回调时不要忘了将i = 0；
#include "headfile.h"
#define PULSES_PER_REV 1600  // 每圈脉冲数定义，可根据步进电机/驱动器实际情况调整

int PWM_num = 0;	// 脉冲数
uint16_t y;
void HAL_TIM_PWM_PulseFinishedCallback(TIM_HandleTypeDef *htim)
{
	if(htim->Instance == TIM2)	// 确认是否为步进电机脉冲中断的回调
	{
		PWM_num ++;
		if(PWM_num >= y)		// 第1600次（一圈）
		{
			PWM_num = 0;
			HAL_TIM_PWM_Stop_IT(&htim2, TIM_CHANNEL_2);		// 停止输出PWM
			HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_5);		// 翻转led灯电平 
		}
	}
}


//步进电机转速,一般ccr的值为arr的一半。
void BJMotor_Speed(uint16_t arr,uint16_t ccr)
{
    if(ccr>=arr)ccr = arr/2;
    TIM2->CCR2 = ccr;
    TIM2->ARR = arr;//arr越小，频率越快
}

//步进电机转的圈数
float record[8];
uint8_t i;
void BJMotor_distant(float x)
{
    record[i++] = x;
    //正数为正转，负数为反转
    
    if(x>0)HAL_GPIO_WritePin(GPIOE, GPIO_PIN_0, GPIO_PIN_SET);
    else {HAL_GPIO_WritePin(GPIOE, GPIO_PIN_0, GPIO_PIN_RESET);x=-x;}
    //启动PWM中断
    y = x * PULSES_PER_REV;//这里1600是因为转一圈1600脉冲；
    printf("%d",i);
    HAL_TIM_PWM_Start_IT(&htim2, TIM_CHANNEL_2);
}

/*
  函数：BJMotor_callblack
  描述：根据记录的运动轨迹计算总旋转圈数
  参数：
         step - 需要回溯的步数
  说明：从最新的运动轨迹开始，回溯指定步数并计算总旋转圈数
  调用了多少次BJMotor_Speed，step的最大值就是多少
*/
void BJMotor_callblack(uint8_t step)
{
    float sum = 0.0f;  // 初始化总和

    // 检查 step 是否超出记录范围
    if (step > i)
    {
        printf("Step exceeds recorded steps!\n");
        return;
    }

    // 计算总旋转圈数
    for (uint8_t j = i - step; j < i; j++)
    {
        sum += record[j];
    }

    // 输出结果
    printf("Total rotations: %.2f\n", sum);
    BJMotor_distant(-sum);
    
}

