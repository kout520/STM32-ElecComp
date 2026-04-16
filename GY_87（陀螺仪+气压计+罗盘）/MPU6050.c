#include "MPU6050.h"
#include "IIC.h"
#include "lcd.h"
#include <stdint.h>

void	MPU6050_Initial(void)
{
	Single_Write(MPU6050_Addr,PWR_MGMT_1, 0x00);	//解除休眠状态
	Single_Write(MPU6050_Addr,SMPLRT_DIV, 0x07);	//陀螺仪采样率
	Single_Write(MPU6050_Addr,CONFIG, 0x06);			//5Hz 

	Single_Write(MPU6050_Addr,INT_PIN_CFG, 0x42);	//使能旁路I2C
	Single_Write(MPU6050_Addr,USER_CTRL, 0x40);		//使能旁路I2C

	Single_Write(MPU6050_Addr,GYRO_CONFIG, 0x18);   
	Single_Write(MPU6050_Addr,ACCEL_CONFIG, 0x01);
}

uint8_t	MPU6050_Read_GYRO(void)
{
	unsigned char BUF[10];
	short T_X,T_Y,T_Z,T_T;

	BUF[0]=Single_Read(MPU6050_Addr,GYRO_XOUT_L); 
	BUF[1]=Single_Read(MPU6050_Addr,GYRO_XOUT_H);
	T_X=	(BUF[1]<<8)|BUF[0];
	T_X/=16.4;

	BUF[2]=Single_Read(MPU6050_Addr,GYRO_YOUT_L);
	BUF[3]=Single_Read(MPU6050_Addr,GYRO_YOUT_H);
	T_Y=	(BUF[3]<<8)|BUF[2];
	T_Y/=16.4;
	
	BUF[4]=Single_Read(MPU6050_Addr,GYRO_ZOUT_L);
	BUF[5]=Single_Read(MPU6050_Addr,GYRO_ZOUT_H);
	T_Z=	(BUF[5]<<8)|BUF[4];
	T_Z/=16.4;

	BUF[6]=Single_Read(MPU6050_Addr,TEMP_OUT_L); 
	BUF[7]=Single_Read(MPU6050_Addr,TEMP_OUT_H); 
	T_T=(BUF[7]<<8)|BUF[6];
	T_T = 35+ ((double) (T_T + 13200)) / 280;
	
	LCD_MPU6050(LINE1,T_X,T_Y,T_Z);
		
	return T_Z;
}

uint8_t	MPU6050_Read_ACCEL(void)
{
	unsigned char BUF[10];
	short T_X,T_Y,T_Z,T_T;
	
	BUF[0]=Single_Read(MPU6050_Addr,ACCEL_XOUT_L); 
	BUF[1]=Single_Read(MPU6050_Addr,ACCEL_XOUT_H);
	T_X=	(BUF[1]<<8)|BUF[0];
	T_X=(float)((float)T_X/16384)*100;

	BUF[2]=Single_Read(MPU6050_Addr,ACCEL_YOUT_L);
	BUF[3]=Single_Read(MPU6050_Addr,ACCEL_YOUT_H);
	T_Y=	(BUF[3]<<8)|BUF[2];
	T_Y=(float)((float)T_Y/16384)*100;
   
	BUF[4]=Single_Read(MPU6050_Addr,ACCEL_ZOUT_L);
	BUF[5]=Single_Read(MPU6050_Addr,ACCEL_ZOUT_H);
	T_Z=	(BUF[5]<<8)|BUF[4];
	T_Z=(float)((float)T_Z/16384)*100;

	
	BUF[6]=Single_Read(MPU6050_Addr,TEMP_OUT_L); 
	BUF[7]=Single_Read(MPU6050_Addr,TEMP_OUT_H); 
	T_T=(BUF[7]<<8)|BUF[6];
	T_T = 35+ ((double) (T_T + 13200)) / 280;
	T_T = (((double) T_T )/340+36.53)*10 ;
	
	LCD_MPU6050(LINE2,T_X,T_Y,T_Z);
	
	return T_T;
}
