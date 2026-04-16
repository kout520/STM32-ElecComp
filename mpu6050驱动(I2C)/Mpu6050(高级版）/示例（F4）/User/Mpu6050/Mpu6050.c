#include "mpu6050.h"
#include "stm32f4xx.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include <math.h>

// 全局变量定义
QuaternionFilter quat_filter;
SensorCalibration sensor_cal = {0};
uint8_t calibration_in_progress = 0;

// 创建角度数据队列（可在外部使用）
QueueHandle_t xAngleQueue = NULL;

// ==================== I2C和初始化函数 ====================

void MPU6050_GPIO_Init(void) {
    GPIO_InitTypeDef GPIO_InitStructure;
    
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
    
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource6, GPIO_AF_I2C1);
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource7, GPIO_AF_I2C1);
}

void MPU6050_I2C_Init(void) {
    I2C_InitTypeDef I2C_InitStructure;
    
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);
    
    I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
    I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;
    I2C_InitStructure.I2C_OwnAddress1 = 0x00;
    I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
    I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
    I2C_InitStructure.I2C_ClockSpeed = 400000;
    
    I2C_Init(I2C1, &I2C_InitStructure);
    I2C_Cmd(I2C1, ENABLE);
}

uint8_t MPU6050_Init(void) {
    uint8_t check;
    
    MPU6050_GPIO_Init();
    MPU6050_I2C_Init();
    
    // 检测设备
    check = I2C_ReadByte(MPU6050_ADDR, WHO_AM_I);
    if (check != 0x68) {
        return 1;  // 设备未连接
    }
    
    // 配置MPU6050
    I2C_WriteByte(MPU6050_ADDR, PWR_MGMT_1, 0x00);  // 唤醒
    I2C_WriteByte(MPU6050_ADDR, GYRO_CONFIG, 0x18); // ±2000度/秒
    I2C_WriteByte(MPU6050_ADDR, ACCEL_CONFIG, 0x18);// ±16g
    I2C_WriteByte(MPU6050_ADDR, SMPLRT_DIV, 0x07);  // 1kHz采样率
    I2C_WriteByte(MPU6050_ADDR, CONFIG, 0x03);      // 陀螺仪DLPF 44Hz
    
    // 初始化四元数滤波器
    QuaternionFilter_Init(&quat_filter, 0.8f, 0.001f, 0.005f);
    
    // 创建角度数据队列
    xAngleQueue = xQueueCreate(10, sizeof(Angles));
    
    return 0;
}

void MPU6050_Read_Data(MPU6050_Data *data) {
    uint8_t buffer[14];
    
    I2C_ReadBytes(MPU6050_ADDR, ACCEL_XOUT_H, 14, buffer);
    
    data->Accel_X = (int16_t)((buffer[0] << 8) | buffer[1]);
    data->Accel_Y = (int16_t)((buffer[2] << 8) | buffer[3]);
    data->Accel_Z = (int16_t)((buffer[4] << 8) | buffer[5]);
    data->Temperature = (int16_t)((buffer[6] << 8) | buffer[7]);
    data->Gyro_X = (int16_t)((buffer[8] << 8) | buffer[9]);
    data->Gyro_Y = (int16_t)((buffer[10] << 8) | buffer[11]);
    data->Gyro_Z = (int16_t)((buffer[12] << 8) | buffer[13]);
	
	
	    data->Gyro_X+=20;
	  data->Gyro_Y-=70;
	  data->Gyro_Z+=20;
	  if(data->Gyro_X<=20&&data->Gyro_X>=-20)data->Gyro_X=0;
	  if(data->Gyro_Y<=20&&data->Gyro_Y>=-20)data->Gyro_Y=0;
		if(data->Gyro_Z<=20&&data->Gyro_Z>=-20)data->Gyro_Z=0;
	
}

