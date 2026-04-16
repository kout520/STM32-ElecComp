#ifndef __OLED_H
#define __OLED_H

#include "stm32f4xx.h"

/* 函数声明 */

// 初始化函数
void OLED_I2C_Init(void);
void OLED_Init(void);

// 基础I2C函数
void OLED_I2C_Start(void);
void OLED_I2C_Stop(void);
void OLED_I2C_SendByte(uint8_t Byte);

// OLED读写函数
void OLED_WriteCommand(uint8_t Command);
void OLED_WriteData(uint8_t Data);

// 光标和清屏函数
void OLED_SetCursor(uint8_t Y, uint8_t X);
void OLED_Clear(void);

// 显示函数
void OLED_ShowChar(uint8_t Line, uint8_t Column, char Char);
void OLED_ShowString(uint8_t Line, uint8_t Column, char *String);
void OLED_ShowNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length);
void OLED_ShowSignedNum(uint8_t Line, uint8_t Column, int32_t Number, uint8_t Length);
void OLED_ShowHexNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length);
void OLED_ShowBinNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length);

// 工具函数
uint32_t OLED_Pow(uint32_t X, uint32_t Y);

#endif