
#include "JLNU_OLED.h"
#include "JLNU_Font.h"


/******************************************注意************************************/
//1、在清屏时可能出现边缘没有清除，可在函数Clear中修改		for(n=0;n<128;n++)改为for(n=0;n<129;n++)甚至更大，直到边缘去除
//2、在使用中文、图片显示时把设置Target中的Use MicroLIB勾选去掉，否则只有调试时可用，掉电重启后会出现问题。（针对Keil5，STM32F103C8T6出现的情况）




//根据情况修改以下函数==========================
extern I2C_HandleTypeDef hi2c2;
#define I2C_WriteByte(addr,data)  	HAL_I2C_Mem_Write(&hi2c2,OLED_ADDRESS,addr,1,&data,1,1000)
#define delay_ms(ms)  HAL_Delay(ms)
//==============================================


void OLED_WriteCmd(uint8_t command)
{

	I2C_WriteByte(0x00,command);

}

void OLED_WriteData(uint8_t data)
{

	I2C_WriteByte(0x40,data);

}

void OLED_Init(void)
{
	delay_ms(100); //上电后需延时
	
	OLED_WriteCmd(0xAE); //关闭显示（0xAE关闭OLED，0xAF打开OLED）
	OLED_WriteCmd(0x20);	//设置GDDRAM寻址模式，由下面命令决定	
	OLED_WriteCmd(0x10);	//00：水平寻址模式; 01：垂直寻址模式; 10：页寻址模式;
	OLED_WriteCmd(0xb0);	//设置页寻址模式的页起始位置
	OLED_WriteCmd(0xc8);	//设置扫描方向
	OLED_WriteCmd(0x02); //设置列地址起始地址低位
	OLED_WriteCmd(0x10); //设置列地址起始地址高位
	OLED_WriteCmd(0x40); //设置GDDRAM起始行
	OLED_WriteCmd(0x81); //设置对比度
	OLED_WriteCmd(0xff); //亮度调节 0x00~0xff
	OLED_WriteCmd(0xa1); //左右反置关
	OLED_WriteCmd(0xa6); //正常显示模式
	OLED_WriteCmd(0xa8); //设置复用率
	OLED_WriteCmd(0x3F); 
	OLED_WriteCmd(0xa4); //输出GDDRAM中的显示数据
	OLED_WriteCmd(0xd3); //设置显示偏移
	OLED_WriteCmd(0x00); //无偏移
	OLED_WriteCmd(0xd5); //设置时钟
	OLED_WriteCmd(0xf0); 
	OLED_WriteCmd(0xd9); //预充电周期
	OLED_WriteCmd(0x22); //
	OLED_WriteCmd(0xda); //设置硬件配置
	OLED_WriteCmd(0x12);
	OLED_WriteCmd(0xdb); //设置Vcomh输出
	OLED_WriteCmd(0x20); //0.77*Vcc
	OLED_WriteCmd(0x8d); //启用电荷泵
	OLED_WriteCmd(0x14); //启用电荷泵
	OLED_WriteCmd(0xaf); //开OLED显示
}

void OLED_SetPos(uint8_t x, uint8_t y)
{
	OLED_WriteCmd(0xb0+y);
	OLED_WriteCmd(((x&0xf0)>>4)|0x10);
	OLED_WriteCmd((x&0x0f)|0x01);
}


void OLED_Clear(void)
{
	uint8_t m,n;
	for(m=0;m<8;m++)
	{
		OLED_WriteCmd(0xb0+m);		
		OLED_WriteCmd(0x00);		
		OLED_WriteCmd(0x10);		
		for(n=0;n<128;n++)
			{
				OLED_WriteData(0x00);
			}
	}
}

void OLED_ON(void)
{
	OLED_WriteCmd(0X8D);  //设置电荷泵
	OLED_WriteCmd(0X14);  //开启电荷泵
	OLED_WriteCmd(0XAF);  //OLED唤醒
}

void OLED_OFF(void)
{
	OLED_WriteCmd(0X8D);  //设置电荷泵
	OLED_WriteCmd(0X10);  //关闭电荷泵
	OLED_WriteCmd(0XAE);  //OLED休眠
}

//===========================================================
//		x,y:起始位置坐标
//		ch：字符，例如‘a’ 'b'
//	TextSize:		6*8字符（TextSize=1）  8*16字符（TextSize=2）
//===========================================================
void OLED_ShowStr(uint8_t x,uint8_t y, uint8_t ch[], uint8_t TextSize)
{
	uint8_t c = 0,i = 0,j=0;
	switch(TextSize)
	{
		case 1:
		{
			while(ch[j] != '\0')
			{
					c = ch[j] - 32;
					if(x > 122)
					{
						x = 0;
						y++;
					}
					OLED_SetPos(x,y);
					for(i=0;i<6;i++)
					{	
					OLED_WriteData(Ch6x8[c][i]);
					}
					x+=6;
					j++;
			}
		}break;
		case 2:
		{
			while(ch[j] != '\0')
			{
				c = ch[j] - 32;
				if(x > 120)
				{
					x = 0;
					y+=2;
				}
				OLED_SetPos(x,y);
				for(i=0;i<8;i++)
				{
					OLED_WriteData(Ch8x16[c*16+i]);
				}
				OLED_SetPos(x,y+1);
				for(i=0;i<8;i++)
				{
					OLED_WriteData(Ch8x16[c*16+i+8]);
				}
				x+=8;
				j++;
			}
		}break;
	}
}

//n代表字库的第几个字，每次只能用一个汉字

void OLED_ShowCN(uint8_t x,uint8_t y,uint8_t n)
{
        uint8_t i=0,y1;
        y1=y+1;

        OLED_SetPos(x,y);
        for(i=0;i<16;i++)
        {
            OLED_WriteData(Ch16x16[2*n][i]);
        }
        OLED_SetPos(x,y1);
        //j++;
        for(i=0;i<16;i++)
        {
            OLED_WriteData(Ch16x16[2*n+1][i]);
        }
        //j++;
        x+=16;

}

//参数范围按以下设置，否则可能会产生错误：
//(x0:0~126, y0:0~6);(x1:1~127,y1:1~7);
void OLED_DrawBMP(uint8_t x0,uint8_t y0,uint8_t x1,uint8_t y1)
{
	uint16_t i;
	uint8_t  x,y;
	for(y=y0;y<=y1;y++)
	{
		OLED_SetPos(x0,y);
    for(x=x0;x<=x1;x++)
		{
			OLED_WriteData(BMP[i++]);
		}
	}
}
