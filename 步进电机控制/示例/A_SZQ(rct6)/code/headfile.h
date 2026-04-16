#ifndef __HEADFILE_H__
#define __HEADFILE_H__

#include "stm32f1xx_hal.h"

#include "main.h"
#include "dma.h"
#include "i2c.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

#include "string.h"
#include "stdio.h"
#include "stdint.h"

#include "bj_motor.h"
#include "oled.h"
#include "UserUsart1.h"
#include "fun.h"

extern uint8_t usart1_RecvBuf[USART1_BUF_SIZE+1];        // 接收数据环形缓冲区 


#endif
