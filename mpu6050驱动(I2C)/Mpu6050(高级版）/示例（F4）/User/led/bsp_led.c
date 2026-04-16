/**
  ******************************************************************************
  * @file    bsp_led.c
  * @author  fire
  * @version V1.0
  * @date    2015-xx-xx
  * @brief   led应用函数接口
  ******************************************************************************
  * @attention
  *
  * 实验平台:野火  STM32 F407 开发板  
  * 论坛    :http://www.firebbs.cn
  * 淘宝    :https://fire-stm32.taobao.com
  *
  ******************************************************************************
  */
  
#include "./led/bsp_led.h"   


void led_init(void)
{ 
    GPIO_InitTypeDef GPIO_InitStructure;
    
    /* 使能端口B、E和D时钟 */
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOE | RCC_AHB1Periph_GPIOD, ENABLE);    
    
    /* 配置PB2引脚 */
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_2;               /* 使用PB2引脚 */
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
    
    /* 初始化GPIOB */
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    
    /* 配置PE5、PE3、PE4引脚 */
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_5 | GPIO_Pin_3 | GPIO_Pin_4;  /* 使用PE5、PE3、PE4引脚 */
    
    /* 初始化GPIOE */
    GPIO_Init(GPIOE, &GPIO_InitStructure);
    
    /* 配置PD15、PD14引脚 */
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_15 | GPIO_Pin_14;  /* 使用PD15、PD14引脚 */
    
    /* 初始化GPIOD */
    GPIO_Init(GPIOD, &GPIO_InitStructure);
    
    /* 设置所有LED引脚输出高电平（LED熄灭） */
    GPIO_SetBits(GPIOB, GPIO_Pin_2);   
    GPIO_SetBits(GPIOE, GPIO_Pin_5 | GPIO_Pin_3 | GPIO_Pin_4);   
    GPIO_SetBits(GPIOD, GPIO_Pin_15 | GPIO_Pin_14);
}


