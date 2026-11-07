#ifndef __BLDC_H__
#define __BLDC_H__

#ifdef __cplusplus
extern "C" {
#endif

/* F1 系列 HAL 头文件；若移植到 F4 请改为 stm32f4xx_hal.h，H7 请改为 stm32h7xx_hal.h */
#include "stm32f1xx_hal.h"

/* ============================ 用户配置区 ============================ */

/* PWM 定时器句柄：使用 TIM1 高级定时器（TIM8 亦可），3 路互补 PWM 输出 */
#define BLDC_TIM_HANDLE      htim1

/* 三个 PWM 通道与三相 U/V/W 的对应关系（TIM1 的 CH1/CH2/CH3） */
#define BLDC_CH_U            TIM_CHANNEL_1
#define BLDC_CH_V            TIM_CHANNEL_2
#define BLDC_CH_W            TIM_CHANNEL_3

/* 霍尔传感器三路引脚（接霍尔模块的 HA/HB/HC 输出） */
#define HALL_GPIO_PORT       GPIOB
#define HALL_A_PIN           GPIO_PIN_5
#define HALL_B_PIN           GPIO_PIN_6
#define HALL_C_PIN           GPIO_PIN_7

/* 电机极对数（磁极对数，查电机手册，常见航模无刷为 7） */
#define MOTOR_POLE_PAIRS     7

/* ========================== 用户配置区结束 ========================== */

/* 电机转向 */
typedef enum {
    BLDC_DIR_FORWARD = 0,   /* 正转 */
    BLDC_DIR_REVERSE = 1,   /* 反转 */
} BLDC_Direction;

/* 初始化：启动定时器与 PWM，进入安全状态（所有输出关断、占空比 0） */
void BLDC_Init(void);

/* 可选：用代码初始化霍尔 GPIO + 外部中断；若已在 CubeMX 配置好可跳过 */
void BLDC_Hall_Init(void);

/* 启动 / 停止（停止为关闭所有输出，电机自由滑行） */
void BLDC_Start(void);
void BLDC_Stop(void);

/* 设置占空比：duty_ccr 取值 0 ~ ARR，直接对应比较寄存器 CCR */
void BLDC_Set_Duty(uint16_t duty_ccr);

/* 设置占空比：percent 取值 0.0 ~ 100.0 */
void BLDC_Set_Speed(float percent);

/* 设置转向 */
void BLDC_Set_Direction(BLDC_Direction dir);

/* 读取霍尔状态：返回 0b(HA HB HC) 三位编码，HA 为最高位 */
uint8_t BLDC_Read_Hall(void);

/* 换相处理：在霍尔 EXTI 中断里调用，或主循环里高频轮询调用 */
void BLDC_Commutation(void);

/* 测速：周期性调用（例如每 50ms 调一次，interval_sec = 0.05f），返回机械转速 RPM */
float BLDC_Get_RPM(float interval_sec);

/* 霍尔变化累计次数（调试用） */
uint32_t BLDC_Get_Hall_Count(void);

#ifdef __cplusplus
}
#endif

#endif /* __BLDC_H__ */