void MPU6050_Convert_To_Real(MPU6050_Data *data) {
    // 加速度计转换 (±16g范围)
    data->Ax = data->Accel_X / 2048.0f;
    data->Ay = data->Accel_Y / 2048.0f;
    data->Az = data->Accel_Z / 2048.0f;
    
    // 陀螺仪转换 (±2000度/秒范围)
    data->Gx = data->Gyro_X / 16.4f;
    data->Gy = data->Gyro_Y / 16.4f;
    data->Gz = data->Gyro_Z / 16.4f;
    
    // 温度转换
    data->Temp = data->Temperature / 340.0f + 36.53f;
    
    // 应用软件校准
    MPU6050_Apply_Calibration(data);
}

// ==================== I2C通信函数 ====================

void I2C_WriteByte(uint8_t devAddr, uint8_t regAddr, uint8_t data) {
    portENTER_CRITICAL();
    
    while (I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY));
    
    I2C_GenerateSTART(I2C1, ENABLE);
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));
    
    I2C_Send7bitAddress(I2C1, devAddr, I2C_Direction_Transmitter);
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));
    
    I2C_SendData(I2C1, regAddr);
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED));
    
    I2C_SendData(I2C1, data);
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED));
    
    I2C_GenerateSTOP(I2C1, ENABLE);
    
    portEXIT_CRITICAL();
}

uint8_t I2C_ReadByte(uint8_t devAddr, uint8_t regAddr) {
    uint8_t data;
    
    portENTER_CRITICAL();
    
    while (I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY));
    
    I2C_GenerateSTART(I2C1, ENABLE);
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));
    
    I2C_Send7bitAddress(I2C1, devAddr, I2C_Direction_Transmitter);
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));
    
    I2C_SendData(I2C1, regAddr);
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED));
    
    I2C_GenerateSTART(I2C1, ENABLE);
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));
    
    I2C_Send7bitAddress(I2C1, devAddr, I2C_Direction_Receiver);
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED));
    
    I2C_AcknowledgeConfig(I2C1, DISABLE);
    I2C_GenerateSTOP(I2C1, ENABLE);
    
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_RECEIVED));
    data = I2C_ReceiveData(I2C1);
    
    I2C_AcknowledgeConfig(I2C1, ENABLE);
    
    portEXIT_CRITICAL();
    
    return data;
}

void I2C_ReadBytes(uint8_t devAddr, uint8_t regAddr, uint8_t length, uint8_t *data) {
    portENTER_CRITICAL();
    
    while (I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY));
    
    I2C_GenerateSTART(I2C1, ENABLE);
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));
    
    I2C_Send7bitAddress(I2C1, devAddr, I2C_Direction_Transmitter);
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));
    
    I2C_SendData(I2C1, regAddr);
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED));
    
    I2C_GenerateSTART(I2C1, ENABLE);
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));
    
    I2C_Send7bitAddress(I2C1, devAddr, I2C_Direction_Receiver);
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED));
    
    while (length) {
        if (length == 1) {
            I2C_AcknowledgeConfig(I2C1, DISABLE);
            I2C_GenerateSTOP(I2C1, ENABLE);
        }
        
        while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_RECEIVED));
        *data++ = I2C_ReceiveData(I2C1);
        length--;
    }
    
    I2C_AcknowledgeConfig(I2C1, ENABLE);
    
    portEXIT_CRITICAL();
}

// ==================== 四元数算法实现 ====================

void QuaternionFilter_Init(QuaternionFilter *filter, float kp, float ki, float dt) {
    filter->quat.q0 = 1.0f;
    filter->quat.q1 = 0.0f;
    filter->quat.q2 = 0.0f;
    filter->quat.q3 = 0.0f;
    filter->kp = kp;
    filter->ki = ki;
    filter->integralFBx = 0.0f;
    filter->integralFBy = 0.0f;
    filter->integralFBz = 0.0f;
    filter->last_time = xTaskGetTickCount();
    filter->dt = dt;
    filter->call_count = 0;
    filter->gyro_bias[0] = 0.0f;
    filter->gyro_bias[1] = 0.0f;
    filter->gyro_bias[2] = 0.0f;
}

void Quaternion_Normalize(Quaternion *q) {
    float norm = sqrtf(q->q0*q->q0 + q->q1*q->q1 + q->q2*q->q2 + q->q3*q->q3);
    if (norm > 0.0f) {
        float inv_norm = 1.0f / norm;
        q->q0 *= inv_norm;
        q->q1 *= inv_norm;
        q->q2 *= inv_norm;
        q->q3 *= inv_norm;
    }
}

