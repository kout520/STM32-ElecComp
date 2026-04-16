#include "Moter.h"
#include "PWM.h"
void Moter_init(void)
{ 
  GPIO_InitTypeDef GPIO_InitStructure;
  
  /* 使能端口B时钟 */
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);    
  
  /* 配置PB5和PB8引脚 */
  GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_5 | GPIO_Pin_8;  /* 使用PB5和PB8引脚 */
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
  
  /* 初始化GPIOB */
  GPIO_Init(GPIOB, &GPIO_InitStructure);
  PWM_Hardware_Init();
	PWM_SetDuty_RTOS_Safe(1,50);
	PWM_SetDuty_RTOS_Safe(2,50);
  /* 设置PB5和PB8输出高电平（LED熄灭） */
  GPIO_SetBits(GPIOB, GPIO_Pin_5 | GPIO_Pin_8);   
}

void Moter_Ctrl_LR(int dummy_param)
{
    if(dummy_param >= 0)
    { PWM_SetFrequency_RTOS_Safe(1,dummy_param);
        /* PB5输出低电平 */
        GPIO_ResetBits(GPIOB, GPIO_Pin_5);
    }
    else
    {
			PWM_SetFrequency_RTOS_Safe(1,-dummy_param);
        /* PB5输出高电平 */
        GPIO_SetBits(GPIOB, GPIO_Pin_5);
    }
   
}

// 线性插值函数1
double linear_interpolation_distance(double x) 
	{
    // 已知数据点（输入, 输出）
    double data_points[][2] = {
        {111, 160}, {123, 145}, {126, 135}, {128, 126}, {135, 115},
        {158, 86}, {175, 93}, {185, 86}, {201, 76}, {227, 69},
        {238, 65}, {273, 57}, {297, 53}
    };
    
    int num_points = sizeof(data_points) / sizeof(data_points[0]);
    int i;
    double x0, y0, x1, y1;
    
    // 如果输入值正好是已知点，直接返回对应输出
    for (i = 0; i < num_points; i++) {
        if (x == data_points[i][0]) {
            return data_points[i][1];
        }
    }
    
    // 如果输入值小于最小已知点，使用最小两个点外推
    if (x < data_points[0][0]) {
        x0 = data_points[0][0];
        y0 = data_points[0][1];
        x1 = data_points[1][0];
        y1 = data_points[1][1];
        return y0 + (y1 - y0) / (x1 - x0) * (x - x0);
    }
    
    // 如果输入值大于最大已知点，使用最大两个点外推
    if (x > data_points[num_points-1][0]) {
        x0 = data_points[num_points-2][0];
        y0 = data_points[num_points-2][1];
        x1 = data_points[num_points-1][0];
        y1 = data_points[num_points-1][1];
        return y0 + (y1 - y0) / (x1 - x0) * (x - x0);
    }
    
    // 查找输入值所在的区间
    for (i = 0; i < num_points - 1; i++) {
        x0 = data_points[i][0];
        y0 = data_points[i][1];
        x1 = data_points[i + 1][0];
        y1 = data_points[i + 1][1];
        
        if (x0 <= x && x <= x1) {
            // 线性插值
            return y0 + (y1 - y0) / (x1 - x0) * (x - x0);
        }
    }
    
    // 理论上不会执行到此处
    return 0.0;
}

// 线性插值函数2
double linear_interpolation_w(double x) 
	{
    // 已知数据点（输入, 输出）
    double data_points[][2] = {
        {105, 136}, {116, 142}, {125, 152}, {135, 170}, {142, 186},
        {158, 201}, {165, 223}, {172, 242}, {198, 266}, {218, 297},
        {231, 297}, {244, 347}, {258, 360}, {291, 390}
    };
    
    int num_points = sizeof(data_points) / sizeof(data_points[0]);
    int i;
    double x0, y0, x1, y1;
    
    // 如果输入值正好是已知点，直接返回对应输出
    for (i = 0; i < num_points; i++) {
        if (x == data_points[i][0]) {
            return data_points[i][1];
        }
    }
    
    // 如果输入值小于最小已知点，使用最小两个点外推
    if (x < data_points[0][0]) {
        x0 = data_points[0][0];
        y0 = data_points[0][1];
        x1 = data_points[1][0];
        y1 = data_points[1][1];
        return y0 + (y1 - y0) / (x1 - x0) * (x - x0);
    }
    
    // 如果输入值大于最大已知点，使用最大两个点外推
    if (x > data_points[num_points-1][0]) {
        x0 = data_points[num_points-2][0];
        y0 = data_points[num_points-2][1];
        x1 = data_points[num_points-1][0];
        y1 = data_points[num_points-1][1];
        return y0 + (y1 - y0) / (x1 - x0) * (x - x0);
    }
    
    // 查找输入值所在的区间
    for (i = 0; i < num_points - 1; i++) {
        x0 = data_points[i][0];
        y0 = data_points[i][1];
        x1 = data_points[i + 1][0];
        y1 = data_points[i + 1][1];
        
        if (x0 <= x && x <= x1) {
            // 线性插值
            return y0 + (y1 - y0) / (x1 - x0) * (x - x0);
        }
    }
    
    // 理论上不会执行到此处
    return 0.0;
}
	
int angle_to_frequency(double angle_degrees) 
{
    /* 所有变量定义在函数开头 */
    const int steps_per_rev = 200;
    const int microsteps = 256;
    long pulses_per_rev;
    double degrees_per_pulse;
    double total_pulses;
    double frequency;
    
    /* 执行计算 */
    pulses_per_rev = steps_per_rev * microsteps;
    degrees_per_pulse = 360.0 / pulses_per_rev;
    
    // 计算完成角度所需的总脉冲数
    total_pulses = angle_degrees / degrees_per_pulse;
    
    // 计算频率：脉冲数/时间(秒)，125ms = 0.125秒
    frequency = total_pulses / 0.125;
    
    return (int)frequency;
}
