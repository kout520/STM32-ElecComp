#include "stm32f4xx.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "usart1.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

// 队列定义 - 用于存储接收到的数据包
QueueHandle_t xUartRxQueue = NULL;

// 状态机上下文结构
typedef struct {
    uint8_t state;          // 当前状态
    uint16_t data_index;    // 数据索引
    char buffer[100];       // 接收缓冲区
    uint32_t error_count;   // 错误计数
} uart_state_machine_t;

static uart_state_machine_t uart_sm = {0};

// 串口互斥信号量定义
SemaphoreHandle_t xUartMutex = NULL;

void Serial_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    /* 开启时钟 */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
    
    /* GPIO初始化 */
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    /* 配置引脚复用 */
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_USART1);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_USART1);
    
    /* USART初始化 */
    USART_InitStructure.USART_BaudRate = 115200;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_Init(USART1, &USART_InitStructure);
    
    /* 中断配置 - 使用正确的优先级 */
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
    
    /* NVIC配置 - 确保优先级在 FreeRTOS 管理范围内 */
    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 6;  // 必须 >= configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;         // 在分组4中，子优先级必须为0
    NVIC_Init(&NVIC_InitStructure);
    
    /* USART使能 */
    USART_Cmd(USART1, ENABLE);
    
    /* 创建接收队列 */
    xUartRxQueue = xQueueCreate(10, sizeof(uart_packet_t));
    if (xUartRxQueue == NULL) {
        /* 队列创建失败处理 */
        while(1);
    }
    
    /* 创建串口互斥信号量 */
    xUartMutex = xSemaphoreCreateMutex();
    if (xUartMutex == NULL) {
        /* 互斥量创建失败处理 */
        while(1);
    }
}

/**
  * 函数名：串口发送一个字节
  */
void Serial_SendByte(uint8_t Byte)
{
    /* 获取串口互斥量 */
    if (xSemaphoreTake(xUartMutex, portMAX_DELAY) == pdTRUE) {
        USART_SendData(USART1, Byte);
        while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET)
        {
            taskYIELD();  // 在FreeRTOS中，长时间等待时让出CPU
        }
        /* 释放互斥量 */
        xSemaphoreGive(xUartMutex);
    }
}

/**
  * 函数名：串口发送一个数组
  */
void Serial_SendArray(uint8_t *Array, uint16_t Length)
{
    /* 获取串口互斥量 */
    if (xSemaphoreTake(xUartMutex, portMAX_DELAY) == pdTRUE) {
        uint16_t i;
        for (i = 0; i < Length; i++)
        {
            USART_SendData(USART1, Array[i]);
            while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET)
            {
                taskYIELD();
            }
        }
        /* 释放互斥量 */
        xSemaphoreGive(xUartMutex);
    }
}

/**
  * 函数名：串口发送一个字符串
  */
void Serial_SendString(char *String)
{
    /* 获取串口互斥量 */
    if (xSemaphoreTake(xUartMutex, portMAX_DELAY) == pdTRUE) {
        uint8_t i;
        for (i = 0; String[i] != '\0'; i++)
        {
            USART_SendData(USART1, String[i]);
            while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET)
            {
                taskYIELD();
            }
        }
        /* 释放互斥量 */
        xSemaphoreGive(xUartMutex);
    }
}

/**
  * 函数名：次方函数（内部使用）
  */
uint32_t Serial_Pow(uint32_t X, uint32_t Y)
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
void Serial_SendNumber(uint32_t Number, uint8_t Length)
{
    /* 获取串口互斥量 */
    if (xSemaphoreTake(xUartMutex, portMAX_DELAY) == pdTRUE) {
        uint8_t i;
        for (i = 0; i < Length; i++)
        {
            USART_SendData(USART1, Number / Serial_Pow(10, Length - i - 1) % 10 + '0');
            while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET)
            {
                taskYIELD();
            }
        }
        /* 释放互斥量 */
        xSemaphoreGive(xUartMutex);
    }
}
/**
  * 函数名：串口发送浮点数
  * 参数：value - 要发送的浮点数
  *       decimal_places - 小数点后位数
  */
void Serial_SendFloat(float value, uint8_t decimal_places)
{
    /* 获取串口互斥量 */
    if (xSemaphoreTake(xUartMutex, portMAX_DELAY) == pdTRUE) {
        /* 所有变量声明放在函数开头 */
        uint32_t integer_part;
        float fractional_part;
        uint32_t temp;
        uint8_t digits;
        uint8_t i;
        uint8_t digit;
        
        // 处理负数
        if (value < 0) {
            USART_SendData(USART1, '-');
            while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET) {
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
            uint32_t divisor = Serial_Pow(10, i - 1);
            digit = (uint8_t)(temp / divisor);
            USART_SendData(USART1, digit + '0');
            while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET) {
                taskYIELD();
            }
            temp %= divisor;
        }
        
        // 发送小数点
        if (decimal_places > 0) {
            USART_SendData(USART1, '.');
            while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET) {
                taskYIELD();
            }
            
            // 发送小数部分
            for (i = 0; i < decimal_places; i++) {
                fractional_part *= 10;
                digit = (uint8_t)fractional_part;
                USART_SendData(USART1, digit + '0');
                while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET) {
                    taskYIELD();
                }
                fractional_part -= digit;
            }
        }
        
        /* 释放互斥量 */
        xSemaphoreGive(xUartMutex);
    }
}

