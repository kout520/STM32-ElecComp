#include "stm32f4xx.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "usart3.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

// 队列定义 - 用于存储接收到的数据包
QueueHandle_t xUart3RxQueue = NULL;

// 状态机上下文结构
typedef struct {
    uint8_t state;          // 当前状态
    uint16_t data_index;    // 数据索引
    char buffer[100];       // 接收缓冲区
    uint32_t error_count;   // 错误计数
} uart3_state_machine_t;

static uart3_state_machine_t uart3_sm = {0};

// 串口互斥信号量定义
SemaphoreHandle_t xUart3Mutex = NULL;

void Serial3_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    /* 开启时钟 */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);  // USART3 在 APB1 总线上
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);   // 使用 GPIOB
    
    /* GPIO初始化 */
    // PB10 - USART3_TX
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    
    // PB11 - USART3_RX
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    
    /* 配置引脚复用 */
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource10, GPIO_AF_USART3);  // PB10 复用为 USART3_TX
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource11, GPIO_AF_USART3);  // PB11 复用为 USART3_RX
    
    /* USART初始化 */
    USART_InitStructure.USART_BaudRate = 115200;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_Init(USART3, &USART_InitStructure);
    
    /* 中断配置 */
    USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);
    
    /* NVIC配置 */
    NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;  // 改为 USART3 中断
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 6;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_Init(&NVIC_InitStructure);
    
    /* USART使能 */
    USART_Cmd(USART3, ENABLE);
    
    /* 创建接收队列 */
    xUart3RxQueue = xQueueCreate(10, sizeof(uart3_packet_t));
    if (xUart3RxQueue == NULL) {
        /* 队列创建失败处理 */
        while(1);
    }
    
    /* 创建串口互斥信号量 */
    xUart3Mutex = xSemaphoreCreateMutex();
    if (xUart3Mutex == NULL) {
        /* 互斥量创建失败处理 */
        while(1);
    }
}

/**
  * 函数名：串口发送一个字节
  */
void Serial3_SendByte(uint8_t Byte)
{
    /* 获取串口互斥量 */
    if (xSemaphoreTake(xUart3Mutex, portMAX_DELAY) == pdTRUE) {
        USART_SendData(USART3, Byte);  // 改为 USART3
        while (USART_GetFlagStatus(USART3, USART_FLAG_TXE) == RESET)  // 改为 USART3
        {
            taskYIELD();
        }
        /* 释放互斥量 */
        xSemaphoreGive(xUart3Mutex);
    }
}

/**
  * 函数名：串口发送一个数组
  */
void Serial3_SendArray(uint8_t *Array, uint16_t Length)
{
    /* 获取串口互斥量 */
    if (xSemaphoreTake(xUart3Mutex, portMAX_DELAY) == pdTRUE) {
        uint16_t i;
        for (i = 0; i < Length; i++)
        {
            USART_SendData(USART3, Array[i]);  // 改为 USART3
            while (USART_GetFlagStatus(USART3, USART_FLAG_TXE) == RESET)  // 改为 USART3
            {
                taskYIELD();
            }
        }
        /* 释放互斥量 */
        xSemaphoreGive(xUart3Mutex);
    }
}

/**
  * 函数名：串口发送一个字符串
  */
void Serial3_SendString(char *String)
{
    /* 获取串口互斥量 */
    if (xSemaphoreTake(xUart3Mutex, portMAX_DELAY) == pdTRUE) {
        uint8_t i;
        for (i = 0; String[i] != '\0'; i++)
        {
            USART_SendData(USART3, String[i]);  // 改为 USART3
            while (USART_GetFlagStatus(USART3, USART_FLAG_TXE) == RESET)  // 改为 USART3
            {
                taskYIELD();
            }
        }
        /* 释放互斥量 */
        xSemaphoreGive(xUart3Mutex);
    }
}

/**
  * 函数名：次方函数（内部使用）
  */
uint32_t Serial3_Pow(uint32_t X, uint32_t Y)
{
    uint32_t Result = 1;
    while (Y--)
    {
        Result *= X;
    }
    return Result;
}

/**
  * 函数名：串口发送数字
  */
void Serial3_SendNumber(uint32_t Number, uint8_t Length)
{
    /* 获取串口互斥量 */
    if (xSemaphoreTake(xUart3Mutex, portMAX_DELAY) == pdTRUE) {
        uint8_t i;
        for (i = 0; i < Length; i++)
        {
            USART_SendData(USART3, Number / Serial3_Pow(10, Length - i - 1) % 10 + '0');  // 改为 USART3
            while (USART_GetFlagStatus(USART3, USART_FLAG_TXE) == RESET)  // 改为 USART3
            {
                taskYIELD();
            }
        }
        /* 释放互斥量 */
        xSemaphoreGive(xUart3Mutex);
    }
}

/**
  * 函数名：串口发送浮点数
  */
