#include "BMP180.h"
#include "IIC.h"
#include "lcd.h"

#define   OSS 0	 //	BMP085สนำร

short ac1,ac2,ac3; 
unsigned short ac4,ac5,ac6;
short b1,b2;
short mb,mc,md;

long  temperature;
long  pressure;

void	BMP180_Initial(void)
{
	ac1 = Single_Read(BMP085_Addr,0xAA);
	ac1 = (ac1<<8)|Single_Read(BMP085_Addr,0xAB);

	ac2 = Single_Read(BMP085_Addr,0xAC);
	ac2 = (ac2<<8)| Single_Read(BMP085_Addr,0xAD);

	ac3 = Single_Read(BMP085_Addr,0xAE);
	ac3 = (ac3<<8)| Single_Read(BMP085_Addr,0xAF);

	ac4 = Single_Read(BMP085_Addr,0xB0);
	ac4 = (ac4<<8)| Single_Read(BMP085_Addr,0xB1);

	ac5 = Single_Read(BMP085_Addr,0xB2);
	ac5 = (ac5<<8)| Single_Read(BMP085_Addr,0xB3);

	ac6 = Single_Read(BMP085_Addr,0xB4);
	ac6 = (ac6<<8)| Single_Read(BMP085_Addr,0xB5);

	b1 = Single_Read(BMP085_Addr,0xB6);
	b1 = (b1<<8)| Single_Read(BMP085_Addr,0xB7);

	b2 = Single_Read(BMP085_Addr,0xB8);
	b2 = (b2<<8)| Single_Read(BMP085_Addr,0xB9);

	mb = Single_Read(BMP085_Addr,0xBA);
	mb = (mb<<8)| Single_Read(BMP085_Addr,0xBB);

	mc = Single_Read(BMP085_Addr,0xBC);
	mc = (mc<<8)| Single_Read(BMP085_Addr,0xBD);

	md = Single_Read(BMP085_Addr,0xBE);
	md = (md<<8)| Single_Read(BMP085_Addr,0xBF);

}

long	BMP180_Read_Temperature(void)
{   
	short  temp_ut;
	Single_Write(BMP085_Addr,0xF4,0x2E);
	
	Delayms(5);
	
	temp_ut = Single_Read(BMP085_Addr,0xF6);
	temp_ut = (temp_ut<<8)| Single_Read(BMP085_Addr,0xF7);
	
	return (long) temp_ut ;
}

long	BMP180_Read_Pressure(void)
{
	long pressure = 0;
	
	Single_Write(BMP085_Addr,0xF4,0x34);
	
	Delayms(5);
	
	pressure = Single_Read(BMP085_Addr,0xF6);
	pressure = (pressure<<8)| Single_Read(BMP085_Addr,0xF7);		
	pressure &= 0x0000FFFF;	
	
	return pressure;	
}

long	BMP180_Read(void)
{
	long ut;
	long up;
	long x1, x2, b5, b6, x3, b3, p;
	unsigned long b4, b7;

	ut = BMP180_Read_Temperature();
	up = BMP180_Read_Pressure();

	x1 = ((long)ut - ac6) * ac5 >> 15;
	x2 = ((long) mc << 11) / (x1 + md);
	b5 = x1 + x2;
	temperature = (b5 + 8) >> 4;

	b6 = b5 - 4000;
	x1 = (b2 * (b6 * b6 >> 12)) >> 11;
	x2 = ac2 * b6 >> 11;
	x3 = x1 + x2;
	
	b3 = (((long)ac1 * 4 + x3) + 2)/4;
	x1 = ac3 * b6 >> 13;
	x2 = (b1 * (b6 * b6 >> 12)) >> 16;
	x3 = ((x1 + x2) + 2) >> 2;
	
	b4 = (ac4 * (unsigned long) (x3 + 32768)) >> 15;
	b7 = ((unsigned long) up - b3) * (50000 >> OSS);
	
	if( b7 < 0x80000000)
		p = (b7 * 2) / b4 ;
	else  
		p = (b7 / b4) * 2;
	
	x1 = (p >> 8) * (p >> 8);
	x1 = (x1 * 3038) >> 16;
	x2 = (-7357 * p) >> 16;
	pressure = p + ((x1 + x2 + 3791) >> 4);

	LCD_Int(COLUMN1,LINE5,pressure);
	return pressure;
}


