#ifndef __HEADFILE_H
#define __HEADFILE_H

#include "stm32f1xx_hal.h"

#include "main.h"
#include "cmsis_os.h"
#include "i2c.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"
#include "fsmc.h"

#include "string.h"
#include "stdio.h"
#include "stdint.h"

#include "lcd.h"
#include "fun.h"
#include "fun1.h"
#include "JLNU_OLED.h"

#include "motor.h"
#include "encoder.h"
#include "pid.h"

extern PIDController Speed_Pid_L;  // 左电机 PID 控制器
extern float output_L;
extern float target_speed_L;
extern Encoder Encoder_L;          // 左电机编码器
extern int32_t pulse_count_L;
extern float kp;
extern float ki;
extern float kd;

#endif
