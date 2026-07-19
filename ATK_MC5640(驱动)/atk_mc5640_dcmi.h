/**
 ****************************************************************************************************
 * @file        atk_mc5640_dcmi.h
 * @author      正点原子团队(ALIENTEK)
 * @version     V1.0
 * @date        2022-06-21
 * @brief       ATK-MC5640模块DCMI接口驱动代码
 * @license     Copyright (c) 2020-2032, 广州市星翼电子科技有限公司
 ****************************************************************************************************
 * @attention
 *
 * 实验平台:正点原子 探索者 F407开发板
 * 在线视频:www.yuanzige.com
 * 技术论坛:www.openedv.com
 * 公司网址:www.alientek.com
 * 购买地址:openedv.taobao.com
 *
 ****************************************************************************************************
 */

#ifndef __ATK_MC5640_DCMI_H
#define __ATK_MC5640_DCMI_H

#include "./BSP/ATK_MC5640/atk_mc5640.h"

/* DCMI接口定义 */
#define ATK_MC5640_DCMI_INTERFACE                   DCMI
#define ATK_MC5640_DCMI_IRQn                        DCMI_IRQn
#define ATK_MC5640_DCMI_IRQHandler                  DCMI_IRQHandler
#define ATK_MC5640_DCMI_CLK_ENABLE()                do{ __HAL_RCC_DCMI_CLK_ENABLE(); }while(0)

/* DCMI接口DMA定义 */
#define ATK_MC5640_DCMI_DMA_INTERFACE               DMA2_Stream1
#define ATK_MC5640_DCMI_DMA_CHANNEL                 DMA_CHANNEL_1
#define ATK_MC5640_DCMI_DMA_IRQn                    DMA2_Stream1_IRQn
#define ATK_MC5640_DCMI_DMA_IRQHandler              DMA2_Stream1_IRQHandler
#define ATK_MC5640_DCMI_DMA_CLK_ENABLE()            do{ __HAL_RCC_DMA2_CLK_ENABLE(); }while(0)

