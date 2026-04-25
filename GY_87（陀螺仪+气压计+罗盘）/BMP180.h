#ifndef __BMP180_H
#define __BMP180_H

#include <stdint.h>

#define BMP180_ADDR        (0x77 << 1)   // BMP180/BMP085 7位地址通常是0x77

void BMP180_Initial(void);
long BMP180_Read_Temperature(void);
long BMP180_Read_Pressure(void);
long BMP180_Read(void);

#endif
