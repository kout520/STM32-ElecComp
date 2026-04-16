#ifndef __USART1_H
#define __USART1_H

#include "stm32f4xx.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "queue.h"

// 数据包结构定义
typedef struct {
    char data[100];         // 数据内容
    uint16_t length;        // 数据长度
    TickType_t timestamp;   // 时间戳
} uart_packet_t;

// 原有的串口函数声明（函数名完全不变）
void Serial_Init(void);
void Serial_SendByte(uint8_t Byte);
void Serial_SendArray(uint8_t *Array, uint16_t Length);
void Serial_SendString(char *String);
void Serial_SendNumber(uint32_t Number, uint8_t Length);
void Serial_Printf(char *format, ...);
void Serial_SendFloat(float value, uint8_t decimal_places);
int ExtractNumbersFromString(const char* inputString, int* outputArray, int maxArraySize);
BaseType_t Serial_ReceivePacket(uart_packet_t *packet, TickType_t xTicksToWait);

#endif /* __USART1_H */