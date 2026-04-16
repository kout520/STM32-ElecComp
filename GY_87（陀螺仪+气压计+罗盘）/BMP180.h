#ifndef __BMP180_H
#define __BMP180_H	

#define	BMP085_Addr     0xee	//气压传感器器件地址     对了

void	BMP180_Initial(void);
long	BMP180_Read_Temperature(void);
long	BMP180_Read_Pressure(void);
long	BMP180_Read(void);

#endif  
