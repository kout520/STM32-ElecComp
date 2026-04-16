//使用说明
//初始化
//Motor_Control_Init();
//Set_Motor_Direction(1);
//HAL_TIM_Base_Start_IT(&htim5);
//电机更新速度
//Motor_Control_Update();
//定义定时器5为20ms中断：
//进行编码器读数

#include "headfile.h"
// 定义左右电机的 PID 控制器和编码器

PIDController Speed_Pid_L;  // 左电机 PID 控制器

Encoder Encoder_L;          // 左电机编码器
// 定义 TIM 句柄（假设已经在 main.c 中初始化）

extern TIM_HandleTypeDef htim2;  // PWM 控制 TIM2
extern TIM_HandleTypeDef htim3;  // 编码器 TIM3

// 定义 H 桥控制引脚
#define LEFT_MOTOR_PIN1 GPIO_PIN_11
#define LEFT_MOTOR_PIN2 GPIO_PIN_12

#define LEFT_MOTOR_PORT GPIOD

// 定义小车控制参数
#define BASE_SPEED_LEFT 41.0f      // 左轮基础速度 (m/s)

#define TURN_GAIN 1.5f       // 转向增益系数

float target_speed_L = BASE_SPEED_LEFT; // 左电机目标速度

// 定义距离控制参数
float distance_traveled = 0.0f;  // 已行驶距离 (m)
uint8_t distance_reached = 0;    // 距离到达标志位

int car_flag = 0;

float output_L;

float kp = 0.85f;
float ki = 7.00f;
float kd = 0.008f;
/**
 * @brief 初始化电机控制和编码器
 */
void Motor_Control_Init(void) {
    // 使能编码器
    HAL_TIM_Encoder_Start(&htim3, TIM_CHANNEL_ALL);  // 左电机编码器
    // 使能 PWM 输出
    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_2);  // 左电机 PWM
    // 初始化 PID 控制器
    PID_Incremental_Init(&Speed_Pid_L, kp, ki, kd, target_speed_L); // 左电机 PID
    // 初始化编码器
    Encoder_Init(&Encoder_L, 0.2f, 1500); // 左电机轮子周长 0.2 米，每转 1500 脉冲
}
/**
 * @brief 更新电机控制
 */
void Motor_Control_Update(void) {

	 // 计算已行驶距离
    //distance_traveled = (Encoder_Get_Distance(&Encoder_R) + Encoder_Get_Distance(&Encoder_L)) / 2.0f;
    // 计算 PID 输出
     output_L = PID_Incremental_Calculate(&Speed_Pid_L, Encoder_Get_Speed(&Encoder_L),0.01);
    // 设置电机 PWM 占空比
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, (uint32_t)output_L);  // 左电机 PWM
}
/**
 * @brief 定时器中断
 * @param 对编码器更新
 */
int32_t pulse_count_L;
//void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
//{
//    if(htim->Instance == TIM5)//20ms
//    {
//        // 获取编码器脉冲计数
//     pulse_count_L = (int32_t)__HAL_TIM_GET_COUNTER(&htim3);  // 左电机编码器
//        __HAL_TIM_SET_COUNTER(&htim3, 0);  // 读取后清零计数器
//    // 更新编码器速度
//    float time_interval = 0.02f; // 假设时间间隔为 0.02 秒
//    Encoder_Update(&Encoder_L, pulse_count_L, time_interval);
//    }
//}


/**
 * @brief 设置电机方向
 * @param left_dir 左电机方向（1: 正转, -1: 反转）
 * @param right_dir 右电机方向（1: 正转, -1: 反转）
 */
void Set_Motor_Direction(int8_t left_dir) {
    // 左电机方向控制
    if (left_dir == 1) {
        HAL_GPIO_WritePin(LEFT_MOTOR_PORT, LEFT_MOTOR_PIN1, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(LEFT_MOTOR_PORT, LEFT_MOTOR_PIN2, GPIO_PIN_SET);
    } else if (left_dir == -1) {
        HAL_GPIO_WritePin(LEFT_MOTOR_PORT, LEFT_MOTOR_PIN1, GPIO_PIN_SET);
        HAL_GPIO_WritePin(LEFT_MOTOR_PORT, LEFT_MOTOR_PIN2, GPIO_PIN_RESET);
    } else if (left_dir == 0) {
        HAL_GPIO_WritePin(LEFT_MOTOR_PORT, LEFT_MOTOR_PIN1, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(LEFT_MOTOR_PORT, LEFT_MOTOR_PIN2, GPIO_PIN_RESET);
    }

}

/**
 * @brief 小车差速控制
 * @param error 偏差值（左偏为负，右偏为正）
 */
void Differential_Control(float error) {
    // 根据偏差值调整左右轮目标速度

    target_speed_L = BASE_SPEED_LEFT - TURN_GAIN * error; // 左轮速度减少
    // 更新 PID 目标速度
    //PID_SetTarget(&Speed_Pid_L,  kp,ki,kd,target_speed_L);
    // 设置电机方向为正转
    Set_Motor_Direction(1);
}
/**
 * @brief 停止电机
 */
void Stop_Motors(void) {
    Set_Motor_Direction(0); // 停止电机
}



