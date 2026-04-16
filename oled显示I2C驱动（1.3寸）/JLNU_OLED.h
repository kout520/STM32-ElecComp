#ifndef __JLNU_OLED_H
#define	__JLNU_OLED_H

#include "headfile.h"

#define OLED_ADDRESS	0x78

void I2C_WriteByte(uint8_t addr,uint8_t data);		//向寄存器发送数据
void OLED_WriteCmd(uint8_t command);					//写命令
void OLED_WriteData(uint8_t data);						//写数据
void OLED_Init(void);									//上电初始化
void OLED_SetPos(uint8_t x, uint8_t y);				//设置起始坐标
void OLED_Clear(void);								//清屏
void OLED_ON(void); 									//唤醒屏幕
void OLED_OFF(void);									//关闭屏幕
void OLED_ShowStr(uint8_t x, uint8_t y, uint8_t ch[], uint8_t TextSize);     //显示字符串
void OLED_ShowCN(uint8_t x,uint8_t y,uint8_t n);					//显示中文
void OLED_DrawBMP(uint8_t x0,uint8_t y0,uint8_t x1,uint8_t y1);  //显示图像


/******************* 6*8点阵 **********************/

#endif
