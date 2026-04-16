#ifndef __JL12864G_109_H
#define __JL12864G_109_H



void LCD_Init(void);
void LCD_Clear(void);

void LCD_DispStr8x16(uint8_t _ucPage,uint8_t _ucColumn,char *str);
void LCD_DispNum(uint8_t _ucPage,uint8_t _ucColumn,uint16_t num,uint8_t _ucLenth);

#endif

