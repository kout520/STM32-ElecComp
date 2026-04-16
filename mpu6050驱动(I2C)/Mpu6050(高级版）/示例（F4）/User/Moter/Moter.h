#ifndef __MOTER_H
#define	__MOTER_H

#include "stm32f4xx.h"



void  Moter_init (void);
void Moter_Ctrl_LR(int dummy_param);
// 线性插值函数1
double linear_interpolation_distance(double x);
double linear_interpolation_w(double x);
int angle_to_frequency(double angle_degrees) ;
#endif
