#ifndef __PWM_H
#define __PWM_H

#ifdef __cplusplus
 extern "C" {
#endif

/* ============================ 头文件包含 ============================ */
#include "stm32f4xx.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

/* ============================ 硬件配置定义 ============================ */
// PWM1配置 - TIM3通道1
#define PWM1_TIMER               TIM3
#define PWM1_TIMER_RCC           RCC_APB1Periph_TIM3
#define PWM1_GPIO_PORT           GPIOB
#define PWM1_GPIO_RCC            RCC_AHB1Periph_GPIOB
#define PWM1_PIN                 GPIO_Pin_4
#define PWM1_PIN_SOURCE          GPIO_PinSource4
#define PWM1_AF                  GPIO_AF_TIM3
#define PWM1_CHANNEL             TIM_Channel_1

// PWM2配置 - TIM4通道4  
#define PWM2_TIMER               TIM4
#define PWM2_TIMER_RCC           RCC_APB1Periph_TIM4
#define PWM2_GPIO_PORT           GPIOB
#define PWM2_GPIO_RCC            RCC_AHB1Periph_GPIOB
#define PWM2_PIN                 GPIO_Pin_9
#define PWM2_PIN_SOURCE          GPIO_PinSource9
#define PWM2_AF                  GPIO_AF_TIM4
#define PWM2_CHANNEL             TIM_Channel_4

// 默认PWM参数
#define DEFAULT_PWM_FREQ         1000      // 默认频率1kHz
#define DEFAULT_PWM_DUTY         50.0f     // 默认占空比50%

/* ============================ 类型定义 ============================ */
// PWM命令数据结构
typedef struct {
    uint8_t pwm_id;         // PWM标识 (1或2)
    float duty_cycle;       // 占空比 (0.0 - 100.0)
    uint32_t frequency;     // 频率 (Hz)
    uint8_t command_type;   // 命令类型: 0=设置占空比, 1=设置频率
} PWM_Command_t;

/* ============================ 外部变量声明 ============================ */
extern SemaphoreHandle_t xPWM1Mutex;
extern SemaphoreHandle_t xPWM2Mutex;
extern uint32_t pwm1_period;
extern uint32_t pwm2_period;

/* ============================ 函数声明 ============================ */

/**
  * @brief  PWM硬件统一初始化
  * @note   初始化所有PWM通道和互斥锁
  */
void PWM_Hardware_Init(void);

/**
  * @brief  PWM互斥锁初始化
  */
void PWM_Mutex_Init(void);

/**
  * @brief  PWM1硬件初始化
  */
void PWM1_Hardware_Init(void);

/**
  * @brief  PWM2硬件初始化
  */
void PWM2_Hardware_Init(void);

/**
  * @brief  FreeRTOS安全的PWM占空比设置
  * @param  pwm_id: PWM标识 (1或2)
  * @param  duty_cycle: 占空比百分比 (0.0 - 100.0)
  */
void PWM_SetDuty_RTOS_Safe(uint8_t pwm_id, float duty_cycle);

/**
  * @brief  FreeRTOS安全的PWM频率设置
  * @param  pwm_id: PWM标识 (1或2)
  * @param  frequency_hz: 频率值 (Hz)
  */
void PWM_SetFrequency_RTOS_Safe(uint8_t pwm_id, uint32_t frequency_hz);


#ifdef __cplusplus
}
#endif

#endif /* __PWM_DRIVER_H */