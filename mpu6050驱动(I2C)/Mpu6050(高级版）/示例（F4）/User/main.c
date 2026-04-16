#include "FreeRTOS.h" 
#include "task.h"
#include "stm32f4xx.h"
#include "key.h"
#include "usart1.h"
#include "usart2.h"
#include "./led/bsp_led.h"   
#include "queue.h"  
#include "semphr.h"
#include <string.h>
#include "event_groups.h"
#include "timers.h"
#include "Mpu6050.h"
#include "PID.h"
#include "Moter.h"
#include "kalman_filter.h"
#include <stdio.h>
#include <math.h>
#include "oled.h"
#define M_PI 3.1415926535
// 任务函数声明
void vInitHardware(void *pvParameters);    // 硬件初始化任务
void vInitTask(void *pvParameters);        // 应用任务初始化任务
void vTask1(void *pvParameters);           // 串口2数据处理任务
void vTask2(void *pvParameters);           // LED闪烁控制任务
void vTask3(void *pvParameters);           // 按键扫描任务
void vTaskUART(void *pvParameters);        // 串口1数据处理任务
void vTaskMpu9250(void *pvParameters);     // MPU6050传感器数据读取任务
void vTimerCallback(TimerHandle_t xTimer); // 软件定时器回调函数
void vStatsTask(void *pvParameters);       // 系统状态统计任务
void vMoterTask(void *pvParameters);
QueueHandle_t xMotorQueue;
// 高精度定时器计数，用于FreeRTOS运行时间统计
volatile unsigned long ulHighFrequencyTimerTicks = 0;

/**
 * @brief 获取运行时间计数器值
 * @return 当前定时器计数值
 */
unsigned long getRunTimeCounterValue(void)
{
    return ulHighFrequencyTimerTicks;
}

/**
 * @brief TIM2定时器中断服务函数
 * @note 用于FreeRTOS运行时间统计，每1ms产生一次中断
 */
void TIM2_IRQHandler(void)
{
    if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)
    {
        ulHighFrequencyTimerTicks++;           // 递增时间计数器
        TIM_ClearITPendingBit(TIM2, TIM_IT_Update);  // 清除中断标志位
    }
}

/**
 * @brief 初始化统计定时器
 * @note 配置TIM2为1kHz频率，为CPU利用率统计提供时间基准
 */
void vInitStatsTimer(void)
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
    
    // 使能TIM2时钟
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
    
    // 定时器基础配置
    TIM_TimeBaseStructure.TIM_Period = 1000 - 1;          // 自动重装载值
    TIM_TimeBaseStructure.TIM_Prescaler = 84 - 1;         // 预分频值
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;          // 时钟分频
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  // 向上计数模式
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);       // 初始化TIM2
    
    // 配置定时器中断
    TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
    
    // 配置NVIC中断
    NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 10;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    
    // 启动定时器
    TIM_Cmd(TIM2, ENABLE);
}

// 全局变量声明
TaskHandle_t xSuspendableTaskHandle = NULL;  // 可挂起任务句柄
TimerHandle_t xPeriodicTimer = NULL;         // 周期性软件定时器句柄

/**
 * @brief 主函数 - 系统入口点
 */
int main(void)
{
    // 系统基础初始化
    SystemInit();  // 初始化系统时钟
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);  // 配置中断优先级分组
    
    // 创建硬件初始化任务
    xTaskCreate(vInitHardware,           // 任务函数
                "Hardware Init",         // 任务名称
                configMINIMAL_STACK_SIZE, // 堆栈大小
                NULL,                    // 任务参数
                6,                       // 任务优先级
                NULL);                   // 任务句柄
                
    // 启动FreeRTOS调度器
    vTaskStartScheduler();

    // 调度器启动失败处理
    while(1) {
        // 系统启动失败，进入死循环
    }
}

/**
 * @brief 硬件初始化任务
 * @param pvParameters 任务参数
 */
