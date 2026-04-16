#ifndef __USART3_H
#define __USART3_H

#include "stm32f4xx.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#ifdef __cplusplus
extern "C" {
#endif

// 数据包结构定义
typedef struct {
    uint16_t length;        // 数据长度
    TickType_t timestamp;   // 时间戳
    char data[100];         // 数据内容
} uart3_packet_t;

// 全局变量声明
extern QueueHandle_t xUart3RxQueue;
extern SemaphoreHandle_t xUart3Mutex;

// 函数声明
void Serial3_Init(void);
void Serial3_SendByte(uint8_t Byte);
void Serial3_SendArray(uint8_t *Array, uint16_t Length);
void Serial3_SendString(char *String);
void Serial3_SendNumber(uint32_t Number, uint8_t Length);
void Serial3_SendFloat(float value, uint8_t decimal_places);
void Serial3_Printf(char *format, ...);
BaseType_t Serial3_ReceivePacket(uart3_packet_t *packet, TickType_t xTicksToWait);

// 重定向函数声明
int fputc3(int ch, FILE *f);

// 中断处理函数声明
void USART3_IRQHandler(void);

#ifdef __cplusplus
}
#endif

#endif /* __USART3_H */