/**
  * 函数名：使用printf需要重定向的底层函数
  */
int fputc(int ch, FILE *f)
{
    Serial_SendByte(ch);  // 这里会调用我们上面保护过的函数
    return ch;
}

/**
  * 函数名：自己封装的printf函数
  */
void Serial_Printf(char *format, ...)
{
    char String[100];
    va_list arg;
    va_start(arg, format);
    vsprintf(String, format, arg);
    va_end(arg);
    Serial_SendString(String);  // 这里会调用我们上面保护过的函数
}


// ==================== 接收相关函数保持不变 ====================

/**
  * 函数名：接收数据包（任务调用）
  * 参数：packet - 接收到的数据包
  *       xTicksToWait - 等待超时时间
  * 返回值：pdTRUE-成功接收到数据包，pdFALSE-超时或错误
  */
BaseType_t Serial_ReceivePacket(uart_packet_t *packet, TickType_t xTicksToWait)
{
    if (packet == NULL || xUartRxQueue == NULL)
    {
        return pdFALSE;
    }
    
    return xQueueReceive(xUartRxQueue, packet, xTicksToWait);
}

/**
  * 函数名：USART1中断函数 - 改造为FreeRTOS兼容版本
  */
void USART1_IRQHandler(void)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    
    // 检查接收中断标志
    if (USART_GetITStatus(USART1, USART_IT_RXNE) == SET)
    {
        uint8_t RxData = USART_ReceiveData(USART1);
        
        // 状态机处理
        switch (uart_sm.state)
        {
            case 0: // 等待包头
                if (RxData == '@')
                {
                    uart_sm.state = 1;
                    uart_sm.data_index = 0;
                    uart_sm.buffer[0] = '\0'; // 清空缓冲区
                }
                break;
                
            case 1: // 接收数据
                if (RxData == '\r')
                {
                    uart_sm.state = 2;
                }
                else if (uart_sm.data_index < (sizeof(uart_sm.buffer) - 1))
                {
                    uart_sm.buffer[uart_sm.data_index++] = RxData;
                }
                else
                {
                    // 缓冲区溢出
                    uart_sm.error_count++;
                    uart_sm.state = 0;
                }
                break;
                
            case 2: // 等待包尾
                if (RxData == '\n')
                {
                    // 数据包接收完成 - 验证数据长度
                    if (uart_sm.data_index > 0)
                    {
                        uart_packet_t packet;
                        packet.length = uart_sm.data_index;
                        packet.timestamp = xTaskGetTickCountFromISR();
                        memcpy(packet.data, uart_sm.buffer, uart_sm.data_index);
                        packet.data[packet.length] = '\0';  // 添加字符串结束符
                        
                        // 发送到队列
                        if (xQueueSendFromISR(xUartRxQueue, &packet, &xHigherPriorityTaskWoken) != pdTRUE)
                        {
                            // 队列满，统计错误
                            uart_sm.error_count++;
                        }
                    }
                    else
                    {
                        uart_sm.error_count++; // 空数据包
                    }
                    uart_sm.state = 0;
                }
                else
                {
                    // 协议错误
                    uart_sm.error_count++;
                    uart_sm.state = 0;
                }
                break;
                
            default:
                uart_sm.state = 0;
                break;
        }
        
        // 注意：读取USART1->DR已经清除了RXNE标志，这行可以去掉
        // USART_ClearITPendingBit(USART1, USART_IT_RXNE);
    }
    
    // 处理过载错误
    if (USART_GetITStatus(USART1, USART_IT_ORE) == SET)
    {
        (void)USART1->SR;
        (void)USART1->DR;
        USART_ClearITPendingBit(USART1, USART_IT_ORE);
        uart_sm.error_count++;
    }
    
    // 如果有更高优先级任务被唤醒，进行上下文切换
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}


/**
 * @brief 从字符串中提取所有整数数字
 * @param inputString 输入字符串
 * @param outputArray 输出整数数组
 * @param maxArraySize 输出数组的最大容量
 * @return 提取到的数字个数，如果出错返回-1
 */
int ExtractNumbersFromString(const char* inputString, int* outputArray, int maxArraySize)
	{   
		int stringLength = strlen(inputString);
    int numberCount = 0;
    int currentNumber = 0;
    int isNegative = 0;
    int isParsingNumber = 0;
    int i;
    // 参数检查
    if (inputString == NULL || outputArray == NULL || maxArraySize <= 0) {
        return -1;
    }
    
 
    for ( i= 0; i <= stringLength && numberCount < maxArraySize; i++) {
        if (inputString[i] >= '0' && inputString[i] <= '9') {
            currentNumber = currentNumber * 10 + (inputString[i] - '0');
            isParsingNumber = 1;
        }
        else if (inputString[i] == '-' && !isParsingNumber) {
            isNegative = 1;
        }
        else {
            if (isParsingNumber) {
                outputArray[numberCount++] = isNegative ? -currentNumber : currentNumber;
                currentNumber = 0;
                isNegative = 0;
                isParsingNumber = 0;
            }
            else if (inputString[i] != '-') {
                isNegative = 0;
            }
        }
    }
    
    return numberCount;
}
