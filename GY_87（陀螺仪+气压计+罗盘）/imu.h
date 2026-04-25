#ifndef __IMU_H
#define __IMU_H

#include "stm32f4xx_hal.h"
#include <stdint.h>

typedef struct
{
    float roll;
    float pitch;
    float yaw;
    float temperature;   // µ•Œª£∫°„C
} EulerAngle_t;


void IMU_CalcEulerAngles(EulerAngle_t *euler);

#endif
