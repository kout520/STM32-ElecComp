#include "headfile.h"
#include "font.h"



#define LCD_CS_H()	HAL_GPIO_WritePin(LCD_CS_GPIO_Port,LCD_CS_Pin,GPIO_PIN_SET)
#define LCD_CS_L()	HAL_GPIO_WritePin(LCD_CS_GPIO_Port,LCD_CS_Pin,GPIO_PIN_RESET)
#define LCD_RS_H()	HAL_GPIO_WritePin(LCD_RS_GPIO_Port,LCD_RS_Pin,GPIO_PIN_SET)
#define LCD_RS_L()	HAL_GPIO_WritePin(LCD_RS_GPIO_Port,LCD_RS_Pin,GPIO_PIN_RESET)
#define LCD_RES_H()	HAL_GPIO_WritePin(LCD_RES_GPIO_Port,LCD_RES_Pin,GPIO_PIN_SET)
#define LCD_RES_L()	HAL_GPIO_WritePin(LCD_RES_GPIO_Port,LCD_RES_Pin,GPIO_PIN_RESET)

#define LCD_CLK_L()	HAL_GPIO_WritePin(LCD_CLK_GPIO_Port,LCD_CLK_Pin,GPIO_PIN_RESET)
#define LCD_CLK_H()	HAL_GPIO_WritePin(LCD_CLK_GPIO_Port,LCD_CLK_Pin,GPIO_PIN_SET)
#define	LCD_SDA_L()	HAL_GPIO_WritePin(LCD_SDA_GPIO_Port,LCD_SDA_Pin,GPIO_PIN_RESET)
#define	LCD_SDA_H()	HAL_GPIO_WritePin(LCD_SDA_GPIO_Port,LCD_SDA_Pin,GPIO_PIN_SET)

#define LCD_Delayms(x)	HAL_Delay(x)

#define REG_DATA	1
#define REG_CMD		0

/****
*	功能：写数据到液晶屏
*	输入：_ucCmd=数据寄存器或者命令寄存器,_ucData=数据
*	输出：无
****/
static void LCD_Write(uint8_t _ucReg,uint8_t _ucData)
{
	uint8_t i;
	if(_ucReg)	LCD_RS_H();
	else		LCD_RS_L();
	for(i=0;i<8;i++)
	{
		LCD_CLK_L();
		if(_ucData & 0x80)	LCD_SDA_H();
		else				LCD_SDA_L();
		LCD_CLK_H();
		_ucData <<= 1;
	}
	
}

/****
*	功能：初始化液晶屏
*	输入：无
*	输出：无
****/
void LCD_Init(void)
{
	LCD_CS_L();
	LCD_RES_L();				/*低电平复位*/
	LCD_Delayms(25);
	LCD_RES_H();				/*复位完成*/
	LCD_Delayms(20);
	LCD_Write(REG_CMD,0XE2);	/*软复位*/
	LCD_Delayms(7);
	LCD_Write(REG_CMD,0x2c);	/*升压1*/
	LCD_Delayms(7);
	LCD_Write(REG_CMD,0x2e);	/*升压2*/
	LCD_Delayms(7);
	LCD_Write(REG_CMD,0x2f);	/*升压3*/
	LCD_Delayms(7);	
	LCD_Write(REG_CMD,0x23);	/*粗调对比度，0x20-0x27*/
	LCD_Write(REG_CMD,0x81);	/*微调对比度*/
	LCD_Write(REG_CMD,0x28);	/*微调对比度值，0x00-0x3f*/
	LCD_Write(REG_CMD,0xa2);	/*1/9偏压比*/
	LCD_Write(REG_CMD,0xc8);	/*行扫描顺序：从上到下*/
	LCD_Write(REG_CMD,0xa0);	/*列扫描顺序：从左到右*/
	LCD_Write(REG_CMD,0x40);	/*起始行：第一行开始*/	
//	LCD_Write(REG_CMD,0xA7);	/*反显*/	
	LCD_Write(REG_CMD,0xaf);	/*开显示*/
	LCD_CS_H();
}
//
/****
*	功能：设置屏幕的位置
*	输入：无
*	输出：无
****/
static void LCD_SetPos(uint8_t _ucPage,uint8_t _ucColumn)
{
	LCD_CS_L();
	_ucPage -= 0x01;
	LCD_Write(REG_CMD,0xb0+_ucPage - 1);
	LCD_Write(REG_CMD,((_ucColumn>>4) & 0x0f) + 0x10);
	LCD_Write(REG_CMD,(_ucColumn & 0x0f));
}
/****
*	功能：清屏
*	输入：无
*	输出：无
****/
void LCD_Clear(void)
{
	uint8_t i,j;
	LCD_CS_L();
	for(i=0; i<9; i++)
	{
        LCD_Write(REG_CMD,0xb0 + i);
        LCD_Write(REG_CMD,0x10);
        LCD_Write(REG_CMD,0x00);
		//LCD_SetPos(1+i,1);
		for(j=0; j<132; j++)
		{
			LCD_Write(REG_DATA,0x00);
            
		}
	}
	LCD_CS_H();
}
/****
*	功能：显示单个8*16字符
*	输入：无
*	输出：无
****/
static void LCD_DispChar8x16(uint8_t _ucPage,uint8_t _ucColumn,uint8_t *p)
{
	uint8_t i,j;
	LCD_CS_L();
	for(j=0; j<2; j++)
	{
		LCD_SetPos(_ucPage+j,_ucColumn);
		for(i=0; i<8; i++)
		{
			LCD_Write(REG_DATA,*p);
			p++;
		}
		
	}
	LCD_CS_H();
}
//
//m^n函数
//返回值:m^n次方.
uint32_t LCD_Pow(uint8_t m,uint8_t n)
{
	uint32_t result=1;	 
	while(n--)result*=m;    
	return result;
}		
/****
*	功能：显示8*16字符串
*	输入：无
*	输出：无
****/
void LCD_DispStr8x16(uint8_t _ucPage,uint8_t _ucColumn,char *str)
{
	uint8_t num,i;
	uint8_t  buff[16];
	while(*str != '\0')
	{
		num = *str - ' ';/*定位字符在二维数组中的位置*/
		for(i=0; i<16; i++)/*获取字符的取模数据*/
		{
			buff[i] = ASCII_8X16[num][i];
		}
		LCD_DispChar8x16(_ucPage,_ucColumn,buff);
		_ucColumn += 8;
		str++;
	}
}

/****
*	功能：显示8*16字符数字
*	输入：
*	输出：无
****/
void LCD_DispNum(uint8_t _ucPage,uint8_t _ucColumn,uint16_t num,uint8_t _ucLenth)
{    	
	uint8_t i,t,temp;
	uint8_t enshow=0;
	uint8_t buff[16];
	for(t=0;t<_ucLenth;t++)
	{
		temp=(num/LCD_Pow(10,_ucLenth-t-1))%10;
		if(enshow==0&&t<(_ucLenth-1))
		{
			if(temp==0)
			{
				LCD_DispStr8x16(_ucPage,_ucColumn+8*t," ");
				continue;
			}else enshow=1; 
		}
		temp += 16 ;
		for(i=0; i<16; i++)/*获取字符的取模数据*/
		{
			buff[i] = ASCII_8X16[temp][i];
		}
	 	LCD_DispChar8x16(_ucPage,_ucColumn+8*t,buff); 
	}
}
//


