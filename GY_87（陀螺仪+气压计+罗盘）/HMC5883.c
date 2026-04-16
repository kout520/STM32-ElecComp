#include "HMC5883.h"
#include "IIC.h"
#include "lcd.h"
#include <math.h>

void	HMC5883L_Initial(void)
{
	Single_Write(HMC5883L_Addr,0x00,0x14);	//配置寄存器A：采样平均数1 输出速率75Hz 正常测量
	Single_Write(HMC5883L_Addr,0x02,0x00);	//模式寄存器：连续测量模式	
	Single_Write(HMC5883L_Addr,0x01,0x20);	//配置寄存器B：增益控制
}

float	HMC5883L_Read(void)
{
	unsigned char BUF[10];
	int   x,y;
	float angle;
	
	Single_Write(HMC5883L_Addr,0x00,0x14);   //配置寄存器A：采样平均数1 输出速率75Hz 正常测量

	Delayms(5);

	BUF[1]=Single_Read(HMC5883L_Addr,0x03);//OUT_X_L_A
	BUF[2]=Single_Read(HMC5883L_Addr,0x04);//OUT_X_H_A

	BUF[3]=Single_Read(HMC5883L_Addr,0x07);//OUT_Y_L_A
	BUF[4]=Single_Read(HMC5883L_Addr,0x08);//OUT_Y_H_A

	x=(BUF[1] << 8) | BUF[2]; //Combine MSB and LSB of X Data output register
	y=(BUF[3] << 8) | BUF[4]; //Combine MSB and LSB of Z Data output register

	if(x>0x7fff)x-=0xffff;	  
	if(y>0x7fff)y-=0xffff;	
  angle = atan2(y,x) * (180 / 3.14159265) + 180; // angle in degrees
	
	LCD_Float(COLUMN1,LINE4,angle);
	return angle;
}