/* 引脚定义 */
#define ATK_MC5640_DCMI_VSYNC_GPIO_PORT             GPIOB
#define ATK_MC5640_DCMI_VSYNC_GPIO_PIN              GPIO_PIN_7
#define ATK_MC5640_DCMI_VSYNC_GPIO_AF               GPIO_AF13_DCMI
#define ATK_MC5640_DCMI_VSYNC_GPIO_CLK_ENABLE()     do{ __HAL_RCC_GPIOB_CLK_ENABLE(); }while(0)
#define ATK_MC5640_DCMI_HSYNC_GPIO_PORT             GPIOA
#define ATK_MC5640_DCMI_HSYNC_GPIO_PIN              GPIO_PIN_4
#define ATK_MC5640_DCMI_HSYNC_GPIO_AF               GPIO_AF13_DCMI
#define ATK_MC5640_DCMI_HSYNC_GPIO_CLK_ENABLE()     do{ __HAL_RCC_GPIOA_CLK_ENABLE(); }while(0)
#define ATK_MC5640_DCMI_PIXCLK_GPIO_PORT            GPIOA
#define ATK_MC5640_DCMI_PIXCLK_GPIO_PIN             GPIO_PIN_6
#define ATK_MC5640_DCMI_PIXCLK_GPIO_AF              GPIO_AF13_DCMI
#define ATK_MC5640_DCMI_PIXCLK_GPIO_CLK_ENABLE()    do{ __HAL_RCC_GPIOA_CLK_ENABLE(); }while(0)
#define ATK_MC5640_DCMI_D0_GPIO_PORT                GPIOC
#define ATK_MC5640_DCMI_D0_GPIO_PIN                 GPIO_PIN_6
#define ATK_MC5640_DCMI_D0_GPIO_AF                  GPIO_AF13_DCMI
#define ATK_MC5640_DCMI_D0_GPIO_CLK_ENABLE()        do{ __HAL_RCC_GPIOC_CLK_ENABLE(); }while(0)
#define ATK_MC5640_DCMI_D1_GPIO_PORT                GPIOC
#define ATK_MC5640_DCMI_D1_GPIO_PIN                 GPIO_PIN_7
#define ATK_MC5640_DCMI_D1_GPIO_AF                  GPIO_AF13_DCMI
#define ATK_MC5640_DCMI_D1_GPIO_CLK_ENABLE()        do{ __HAL_RCC_GPIOC_CLK_ENABLE(); }while(0)
#define ATK_MC5640_DCMI_D2_GPIO_PORT                GPIOC
#define ATK_MC5640_DCMI_D2_GPIO_PIN                 GPIO_PIN_8
#define ATK_MC5640_DCMI_D2_GPIO_AF                  GPIO_AF13_DCMI
#define ATK_MC5640_DCMI_D2_GPIO_CLK_ENABLE()        do{ __HAL_RCC_GPIOC_CLK_ENABLE(); }while(0)
#define ATK_MC5640_DCMI_D3_GPIO_PORT                GPIOC
#define ATK_MC5640_DCMI_D3_GPIO_PIN                 GPIO_PIN_9
#define ATK_MC5640_DCMI_D3_GPIO_AF                  GPIO_AF13_DCMI
#define ATK_MC5640_DCMI_D3_GPIO_CLK_ENABLE()        do{ __HAL_RCC_GPIOC_CLK_ENABLE(); }while(0)
#define ATK_MC5640_DCMI_D4_GPIO_PORT                GPIOC
#define ATK_MC5640_DCMI_D4_GPIO_PIN                 GPIO_PIN_11
#define ATK_MC5640_DCMI_D4_GPIO_AF                  GPIO_AF13_DCMI
#define ATK_MC5640_DCMI_D4_GPIO_CLK_ENABLE()        do{ __HAL_RCC_GPIOC_CLK_ENABLE(); }while(0)
#define ATK_MC5640_DCMI_D5_GPIO_PORT                GPIOB
#define ATK_MC5640_DCMI_D5_GPIO_PIN                 GPIO_PIN_6
#define ATK_MC5640_DCMI_D5_GPIO_AF                  GPIO_AF13_DCMI
#define ATK_MC5640_DCMI_D5_GPIO_CLK_ENABLE()        do{ __HAL_RCC_GPIOB_CLK_ENABLE(); }while(0)
#define ATK_MC5640_DCMI_D6_GPIO_PORT                GPIOE
#define ATK_MC5640_DCMI_D6_GPIO_PIN                 GPIO_PIN_5
#define ATK_MC5640_DCMI_D6_GPIO_AF                  GPIO_AF13_DCMI
#define ATK_MC5640_DCMI_D6_GPIO_CLK_ENABLE()        do{ __HAL_RCC_GPIOE_CLK_ENABLE(); }while(0)
#define ATK_MC5640_DCMI_D7_GPIO_PORT                GPIOE
#define ATK_MC5640_DCMI_D7_GPIO_PIN                 GPIO_PIN_6
#define ATK_MC5640_DCMI_D7_GPIO_AF                  GPIO_AF13_DCMI
#define ATK_MC5640_DCMI_D7_GPIO_CLK_ENABLE()        do{ __HAL_RCC_GPIOE_CLK_ENABLE(); }while(0)

/* 错误代码 */
#define ATK_MC5640_DCMI_EOK     0   /* 没有错误 */
#define ATK_MC5640_DCMI_ERROR   1   /* 错误 */
#define ATK_MC5640_DCMI_EINVAL  2   /* 非法参数 */

/* 操作函数 */
void atk_mc5640_dcmi_init(void);                                                                            /* 初始化ATK-MC5640模块DCMI接口 */
void atk_mc5640_dcmi_start(uint32_t dts_addr, uint32_t meminc, uint32_t memdataalignment, uint32_t len);    /* 开始ATK-MC5640模块DCMI接口DMA传输 */

#endif

