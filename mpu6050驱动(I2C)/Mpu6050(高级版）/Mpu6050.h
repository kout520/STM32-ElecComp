#ifndef __MPU6050_H
#define __MPU6050_H

#include "stm32f4xx.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include <math.h>

// 定义M_PI如果不存在
#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

// MPU6050地址定义
#define MPU6050_ADDR 0xD0

// MPU6050寄存器地址
#define SMPLRT_DIV 0x19
#define CONFIG 0x1A
#define GYRO_CONFIG 0x1B
#define ACCEL_CONFIG 0x1C
#define ACCEL_XOUT_H 0x3B
#define ACCEL_XOUT_L 0x3C
#define ACCEL_YOUT_H 0x3D
#define ACCEL_YOUT_L 0x3E
#define ACCEL_ZOUT_H 0x3F
#define ACCEL_ZOUT_L 0x40
#define TEMP_OUT_H 0x41
#define TEMP_OUT_L 0x42
#define GYRO_XOUT_H 0x43
#define GYRO_XOUT_L 0x44
#define GYRO_YOUT_H 0x45
#define GYRO_YOUT_L 0x46
#define GYRO_ZOUT_H 0x47
#define GYRO_ZOUT_L 0x48
#define PWR_MGMT_1 0x6B
#define WHO_AM_I 0x75

// 角度数据结构
typedef struct {
    float pitch;  // 俯仰角 (度)
    float roll;   // 横滚角 (度)  
    float yaw;    // 偏航角 (度)
    TickType_t timestamp; // 时间戳
} Angles;

// MPU6050原始数据结构
typedef struct {
    int16_t Accel_X;
    int16_t Accel_Y;
    int16_t Accel_Z;
    int16_t Temperature;
    int16_t Gyro_X;
    int16_t Gyro_Y;
    int16_t Gyro_Z;
    
    // 转换后的实际值
    float Ax, Ay, Az;  // 加速度 (g)
    float Gx, Gy, Gz;  // 角速度 (度/秒)
    float Temp;        // 温度 (°C)
} MPU6050_Data;

// 四元数结构体
typedef struct {
    float q0, q1, q2, q3;  // 四元数分量: q0 + q1*i + q2*j + q3*k
} Quaternion;

// 四元数姿态解算器（FreeRTOS优化）
typedef struct {
    Quaternion quat;        // 当前四元数
    float kp;              // 比例系数
    float ki;              // 积分系数
    float integralFBx, integralFBy, integralFBz; // 积分误差
    TickType_t last_time;  // FreeRTOS时间戳
    float dt;              // 采样周期
    uint32_t call_count;   // 调用计数
    float gyro_bias[3];    // 陀螺仪零偏估计
} QuaternionFilter;

// 传感器校准结构
typedef struct {
    float accel_offset[3];
    float gyro_offset[3];
    uint32_t calibration_samples;
} SensorCalibration;

// 函数声明
void MPU6050_GPIO_Init(void);
void MPU6050_I2C_Init(void);
uint8_t MPU6050_Init(void);
void MPU6050_Read_Data(MPU6050_Data *data);
void MPU6050_Convert_To_Real(MPU6050_Data *data);
void I2C_WriteByte(uint8_t devAddr, uint8_t regAddr, uint8_t data);
uint8_t I2C_ReadByte(uint8_t devAddr, uint8_t regAddr);
void I2C_ReadBytes(uint8_t devAddr, uint8_t regAddr, uint8_t length, uint8_t *data);

// 四元数相关函数
void QuaternionFilter_Init(QuaternionFilter *filter, float kp, float ki, float dt);
void Quaternion_Update(QuaternionFilter *filter, MPU6050_Data *data);
void Quaternion_To_Euler(Quaternion *q, Angles *angles);
void Quaternion_Normalize(Quaternion *q);
void MPU6050_Calculate_Angles(MPU6050_Data *data, Angles *angles);
void MPU6050_Reset_Orientation(void);

// 校准函数
void MPU6050_Start_Calibration(void);
uint8_t MPU6050_Is_Calibrating(void);
void MPU6050_Apply_Calibration(MPU6050_Data *data);

// FreeRTOS任务
void MPU6050_Task(void *pvParameters);
extern QueueHandle_t xAngleQueue;

#endif