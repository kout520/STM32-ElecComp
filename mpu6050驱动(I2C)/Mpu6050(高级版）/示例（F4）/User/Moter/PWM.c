#include "stm32f4xx.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "PWM.h"

// PWM硬件资源互斥锁（每个PWM独立）
SemaphoreHandle_t xPWM1Mutex;
SemaphoreHandle_t xPWM2Mutex;

// PWM周期值
uint32_t pwm1_period = 0;
uint32_t pwm2_period = 0;

// 当前占空比缓存
float pwm1_duty = DEFAULT_PWM_DUTY;
float pwm2_duty = DEFAULT_PWM_DUTY;

/**
  * @brief  PWM互斥锁初始化
  */
void PWM_Mutex_Init(void)
{
    xPWM1Mutex = xSemaphoreCreateMutex();
    xPWM2Mutex = xSemaphoreCreateMutex();
}

/**
  * @brief  PWM1硬件初始化函数
  */
void PWM1_Hardware_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_OCInitTypeDef TIM_OCInitStructure;
    
    /* GPIO引脚配置 */
    RCC_AHB1PeriphClockCmd(PWM1_GPIO_RCC, ENABLE);
    
    GPIO_InitStructure.GPIO_Pin = PWM1_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(PWM1_GPIO_PORT, &GPIO_InitStructure);
    
    GPIO_PinAFConfig(PWM1_GPIO_PORT, PWM1_PIN_SOURCE, PWM1_AF);
    
    /* 定时器基础配置 */
    RCC_APB1PeriphClockCmd(PWM1_TIMER_RCC, ENABLE);
    
    TIM_TimeBaseStructure.TIM_Prescaler = 83;
    TIM_TimeBaseStructure.TIM_Period = (2000000 / DEFAULT_PWM_FREQ) - 1;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit(PWM1_TIMER, &TIM_TimeBaseStructure);
    
    pwm1_period = TIM_TimeBaseStructure.TIM_Period;
    
    /* PWM输出通道配置 */
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_OutputNState = TIM_OutputNState_Disable;
    TIM_OCInitStructure.TIM_Pulse = (pwm1_period * DEFAULT_PWM_DUTY) / 100;
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
    TIM_OCInitStructure.TIM_OCIdleState = TIM_OCIdleState_Reset;
    
    TIM_OC1Init(PWM1_TIMER, &TIM_OCInitStructure);
    TIM_OC1PreloadConfig(PWM1_TIMER, TIM_OCPreload_Enable);
    
    /* 启动定时器 */
    TIM_ARRPreloadConfig(PWM1_TIMER, ENABLE);
    TIM_Cmd(PWM1_TIMER, ENABLE);
    TIM_CtrlPWMOutputs(PWM1_TIMER, ENABLE);
}

/**
  * @brief  PWM2硬件初始化函数
  */
void PWM2_Hardware_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_OCInitTypeDef TIM_OCInitStructure;
    
    /* GPIO引脚配置 */
    RCC_AHB1PeriphClockCmd(PWM2_GPIO_RCC, ENABLE);
    
    GPIO_InitStructure.GPIO_Pin = PWM2_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(PWM2_GPIO_PORT, &GPIO_InitStructure);
    
    GPIO_PinAFConfig(PWM2_GPIO_PORT, PWM2_PIN_SOURCE, PWM2_AF);
    
    /* 定时器基础配置 */
    RCC_APB1PeriphClockCmd(PWM2_TIMER_RCC, ENABLE);
    
    TIM_TimeBaseStructure.TIM_Prescaler = 83;
    TIM_TimeBaseStructure.TIM_Period = (2000000 / DEFAULT_PWM_FREQ) - 1;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit(PWM2_TIMER, &TIM_TimeBaseStructure);
    
    pwm2_period = TIM_TimeBaseStructure.TIM_Period;
    
    /* PWM输出通道配置 */
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_OutputNState = TIM_OutputNState_Disable;
    TIM_OCInitStructure.TIM_Pulse = (pwm2_period * DEFAULT_PWM_DUTY) / 100;
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
    TIM_OCInitStructure.TIM_OCIdleState = TIM_OCIdleState_Reset;
    
    // 修改为通道4初始化
    TIM_OC4Init(PWM2_TIMER, &TIM_OCInitStructure);
    TIM_OC4PreloadConfig(PWM2_TIMER, TIM_OCPreload_Enable);
    
    /* 启动定时器 */
    TIM_ARRPreloadConfig(PWM2_TIMER, ENABLE);
    TIM_Cmd(PWM2_TIMER, ENABLE);
    TIM_CtrlPWMOutputs(PWM2_TIMER, ENABLE);
}

