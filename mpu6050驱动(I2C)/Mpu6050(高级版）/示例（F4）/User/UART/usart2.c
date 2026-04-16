#include "stm32f4xx.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "usart2.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

// 队列定义 - 用于存储接收到的数据包
QueueHandle_t xUart2RxQueue = NULL;

// 状态机上下文结构
typedef struct {
    uint8_t state;          // 当前状态
    uint16_t data_index;    // 数据索引
    char buffer[100];       // 接收缓冲区
    uint32_t error_count;   // 错误计数
} uart2_state_machine_t;

static uart2_state_machine_t uart2_sm = {0};

// 串口互斥信号量定义
SemaphoreHandle_t xUart2Mutex = NULL;

void Serial2_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    /* 开启时钟 */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
    
    /* GPIO初始化 */
    // PA2 - USART2_TX
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    // PA3 - USART2_RX
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    /* 配置引脚复用 */
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_USART2);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource3, GPIO_AF_USART2);
    
    /* USART初始化 */
    USART_InitStructure.USART_BaudRate = 115200;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_Init(USART2, &USART_InitStructure);
    
    /* 中断配置 */
    USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
    
    /* NVIC配置 */
    NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 6;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_Init(&NVIC_InitStructure);
    
    /* USART使能 */
    USART_Cmd(USART2, ENABLE);
    
    /* 创建接收队列 */
    xUart2RxQueue = xQueueCreate(10, sizeof(uart2_packet_t));
    if (xUart2RxQueue == NULL) {
        /* 队列创建失败处理 */
        while(1);
    }
    
    /* 创建串口互斥信号量 */
    xUart2Mutex = xSemaphoreCreateMutex();
    if (xUart2Mutex == NULL) {
        /* 互斥量创建失败处理 */
        while(1);
    }
}

/**
  * 函数名：串口发送一个字节
  */
void Serial2_SendByte(uint8_t Byte)
{
    /* 获取串口互斥量 */
    if (xSemaphoreTake(xUart2Mutex, portMAX_DELAY) == pdTRUE) {
        USART_SendData(USART2, Byte);
        while (USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET)
        {
            taskYIELD();
        }
        /* 释放互斥量 */
        xSemaphoreGive(xUart2Mutex);
    }
}

/**
  * 函数名：串口发送一个数组
  */
void Serial2_SendArray(uint8_t *Array, uint16_t Length)
{
    /* 获取串口互斥量 */
    if (xSemaphoreTake(xUart2Mutex, portMAX_DELAY) == pdTRUE) {
        uint16_t i;
        for (i = 0; i < Length; i++)
        {
            USART_SendData(USART2, Array[i]);
            while (USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET)
            {
                taskYIELD();
            }
        }
        /* 释放互斥量 */
        xSemaphoreGive(xUart2Mutex);
    }
}

/**
  * 函数名：串口发送一个字符串
  */
void Serial2_SendString(char *String)
{
    /* 获取串口互斥量 */
    if (xSemaphoreTake(xUart2Mutex, portMAX_DELAY) == pdTRUE) {
        uint8_t i;
        for (i = 0; String[i] != '\0'; i++)
        {
            USART_SendData(USART2, String[i]);
            while (USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET)
            {
                taskYIELD();
            }
        }
        /* 释放互斥量 */
        xSemaphoreGive(xUart2Mutex);
    }
}

/**
  * 函数名：次方函数（内部使用）
  */
uint32_t Serial2_Pow(uint32_t X, uint32_t Y)
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
void Serial2_SendNumber(uint32_t Number, uint8_t Length)
{
    /* 获取串口互斥量 */
    if (xSemaphoreTake(xUart2Mutex, portMAX_DELAY) == pdTRUE) {
        uint8_t i;
        for (i = 0; i < Length; i++)
        {
            USART_SendData(USART2, Number / Serial2_Pow(10, Length - i - 1) % 10 + '0');
            while (USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET)
            {
                taskYIELD();
            }
        }
        /* 释放互斥量 */
        xSemaphoreGive(xUart2Mutex);
    }
}
/**
  * 函数名：串口直接发送原始字节数据
  * 参数：Array - 要发送的数据数组地址
  *      Length - 要发送的数据长度（字节数）
  * 功能：直接发送原始字节数据，不进行任何转换
  */
void Serial2_SendHex(uint8_t *Array, uint16_t Length)
{
    /* 获取串口互斥量 */
    if (xSemaphoreTake(xUart2Mutex, portMAX_DELAY) == pdTRUE) {
        uint16_t i;
        
        for (i = 0; i < Length; i++)
        {
            USART_SendData(USART2, Array[i]);
            while (USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET)
            {
                taskYIELD();
            }
        }
        
        /* 释放互斥量 */
        xSemaphoreGive(xUart2Mutex);
    }
}
/**
  * 函数名：串口发送浮点数
  */