void vInitHardware(void *pvParameters)
{
    // 初始化各硬件外设
    led_init();        // LED初始化
    key_init();        // 按键初始化
    Serial_Init();     // 串口1初始化
    Serial2_Init();    // 串口2初始化
	Moter_init();
    vInitStatsTimer(); // 统计定时器初始化

    // 创建应用初始化任务
    xTaskCreate(vInitTask, 
                "Init Task", 
                configMINIMAL_STACK_SIZE, 
                NULL, 
                5,     // 优先级5
                NULL);
                
    // 删除当前任务
    vTaskDelete(NULL);
}

/**
 * @brief 应用任务初始化任务
 * @param pvParameters 任务参数
 */
void vInitTask(void *pvParameters)
{
	// 创建队列，存储5个数组，每个数组4个int
    xMotorQueue = xQueueCreate(5, sizeof(int[4]));
    // 创建串口2数据处理任务
    xTaskCreate(vTask1, 
                "UART2 Task", 
                configMINIMAL_STACK_SIZE, 
                NULL, 
                1,                       // 优先级1
                &xSuspendableTaskHandle); // 保存任务句柄

    // 创建LED控制任务
    xTaskCreate(vTask2, 
                "LED Control", 
                configMINIMAL_STACK_SIZE, 
                NULL, 
                2,     // 优先级2
                NULL);

    // 创建按键扫描任务
    xTaskCreate(vTask3, 
                "Key Scan", 
                configMINIMAL_STACK_SIZE, 
                NULL, 
                3,     // 优先级3
                NULL);

    // 创建串口1数据处理任务
    xTaskCreate(vTaskUART, 
                "UART Process", 
                256,   // 分配256字堆栈
                NULL, 
                4,     // 优先级4
                NULL);

    // 创建MPU6050传感器任务
    xTaskCreate(vTaskMpu9250, 
                "Mpu9250", 
                256,   // 分配256字堆栈
                NULL, 
                3,     // 优先级3
                NULL);

    // 创建CPU统计任务
    xTaskCreate(vStatsTask, 
                "CPU Stats", 
                256,   // 分配256字堆栈
                NULL, 
                1,     // 优先级1
                NULL);
								
    // 创建CPU统计任务
    xTaskCreate(vMoterTask, 
                "Moter Stats", 
                256,   // 分配256字堆栈
                NULL, 
                1,     // 优先级1
                NULL);
    // 创建周期性软件定时器
    xPeriodicTimer = xTimerCreate(
        "PeriodicTimer",        // 定时器名称
        pdMS_TO_TICKS(1000),    // 定时周期1000ms
        pdTRUE,                 // 自动重载模式
        NULL,                   // 定时器ID
        vTimerCallback          // 回调函数
    );
    
    // 启动定时器
    xTimerStart(xPeriodicTimer, 0);
    
    // 删除当前任务
    vTaskDelete(NULL);
}

/**
 * @brief 串口2数据处理任务
 * @param pvParameters 任务参数
 */
void vTask1(void *pvParameters)
{
    uart2_packet_t packet;      // 串口2数据包
    BaseType_t xResult;         // 操作结果状态
    int ZuoBiao[4] = {0,0,0,0}; // 坐标数据数组
      

		
		
    for(;;)
    {
        // 等待接收串口2数据包
        xResult = Serial2_ReceivePacket(&packet, portMAX_DELAY);
        
        if(xResult == pdTRUE)  // 成功接收到数据包
        {
            
            // 从字符串中提取数字到坐标数组
            ExtractNumbersFromString(packet.data, ZuoBiao, 4);
            

					 xQueueSend(xMotorQueue, ZuoBiao, 0);
        }
    }
}

/**
 * @brief 按键扫描任务
 * @param pvParameters 任务参数
 */
