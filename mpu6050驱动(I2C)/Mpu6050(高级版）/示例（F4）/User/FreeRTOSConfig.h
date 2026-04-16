#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

#include "stm32f4xx.h"
#include <stdio.h>

//针对不同的编译器调用不同的 stdint.h 文件
#if defined(__ICCARM__) || defined(__CC_ARM) || defined(__GNUC__)
#include <stdint.h>
extern uint32_t SystemCoreClock;
#endif

//断言 - 添加extern声明避免隐式函数声明警告
extern void vAssertCalled(const char *file, int line);
#define vAssertCalled(char,int) printf("Error:%s,%d\r\n",char,int)
#define configASSERT(x) if((x)==0) vAssertCalled(__FILE__,__LINE__)

/************************************************************************
 * FreeRTOS 基础配置配置选项
 *********************************************************************/
/* 置 1：RTOS 使用抢占式调度器；置 0：RTOS 使用协作式调度器（时间片）*/
#define configUSE_PREEMPTION 1

//1 使能时间片调度(默认式使能的)
#define configUSE_TIME_SLICING 1

/* 使用硬件优化任务选择 */
#define configUSE_PORT_OPTIMISED_TASK_SELECTION 1

/* 置 1：使能低功耗 tickless 模式；置 0：保持系统节拍（tick）中断一直运行 */
#define configUSE_TICKLESS_IDLE 0

/* CPU 内核时钟频率 */
#define configCPU_CLOCK_HZ (SystemCoreClock)

//RTOS 系统节拍中断的频率。即一秒中断的次数，每次中断 RTOS 都会进行任务调度
#define configTICK_RATE_HZ (( TickType_t )1000)

//可使用的最大优先级
#define configMAX_PRIORITIES (32)

//空闲任务使用的堆栈大小
#define configMINIMAL_STACK_SIZE ((unsigned short)128)

//任务名字字符串长度
#define configMAX_TASK_NAME_LEN (16)

//系统节拍计数器变量数据类型，1 表示为 16 位无符号整形，0 表示为 32 位无符号整形
#define configUSE_16_BIT_TICKS 0

//空闲任务放弃 CPU 使用权给其他同优先级的用户任务
#define configIDLE_SHOULD_YIELD 1

//启用队列
#define configUSE_QUEUE_SETS 1

//开启任务通知功能，默认开启
#define configUSE_TASK_NOTIFICATIONS 1

//使用互斥信号量
#define configUSE_MUTEXES 1

//使用递归互斥信号量
#define configUSE_RECURSIVE_MUTEXES 1

//为 1 时使用计数信号量
#define configUSE_COUNTING_SEMAPHORES 1

/* 设置可以注册的信号量和消息队列个数 */
#define configQUEUE_REGISTRY_SIZE 10

#define configUSE_APPLICATION_TASK_TAG 0

/*****************************************************************
 FreeRTOS 与内存申请有关配置选项
 *****************************************************************/
//支持动态内存申请
#define configSUPPORT_DYNAMIC_ALLOCATION 1
//系统所有总的堆大小
#define configTOTAL_HEAP_SIZE ((size_t)(36*1024))

/***************************************************************
 FreeRTOS 与钩子函数有关的配置选项
 **************************************************************/
/* 置 1：使用空闲钩子；置 0：忽略空闲钩子 */
#define configUSE_IDLE_HOOK 0

/* 置 1：使用时间片钩子；置 0：忽略时间片钩子 */
#define configUSE_TICK_HOOK 0

//使用内存申请失败钩子函数
#define configUSE_MALLOC_FAILED_HOOK 0

/* 栈溢出检测功能 */
#define configCHECK_FOR_STACK_OVERFLOW 0

/********************************************************************
 FreeRTOS 与运行时间和任务状态收集有关的配置选项
 **********************************************************************/
//启用运行时间统计功能
#define configGENERATE_RUN_TIME_STATS 1
//启用可视化跟踪调试
#define configUSE_TRACE_FACILITY 1

#define configUSE_STATS_FORMATTING_FUNCTIONS 1

/********************************************************************
 FreeRTOS 与协程有关的配置选项
 *********************************************************************/
//启用协程，启用协程以后必须添加文件 croutine.c
#define configUSE_CO_ROUTINES 0
//协程的有效优先级数目
#define configMAX_CO_ROUTINE_PRIORITIES ( 2 )

/***********************************************************************
 FreeRTOS 与软件定时器有关的配置选项
 **********************************************************************/
//启用软件定时器
#define configUSE_TIMERS 1
//软件定时器优先级
#define configTIMER_TASK_PRIORITY (configMAX_PRIORITIES-1)
//软件定时器队列长度
#define configTIMER_QUEUE_LENGTH 10
//软件定时器任务堆栈大小
#define configTIMER_TASK_STACK_DEPTH (configMINIMAL_STACK_SIZE*2)

/************************************************************
 FreeRTOS 可选函数配置选项
 ************************************************************/
#define INCLUDE_xTaskGetSchedulerState 1
#define INCLUDE_vTaskPrioritySet 1
#define INCLUDE_uxTaskPriorityGet 1
#define INCLUDE_vTaskDelete 1
#define INCLUDE_vTaskCleanUpResources 1
#define INCLUDE_vTaskSuspend 1
#define INCLUDE_vTaskDelayUntil 1
#define INCLUDE_vTaskDelay 1
#define INCLUDE_eTaskGetState 1
#define INCLUDE_xTimerPendFunctionCall 1

/******************************************************************
 FreeRTOS 与中断有关的配置选项
 ******************************************************************/
#ifdef __NVIC_PRIO_BITS
#define configPRIO_BITS __NVIC_PRIO_BITS
#else
#define configPRIO_BITS 4
#endif

//中断最低优先级
#define configLIBRARY_LOWEST_INTERRUPT_PRIORITY 15

//系统可管理的最高中断优先级
#define configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY 5

#define configKERNEL_INTERRUPT_PRIORITY \
    ( configLIBRARY_LOWEST_INTERRUPT_PRIORITY << (8 - configPRIO_BITS) )
    
#define configMAX_SYSCALL_INTERRUPT_PRIORITY \
    ( configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY << (8 - configPRIO_BITS) )

/****************************************************************
 FreeRTOS 与中断服务函数有关的配置选项
 ****************************************************************/
#define xPortPendSVHandler PendSV_Handler
#define vPortSVCHandler SVC_Handler
#define xPortSysTickHandler SysTick_Handler


/* 运行时间统计配置 */
extern unsigned long getRunTimeCounterValue(void);
extern void vInitStatsTimer(void);

#define portCONFIGURE_TIMER_FOR_RUN_TIME_STATS() vInitStatsTimer()
#define portGET_RUN_TIME_COUNTER_VALUE() getRunTimeCounterValue()




#endif /* FREERTOS_CONFIG_H */