void Serial2_SendFloat(float value, uint8_t decimal_places)
{
    /* 获取串口互斥量 */
    if (xSemaphoreTake(xUart2Mutex, portMAX_DELAY) == pdTRUE) {
        uint32_t integer_part;
        float fractional_part;
        uint32_t temp;
        uint8_t digits;
        uint8_t i;
        uint8_t digit;
        
        // 处理负数
        if (value < 0) {
            USART_SendData(USART2, '-');
            while (USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET) {
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
            uint32_t divisor = Serial2_Pow(10, i - 1);
            digit = (uint8_t)(temp / divisor);
            USART_SendData(USART2, digit + '0');
            while (USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET) {
                taskYIELD();
            }
            temp %= divisor;
        }
        
        // 发送小数点
        if (decimal_places > 0) {
            USART_SendData(USART2, '.');
            while (USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET) {
                taskYIELD();
            }
            
            // 发送小数部分
            for (i = 0; i < decimal_places; i++) {
                fractional_part *= 10;
                digit = (uint8_t)fractional_part;
                USART_SendData(USART2, digit + '0');
                while (USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET) {
                    taskYIELD();
                }
                fractional_part -= digit;
            }
        }
        
        /* 释放互斥量 */
        xSemaphoreGive(xUart2Mutex);
    }
}


/**
  * 函数名：Position_Move
  * 参数：speed - 速度值 (RPM)
  *       acceleration - 加速度档位 (0-255)
  *       pulses - 脉冲数
  * 功能：位置模式控制，移动到指定位置（绝对模式）
  */
void Position_Move( uint16_t speed, uint8_t acceleration,int32_t pulses)
{
    int i;
    uint8_t cmd_buffer[13]; // 命令缓冲区
    uint32_t abs_pulses;    // 脉冲数的绝对值
    
    // 填充命令数据
    cmd_buffer[0] = 0x01;  // 设备地址 (假设为01)
    cmd_buffer[1] = 0xFD;  // 命令标识
    
    // 根据脉冲正负判断方向
    if(pulses >= 0) {
        cmd_buffer[2] = 0x00;  // 正脉冲：CW方向
        abs_pulses = (uint32_t)pulses;
    } else {
        cmd_buffer[2] = 0x01;  // 负脉冲：CCW方向
        abs_pulses = (uint32_t)(-pulses); // 取绝对值
    }
    
    cmd_buffer[3] = (speed >> 8) & 0xFF;
    cmd_buffer[4] = speed & 0xFF;
    
    // 加速度档位
    cmd_buffer[5] = acceleration;
    
    // 脉冲数 (32位，大端序)
    cmd_buffer[6] = (abs_pulses >> 24) & 0xFF;  // 脉冲数字节3
    cmd_buffer[7] = (abs_pulses >> 16) & 0xFF;  // 脉冲数字节2  
    cmd_buffer[8] = (abs_pulses >> 8) & 0xFF;   // 脉冲数字节1
    cmd_buffer[9] = abs_pulses & 0xFF;          // 脉冲数字节0
    
    cmd_buffer[10] = 0x01;  // 位置模式：0x01=绝对模式
    cmd_buffer[11] = 0x00; // 多机同步：0x00=不启用
    
    cmd_buffer[12] = 0x6B; // 校验位或其他固定值
    
    // 发送命令
    Serial2_SendHex(cmd_buffer, 13); // 注意：根据实际缓冲区大小调整
}

/**
  * 函数名：使用printf需要重定向的底层函数
  */
int fputc2(int ch, FILE *f)
{
    Serial2_SendByte(ch);
    return ch;
}

/**
  * 函数名：自己封装的printf函数
  */
void Serial2_Printf(char *format, ...)
{
    char String[100];
    va_list arg;
    va_start(arg, format);
    vsprintf(String, format, arg);
    va_end(arg);
    Serial2_SendString(String);
}

// ==================== 接收相关函数 ====================

/**
  * 函数名：接收数据包（任务调用）
  */
BaseType_t Serial2_ReceivePacket(uart2_packet_t *packet, TickType_t xTicksToWait)
{
    if (packet == NULL || xUart2RxQueue == NULL)
    {
        return pdFALSE;
    }
    
    return xQueueReceive(xUart2RxQueue, packet, xTicksToWait);
}

/**
  * 函数名：USART2中断函数
  */
void USART2_IRQHandler(void)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    
    // 检查接收中断标志
    if (USART_GetITStatus(USART2, USART_IT_RXNE) == SET)
    {
        uint8_t RxData = USART_ReceiveData(USART2);
        
        // 状态机处理
        switch (uart2_sm.state)
        {
            case 0: // 等待包头
                if (RxData == '@')
                {
                    uart2_sm.state = 1;
                    uart2_sm.data_index = 0;
                    uart2_sm.buffer[0] = '\0';
                }
                break;
                
            case 1: // 接收数据
                if (RxData == '\r')
                {
                    uart2_sm.state = 2;
                }
                else if (uart2_sm.data_index < (sizeof(uart2_sm.buffer) - 1))
                {
                    uart2_sm.buffer[uart2_sm.data_index++] = RxData;
                }
                else
                {
                    // 缓冲区溢出
                    uart2_sm.error_count++;
                    uart2_sm.state = 0;
                }
                break;
                
            case 2: // 等待包尾
                if (RxData == '\n')
                {
                    // 数据包接收完成
                    if (uart2_sm.data_index > 0)
                    {
                        uart2_packet_t packet;
                        packet.length = uart2_sm.data_index;
                        packet.timestamp = xTaskGetTickCountFromISR();
                        memcpy(packet.data, uart2_sm.buffer, uart2_sm.data_index);
                        packet.data[packet.length] = '\0';
                        
                        // 发送到队列
                        if (xQueueSendFromISR(xUart2RxQueue, &packet, &xHigherPriorityTaskWoken) != pdTRUE)
                        {
                            uart2_sm.error_count++;
                        }
                    }
                    else
                    {
                        uart2_sm.error_count++;
                    }
                    uart2_sm.state = 0;
                }
                else
                {
                    // 协议错误
                    uart2_sm.error_count++;
                    uart2_sm.state = 0;
                }
                break;
                
            default:
                uart2_sm.state = 0;
                break;
        }
    }
    
    // 处理过载错误
    if (USART_GetITStatus(USART2, USART_IT_ORE) == SET)
    {
        (void)USART2->SR;
        (void)USART2->DR;
        USART_ClearITPendingBit(USART2, USART_IT_ORE);
        uart2_sm.error_count++;
    }
    
    // 如果有更高优先级任务被唤醒，进行上下文切换
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}