void Serial3_SendFloat(float value, uint8_t decimal_places)
{
    /* 获取串口互斥量 */
    if (xSemaphoreTake(xUart3Mutex, portMAX_DELAY) == pdTRUE) {
        uint32_t integer_part;
        float fractional_part;
        uint32_t temp;
        uint8_t digits;
        uint8_t i;
        uint8_t digit;
        
        // 处理负数
        if (value < 0) {
            USART_SendData(USART3, '-');  // 改为 USART3
            while (USART_GetFlagStatus(USART3, USART_FLAG_TXE) == RESET) {  // 改为 USART3
                taskYIELD();
            }
            value = -value;
        }
        
        // 提取整数部分
        integer_part = (uint32_t)value;
        fractional_part = value - integer_part;
        
        // 发送整数部分
        temp = integer_part;
        digits = 0;
        
        // 计算整数位数
        do {
            digits++;
            temp /= 10;
        } while (temp > 0);
        
        // 发送整数部分
        temp = integer_part;
        for (i = digits; i > 0; i--) {
            uint32_t divisor = Serial3_Pow(10, i - 1);
            digit = (uint8_t)(temp / divisor);
            USART_SendData(USART3, digit + '0');  // 改为 USART3
            while (USART_GetFlagStatus(USART3, USART_FLAG_TXE) == RESET) {  // 改为 USART3
                taskYIELD();
            }
            temp %= divisor;
        }
        
        // 发送小数点
        if (decimal_places > 0) {
            USART_SendData(USART3, '.');  // 改为 USART3
            while (USART_GetFlagStatus(USART3, USART_FLAG_TXE) == RESET) {  // 改为 USART3
                taskYIELD();
            }
            
            // 发送小数部分
            for (i = 0; i < decimal_places; i++) {
                fractional_part *= 10;
                digit = (uint8_t)fractional_part;
                USART_SendData(USART3, digit + '0');  // 改为 USART3
                while (USART_GetFlagStatus(USART3, USART_FLAG_TXE) == RESET) {  // 改为 USART3
                    taskYIELD();
                }
                fractional_part -= digit;
            }
        }
        
        /* 释放互斥量 */
        xSemaphoreGive(xUart3Mutex);
    }
}

/**
  * 函数名：使用printf需要重定向的底层函数
  */
int fputc3(int ch, FILE *f)
{
    Serial3_SendByte(ch);
    return ch;
}

/**
  * 函数名：自己封装的printf函数
  */
void Serial3_Printf(char *format, ...)
{
    char String[100];
    va_list arg;
    va_start(arg, format);
    vsprintf(String, format, arg);
    va_end(arg);
    Serial3_SendString(String);
}

// ==================== 接收相关函数 ====================

/**
  * 函数名：接收数据包（任务调用）
  */
BaseType_t Serial3_ReceivePacket(uart3_packet_t *packet, TickType_t xTicksToWait)
{
    if (packet == NULL || xUart3RxQueue == NULL)
    {
        return pdFALSE;
    }
    
    return xQueueReceive(xUart3RxQueue, packet, xTicksToWait);
}

/**
  * 函数名：USART3中断函数
  */
void USART3_IRQHandler(void)  // 改为 USART3 中断处理函数
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    
    // 检查接收中断标志
    if (USART_GetITStatus(USART3, USART_IT_RXNE) == SET)  // 改为 USART3
    {
        uint8_t RxData = USART_ReceiveData(USART3);  // 改为 USART3
        
        // 状态机处理
        switch (uart3_sm.state)
        {
            case 0: // 等待包头
                if (RxData == '@')
                {
                    uart3_sm.state = 1;
                    uart3_sm.data_index = 0;
                    uart3_sm.buffer[0] = '\0';
                }
                break;
                
            case 1: // 接收数据
                if (RxData == '\r')
                {
                    uart3_sm.state = 2;
                }
                else if (uart3_sm.data_index < (sizeof(uart3_sm.buffer) - 1))
                {
                    uart3_sm.buffer[uart3_sm.data_index++] = RxData;
                }
                else
                {
                    // 缓冲区溢出
                    uart3_sm.error_count++;
                    uart3_sm.state = 0;
                }
                break;
                
            case 2: // 等待包尾
                if (RxData == '\n')
                {
                    // 数据包接收完成
                    if (uart3_sm.data_index > 0)
                    {
                        uart3_packet_t packet;
                        packet.length = uart3_sm.data_index;
                        packet.timestamp = xTaskGetTickCountFromISR();
                        memcpy(packet.data, uart3_sm.buffer, uart3_sm.data_index);
                        packet.data[packet.length] = '\0';
                        
                        // 发送到队列
                        if (xQueueSendFromISR(xUart3RxQueue, &packet, &xHigherPriorityTaskWoken) != pdTRUE)
                        {
                            uart3_sm.error_count++;
                        }
                    }
                    else
                    {
                        uart3_sm.error_count++;
                    }
                    uart3_sm.state = 0;
                }
                else
                {
                    // 协议错误
                    uart3_sm.error_count++;
                    uart3_sm.state = 0;
                }
                break;
                
            default:
                uart3_sm.state = 0;
                break;
        }
    }
    
    // 处理过载错误
    if (USART_GetITStatus(USART3, USART_IT_ORE) == SET)  // 改为 USART3
    {
        (void)USART3->SR;  // 改为 USART3
        (void)USART3->DR;  // 改为 USART3
        USART_ClearITPendingBit(USART3, USART_IT_ORE);  // 改为 USART3
        uart3_sm.error_count++;
    }
    
    // 如果有更高优先级任务被唤醒，进行上下文切换
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}