#ifndef __USART2_H
#define __USART2_H

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
} uart2_packet_t;

// 全局变量声明
extern QueueHandle_t xUart2RxQueue;
extern SemaphoreHandle_t xUart2Mutex;

// 函数声明
void Serial2_Init(void);
void Serial2_SendByte(uint8_t Byte);
void Serial2_SendArray(uint8_t *Array, uint16_t Length);
void Serial2_SendString(char *String);
void Serial2_SendNumber(uint32_t Number, uint8_t Length);
void Serial2_SendFloat(float value, uint8_t decimal_places);
void Serial2_SendHex(uint8_t *Array, uint16_t Length);
void Serial2_Printf(char *format, ...);
void Position_Move(uint16_t speed, uint8_t acceleration, int32_t pulses);
BaseType_t Serial2_ReceivePacket(uart2_packet_t *packet, TickType_t xTicksToWait);

// 重定向函数声明（如果需要）
int fputc2(int ch, FILE *f);

// 中断处理函数声明
void USART2_IRQHandler(void);

#ifdef __cplusplus
}
#endif

#endif /* __USART2_H */