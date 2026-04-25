#ifndef __MPU6050_H
#define __MPU6050_H

#include "stm32f4xx_hal.h"
#include <stdint.h>

#define MPU6050_ADDR        (0x68 << 1)

/* ¼Ä´æÆ÷µØÖ· */
#define MPU6050_WHO_AM_I    0x75
#define PWR_MGMT_1          0x6B
#define SMPLRT_DIV          0x19
#define CONFIG              0x1A
#define GYRO_CONFIG         0x1B
#define ACCEL_CONFIG        0x1C
#define INT_PIN_CFG         0x37
#define USER_CTRL           0x6A

#define ACCEL_XOUT_H        0x3B
#define TEMP_OUT_H          0x41
#define GYRO_XOUT_H         0x43

typedef struct
{
    int16_t ax;
    int16_t ay;
    int16_t az;
    int16_t gx;
    int16_t gy;
    int16_t gz;
    int16_t temp;
} MPU6050_RawData_t;

void MPU6050_Init(void);
float MPU6050_GetTemperature(int16_t raw_temp);


HAL_StatusTypeDef MPU6050_ReadAllRaw(MPU6050_RawData_t *data);
HAL_StatusTypeDef MPU6050_ReadGyroRaw(MPU6050_RawData_t *data);
HAL_StatusTypeDef MPU6050_ReadAccelRaw(MPU6050_RawData_t *data);

#endif