void Quaternion_To_Euler(Quaternion *q, Angles *angles) {
    float sinr_cosp, cosr_cosp, sinp, siny_cosp, cosy_cosp;
    
    // 计算横滚角 (绕X轴)
    sinr_cosp = 2.0f * (q->q0 * q->q1 + q->q2 * q->q3);
    cosr_cosp = 1.0f - 2.0f * (q->q1 * q->q1 + q->q2 * q->q2);
    angles->roll = atan2f(sinr_cosp, cosr_cosp) * 180.0f / M_PI;
    
    // 计算俯仰角 (绕Y轴)
    sinp = 2.0f * (q->q0 * q->q2 - q->q3 * q->q1);
    if (fabsf(sinp) >= 1.0f) {
        angles->pitch = copysignf(M_PI / 2.0f, sinp) * 180.0f / M_PI;
    } else {
        angles->pitch = asinf(sinp) * 180.0f / M_PI;
    }
    
    // 计算偏航角 (绕Z轴)
    siny_cosp = 2.0f * (q->q0 * q->q3 + q->q1 * q->q2);
    cosy_cosp = 1.0f - 2.0f * (q->q2 * q->q2 + q->q3 * q->q3);
    angles->yaw = atan2f(siny_cosp, cosy_cosp) * 180.0f / M_PI;
}

void Quaternion_Update(QuaternionFilter *filter, MPU6050_Data *data) {
    TickType_t current_time;
    float dt;
    float ax, ay, az, gx, gy, gz;
    float norm;
    float q0, q1, q2, q3;
    float vx, vy, vz;
    float ex, ey, ez;
    float q0_dot, q1_dot, q2_dot, q3_dot;
    const float integral_limit = 0.1f;
    
    current_time = xTaskGetTickCount();
    
    // 计算实际时间间隔（秒）
    if (current_time >= filter->last_time) {
        dt = (current_time - filter->last_time) * portTICK_PERIOD_MS / 1000.0f;
    } else {
        dt = filter->dt; // 处理时间回绕
    }
    
    // 时间间隔异常处理
    if (dt <= 0.0f || dt > 0.1f) {
        dt = filter->dt;
    }
    
    filter->last_time = current_time;
    filter->call_count++;
    
    ax = data->Ax; ay = data->Ay; az = data->Az;
    gx = data->Gx; gy = data->Gy; gz = data->Gz;
    
    // 应用陀螺仪零偏补偿
    gx -= filter->gyro_bias[0];
    gy -= filter->gyro_bias[1];
    gz -= filter->gyro_bias[2];
    
    // 将角速度从度/秒转换为弧度/秒
    gx *= M_PI / 180.0f;
    gy *= M_PI / 180.0f;
    gz *= M_PI / 180.0f;
    
    // 归一化加速度计数据
    norm = sqrtf(ax*ax + ay*ay + az*az);
    if (norm > 0.0f) {
        ax /= norm;
        ay /= norm;
        az /= norm;
    }
    
    // 从当前四元数中提取重力方向
    q0 = filter->quat.q0; q1 = filter->quat.q1;
    q2 = filter->quat.q2; q3 = filter->quat.q3;
    
    // 估计的重力方向
    vx = 2.0f * (q1*q3 - q0*q2);
    vy = 2.0f * (q0*q1 + q2*q3);
    vz = q0*q0 - q1*q1 - q2*q2 + q3*q3;
    
    // 计算误差（叉积）
    ex = (ay*vz - az*vy);
    ey = (az*vx - ax*vz);
    ez = (ax*vy - ay*vx);
    
    // 积分误差（带限幅）
    filter->integralFBx += filter->ki * ex * dt;
    filter->integralFBy += filter->ki * ey * dt;
    filter->integralFBz += filter->ki * ez * dt;
    
    // 积分限幅
    if (filter->integralFBx > integral_limit) filter->integralFBx = integral_limit;
    if (filter->integralFBx < -integral_limit) filter->integralFBx = -integral_limit;
    if (filter->integralFBy > integral_limit) filter->integralFBy = integral_limit;
    if (filter->integralFBy < -integral_limit) filter->integralFBy = -integral_limit;
    if (filter->integralFBz > integral_limit) filter->integralFBz = integral_limit;
    if (filter->integralFBz < -integral_limit) filter->integralFBz = -integral_limit;
    
    // 应用比例和积分反馈
    gx += filter->kp * ex + filter->integralFBx;
    gy += filter->kp * ey + filter->integralFBy;
    gz += filter->kp * ez + filter->integralFBz;
    
    // 四元数微分方程
    q0_dot = 0.5f * (-q1 * gx - q2 * gy - q3 * gz);
    q1_dot = 0.5f * ( q0 * gx + q2 * gz - q3 * gy);
    q2_dot = 0.5f * ( q0 * gy - q1 * gz + q3 * gx);
    q3_dot = 0.5f * ( q0 * gz + q1 * gy - q2 * gx);
    
    // 一阶积分更新四元数
    filter->quat.q0 += q0_dot * dt;
    filter->quat.q1 += q1_dot * dt;
    filter->quat.q2 += q2_dot * dt;
    filter->quat.q3 += q3_dot * dt;
    
    // 归一化四元数
    Quaternion_Normalize(&filter->quat);
}

