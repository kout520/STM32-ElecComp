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

int PWM_num_1 = 0;	// 脉冲数
int PWM_num_2 = 0;	// 脉冲数
int PWM_num_3 = 0;	// 脉冲数
uint16_t y1;
uint16_t y2;
uint16_t y3;


//步进电机转的圈数
float record[8];
uint8_t i;
uint8_t motor1;
uint8_t motor3;
uint8_t motor2;
void BJMotor_distant(uint8_t c, float x)
{
    record[i++] = x;
    //正数为正转，负数为反转
    if(c==1)
    {
    if(x>0)HAL_GPIO_WritePin(GPIOA, GPIO_PIN_11, GPIO_PIN_SET);
    else {HAL_GPIO_WritePin(GPIOA, GPIO_PIN_11, GPIO_PIN_RESET);x=-x;}
    //启动PWM中断
    y1 = x * PULSES_PER_REV;//这里1600是因为转一圈1600脉冲；
    motor1 = 1;
    HAL_TIM_PWM_Start_IT(&htim4, TIM_CHANNEL_1);
    }
    if(c==2)
    {
    if(x>0)HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12, GPIO_PIN_SET);
    else {HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12, GPIO_PIN_RESET);x=-x;}
    //启动PWM中断
    y2 = x * PULSES_PER_REV;//这里1600是因为转一圈1600脉冲；
    motor2 = 2;
    HAL_TIM_PWM_Start_IT(&htim3, TIM_CHANNEL_1);
    }
    if(c==3)
    {
    if(x>0)HAL_GPIO_WritePin(GPIOC, GPIO_PIN_10, GPIO_PIN_SET);
    else {HAL_GPIO_WritePin(GPIOC, GPIO_PIN_10, GPIO_PIN_RESET);x=-x;}
    //启动PWM中断
    y3 = x * PULSES_PER_REV;//这里1600是因为转一圈1600脉冲；
    motor3 = 3;
    HAL_TIM_PWM_Start_IT(&htim8, TIM_CHANNEL_1);
    }
}
void HAL_TIM_PWM_PulseFinishedCallback(TIM_HandleTypeDef *htim)
{
    if(htim->Instance == TIM3)	// 确认是否为步进电机脉冲中断的回调
    {
            if(motor2 == 2)
        {
        PWM_num_2 ++;
         if(PWM_num_2 >= y2)		// 第1600次（一圈）
            {
            PWM_num_2= 0;
            HAL_TIM_PWM_Stop_IT(&htim3, TIM_CHANNEL_1);		// 停止输出PWM
                motor2 = 0;
            }
        }
        
    }
    if(htim->Instance == TIM8)	// 确认是否为步进电机脉冲中断的回调
    {
        if(motor3 == 3)
        {
            PWM_num_3 ++;
            if(PWM_num_3 >= y3)		// 第1600次（一圈）
                {
                PWM_num_3 = 0;
                HAL_TIM_PWM_Stop_IT(&htim8, TIM_CHANNEL_1);		// 停止输出PWM
                    motor3 = 0;
                }
        }
        
    }    
    if(htim->Instance == TIM4)	// 确认是否为步进电机脉冲中断的回调
    {
        if(motor1 == 1)
        {
            PWM_num_1 ++;
            if(PWM_num_1 >= y1)		// 第1600次（一圈）
               {
                PWM_num_1 = 0;
                HAL_TIM_PWM_Stop_IT(&htim4, TIM_CHANNEL_1);		// 停止输出PWM
                   motor1 = 0;
               }
        }
	}
}


//步进电机转速,一般ccr的值为arr的一半。
void BJMotor_Speed(uint16_t arr,uint16_t ccr)
{
    if(ccr>=arr)ccr = arr/2;
    TIM4->CCR1 = ccr;
    TIM4->ARR = arr;//arr越小，频率越快
    TIM3->CCR1 = ccr;
    TIM3->ARR = arr;
    TIM8->CCR1 = ccr;
    TIM8->ARR = arr;    
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
        return;
    }

    // 计算总旋转圈数
    for (uint8_t j = i - step; j < i; j++)
    {
        sum += record[j];
    }

    // 输出结果
    //BJMotor_distant(c,-sum);
}