void vTask3(void *pvParameters)
{
	

    unsigned char Key_Down = 0;   // 按键按下检测
    unsigned char Key_Now = 0;    // 当前按键状态
    unsigned char Key_Before = 0; // 上次按键状态
    OLED_Init();
    for(;;)
    {
        // 按键状态边缘检测
        Key_Before = Key_Now;
        Key_Now = key_get();
        Key_Down = Key_Now & (Key_Now ^ Key_Before);

        // 检测到按键1按下
        if(Key_Down == 5)
        {
            GPIO_ToggleBits(GPIOB, GPIO_Pin_2);  // 切换LED状态
        }
				        if(Key_Down == 1)
        {
            GPIO_ToggleBits(GPIOE, GPIO_Pin_3);  // 切换LED状态
        }
				        if(Key_Down == 2)
        {
            GPIO_ToggleBits(GPIOE, GPIO_Pin_4);  // 切换LED状态
        }
				        if(Key_Down == 3)
        {
            GPIO_ToggleBits(GPIOE, GPIO_Pin_5);  // 切换LED状态
        }
				        if(Key_Down == 4)
        {
            GPIO_ToggleBits(GPIOB, GPIO_Pin_2);  // 切换LED状态
					 GPIO_ToggleBits(GPIOE, GPIO_Pin_5);  // 切换LED状态
					 GPIO_ToggleBits(GPIOE, GPIO_Pin_3);  // 切换LED状态
        }
        OLED_ShowNum(1,1,Key_Now,2);
        // 延时10ms进行下一次扫描
        vTaskDelay(10 / portTICK_RATE_MS);
    }
}

/**
 * @brief LED控制任务
 * @param pvParameters 任务参数
 */
void vTask2(void *pvParameters)
{
    for(;;)
    {
        // 切换LED状态
        GPIO_ToggleBits(GPIOB, GPIO_Pin_2);
        GPIO_ToggleBits(GPIOE, GPIO_Pin_3);
			GPIO_ToggleBits(GPIOE, GPIO_Pin_4);
			GPIO_ToggleBits(GPIOE, GPIO_Pin_5);
        // 延时1000ms
        vTaskDelay(1000 / portTICK_RATE_MS);
    }
}

/**
 * @brief 串口1数据处理任务
 * @param pvParameters 任务参数
 */
void vTaskUART(void *pvParameters)
{
    uart_packet_t packet;  // 串口数据包
    BaseType_t xResult;    // 操作结果状态
    
    for(;;)
    {
        // 等待接收串口1数据包
        xResult = Serial_ReceivePacket(&packet, portMAX_DELAY);
        
        if(xResult == pdTRUE)  // 成功接收到数据包
        {
            // 打印接收到的数据
            Serial_Printf("收到串口命令: %s, 数据长度: %d\r\n", packet.data, packet.length);
        }
        else  // 数据包接收失败
        {
            Serial_SendString("错误：串口数据包接收失败\r\n");
        }
    }
}


/**
 * @brief MPU6050传感器数据读取任务
 * @param pvParameters 任务参数
 */
void vTaskMpu9250(void *pvParameters)
{
    MPU6050_Data data;
    Angles angles;
	
//	    MPU6050_Init();
	
	
    for(;;) 
	{
//		      MPU6050_Read_Data(&data);
//        MPU6050_Convert_To_Real(&data);
//        
//        if(!MPU6050_Is_Calibrating()) 
//					{
//            MPU6050_Calculate_Angles(&data, &angles);
//						Serial_Printf("%.1f,%.1f,%.1f\n", 
//                             angles.pitch, angles.roll, angles.yaw);
//        }
					 vTaskDelay(5 / portTICK_PERIOD_MS);  // 5ms延时，200Hz
    }
}




