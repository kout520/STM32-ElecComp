#ifndef __HMC5883_H
#define __HMC5883_H	

#include <stdint.h>

#define	HMC5883L_Addr   0x3C	//磁场传感器器件地址 

void		HMC5883L_Initial(void);
float		HMC5883L_Read(void);
 
#endif  