/**
  * @brief  FreeRTOS安全的PWM占空比设置函数
  */
void PWM_SetDuty_RTOS_Safe(uint8_t pwm_id, float duty_cycle)
{
    if(duty_cycle > 100.0f) duty_cycle = 100.0f;
    if(duty_cycle < 0.0f) duty_cycle = 0.0f;
    
    switch(pwm_id)
    {
        case 1:
            if(xSemaphoreTake(xPWM1Mutex, portMAX_DELAY) == pdTRUE)
            {
                uint32_t compare_value = (uint32_t)((pwm1_period * duty_cycle) / 100.0f);
                PWM1_TIMER->CCR1 = compare_value;
                pwm1_duty = duty_cycle;  // 更新占空比缓存
                xSemaphoreGive(xPWM1Mutex);
            }
            break;
            
        case 2:
            if(xSemaphoreTake(xPWM2Mutex, portMAX_DELAY) == pdTRUE)
            {
                uint32_t compare_value = (uint32_t)((pwm2_period * duty_cycle) / 100.0f);
                // 修改为通道4的比较寄存器
                PWM2_TIMER->CCR4 = compare_value;
                pwm2_duty = duty_cycle;  // 更新占空比缓存
                xSemaphoreGive(xPWM2Mutex);
            }
            break;
            
        default:
            break;
    }
}

/**
  * @brief  FreeRTOS安全的PWM频率设置函数
  */
void PWM_SetFrequency_RTOS_Safe(uint8_t pwm_id, uint32_t frequency_hz)
{
    uint32_t new_period;
    uint32_t compare_value;
    
    if(frequency_hz < 1) frequency_hz = 1;
    if(frequency_hz > 100000) frequency_hz = 100000;
    
    switch(pwm_id)
    {
        case 1:
            if(xSemaphoreTake(xPWM1Mutex, portMAX_DELAY) == pdTRUE)
            {
                new_period = (2000000 / frequency_hz) - 1;
                PWM1_TIMER->CR1 &= ~TIM_CR1_CEN;
                PWM1_TIMER->ARR = new_period;
                pwm1_period = new_period;
                
                // 重新设置占空比，保持50%
                compare_value = (uint32_t)((pwm1_period * 50.0f) / 100.0f);
                PWM1_TIMER->CCR1 = compare_value;
                pwm1_duty = 50.0f;  // 更新缓存为50%
                
                PWM1_TIMER->CR1 |= TIM_CR1_CEN;
                xSemaphoreGive(xPWM1Mutex);
            }
            break;
            
        case 2:
            if(xSemaphoreTake(xPWM2Mutex, portMAX_DELAY) == pdTRUE)
            {
                new_period = (2000000 / frequency_hz) - 1;
                PWM2_TIMER->CR1 &= ~TIM_CR1_CEN;
                PWM2_TIMER->ARR = new_period;
                pwm2_period = new_period;
                
                // 重新设置占空比，保持50%
                compare_value = (uint32_t)((pwm2_period * 50.0f) / 100.0f);
                PWM2_TIMER->CCR4 = compare_value;
                pwm2_duty = 50.0f;  // 更新缓存为50%
                
                PWM2_TIMER->CR1 |= TIM_CR1_CEN;
                xSemaphoreGive(xPWM2Mutex);
            }
            break;
            
        default:
            break;
    }
}
/**
  * @brief  获取当前PWM占空比
  */
float PWM_GetDuty(uint8_t pwm_id)
{
    switch(pwm_id)
    {
        case 1:
            return pwm1_duty;
        case 2:
            return pwm2_duty;
        default:
            return 0.0f;
    }
}

/**
  * @brief  PWM硬件统一初始化函数
  * @note   初始化所有PWM通道和互斥锁
  */
void PWM_Hardware_Init(void)
{
    // 初始化互斥锁
    PWM_Mutex_Init();
    
    // 初始化PWM1硬件
    PWM1_Hardware_Init();
    
    // 初始化PWM2硬件  
    PWM2_Hardware_Init();
}