void vMoterTask(void *pvParameters)
{
    int ZuoBiao[4];
	int MuBiao_X=400;
	int MuBiao_Y=200;
    double error_Distance=0;
	double error_JiaoDu=0;
	double error_JiaoDu_Sum=0;
    // 声明卡尔曼滤波器参数和实例
    Vector2 initial_state = {{0.0, 0.0}};
    Matrix2x2 initial_covariance = {{{10.0, 0.0}, {0.0, 50.0}}};
    Matrix2x2 F_x = {{{1.0, 10}, {0.0, 1.0}}};
    Matrix2x2 F_y = {{{1.0, 5}, {0.0, 1.0}}};
    Matrix2x2 Q = {{{0.1, 0.0}, {0.0, 0.1}}};
    double R = 2.0;
  int PL=0;
    KalmanFilter kf_x, kf_y;
		 // 声明用于存储下一时刻位置的变量
    double next_x, next_y;
    double Distance=0;
		double JiaoDu=0;
    // 初始化卡尔曼滤波器（只需一次）
    kalman_init(&kf_x, &initial_state, &initial_covariance, &F_x, &Q, R);
    kalman_init(&kf_y, &initial_state, &initial_covariance, &F_y, &Q, R);
    
    for(;;) 
    {
        if(xQueueReceive(xMotorQueue, ZuoBiao, portMAX_DELAY)==pdPASS)
        {
					if(ZuoBiao[0]!=0)
					{
            // 更新卡尔曼滤波器
            kalman_update(&kf_x, ZuoBiao[0]);  // x坐标
            kalman_update(&kf_y, ZuoBiao[1]);  // y坐标
            kalman_predict(&kf_x);
            kalman_predict(&kf_y);
						next_x = kf_x.x.data[0];           // 下一时刻X位置
            next_y = kf_y.x.data[0];           // 下一时刻Y位置 
					Distance=linear_interpolation_distance(ZuoBiao[3]);
					JiaoDu=linear_interpolation_w(ZuoBiao[3]);
					error_Distance=next_x-MuBiao_X;
					error_Distance=error_Distance/JiaoDu*297;
					error_JiaoDu=atan(error_Distance/(Distance*10))*180/M_PI;
						error_JiaoDu_Sum+=error_JiaoDu;
					PL=angle_to_frequency(error_JiaoDu+error_JiaoDu_Sum*0.1);
            printf("%d,%.2f，%.2f,%d\n", ZuoBiao[0], error_Distance,error_JiaoDu,PL);
            // 使用滤波后的位置进行电机控制
					if(PL<=1000&&PL>=-1000)PL=0;
             Moter_Ctrl_LR(-PL/2);
						}
        }
    }
}

/**
 * @brief 系统状态统计任务
 * @param pvParameters 任务参数
 */
void vStatsTask(void *pvParameters)
{
    char CPU_RunInfo[512];  // 统计信息缓冲区
    
    // 等待系统稳定运行
    vTaskDelay(pdMS_TO_TICKS(2000));
    
    for(;;)
    {
        // 每5秒执行一次统计
        vTaskDelay(pdMS_TO_TICKS(1000));
        
        Serial_SendString("\r\n=== FreeRTOS 任务状态统计 ===\r\n");
        
        // 获取并输出任务列表
        memset(CPU_RunInfo, 0, sizeof(CPU_RunInfo));
        vTaskList(CPU_RunInfo);
        Serial_SendString("任务列表:\r\n");
        Serial_SendString(CPU_RunInfo);
        
        // 获取并输出CPU利用率统计
        memset(CPU_RunInfo, 0, sizeof(CPU_RunInfo));
        vTaskGetRunTimeStats(CPU_RunInfo);
        Serial_SendString("\r\nCPU利用率统计:\r\n");
        Serial_SendString(CPU_RunInfo);
        
        Serial_SendString("==============================\r\n\r\n");
    }
}

/**
 * @brief 软件定时器回调函数
 * @param xTimer 触发回调的定时器句柄
 */
void vTimerCallback(TimerHandle_t xTimer)
{
    // 定时器超时回调函数
    // 注意：此函数在定时器服务任务中执行，不应调用阻塞API
}