void MPU6050_Calculate_Angles(MPU6050_Data *data, Angles *angles) {
    Quaternion_Update(&quat_filter, data);
    Quaternion_To_Euler(&quat_filter.quat, angles);
    angles->timestamp = xTaskGetTickCount();
}

void MPU6050_Reset_Orientation(void) {
    QuaternionFilter_Init(&quat_filter, 0.8f, 0.001f, 0.005f);
}

// ==================== 校准函数 ====================

void MPU6050_Start_Calibration(void) {
    int i;
    calibration_in_progress = 1;
    sensor_cal.calibration_samples = 0;
    
    // 重置校准数据
    for(i = 0; i < 3; i++) {
        sensor_cal.accel_offset[i] = 0.0f;
        sensor_cal.gyro_offset[i] = 0.0f;
    }
    
    // 重置零偏估计
    for(i = 0; i < 3; i++) {
        quat_filter.gyro_bias[i] = 0.0f;
    }
}

uint8_t MPU6050_Is_Calibrating(void) {
    return calibration_in_progress;
}

void MPU6050_Apply_Calibration(MPU6050_Data *data) {
    int i;
    const float deadzone = 0.5f;
    
    if (calibration_in_progress) {
        // 收集校准数据
        sensor_cal.accel_offset[0] += data->Ax;
        sensor_cal.accel_offset[1] += data->Ay;
        sensor_cal.accel_offset[2] += data->Az;
        
        sensor_cal.gyro_offset[0] += data->Gx;
        sensor_cal.gyro_offset[1] += data->Gy;
        sensor_cal.gyro_offset[2] += data->Gz;
        
        sensor_cal.calibration_samples++;
        
        // 校准完成（收集500个样本）
        if (sensor_cal.calibration_samples >= 500) {
            for(i = 0; i < 3; i++) {
                sensor_cal.accel_offset[i] /= sensor_cal.calibration_samples;
                sensor_cal.gyro_offset[i] /= sensor_cal.calibration_samples;
            }
            
            // Z轴加速度计应该接近1g（重力）
            sensor_cal.accel_offset[2] -= 1.0f;
            
            calibration_in_progress = 0;
        }
    } else {
        // 应用校准
        data->Ax -= sensor_cal.accel_offset[0];
        data->Ay -= sensor_cal.accel_offset[1];
        data->Az -= sensor_cal.accel_offset[2];
        
        data->Gx -= sensor_cal.gyro_offset[0];
        data->Gy -= sensor_cal.gyro_offset[1];
        data->Gz -= sensor_cal.gyro_offset[2];
        
        // 软件死区处理
        if(fabsf(data->Gx) < deadzone) data->Gx = 0.0f;
        if(fabsf(data->Gy) < deadzone) data->Gy = 0.0f;
        if(fabsf(data->Gz) < deadzone) data->Gz = 0.0f;
    }
}

// ==================== FreeRTOS任务 ====================
