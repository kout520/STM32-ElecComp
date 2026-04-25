#include "MPU6050.h"
#include "main.h"

extern I2C_HandleTypeDef hi2c2;

/* 写单个寄存器 */
static HAL_StatusTypeDef MPU6050_WriteReg(uint8_t reg, uint8_t value)
{
    return HAL_I2C_Mem_Write(&hi2c2,
                             MPU6050_ADDR,
                             reg,
                             I2C_MEMADD_SIZE_8BIT,
                             &value,
                             1,
                             100);
}

/* 读单个寄存器 */
static HAL_StatusTypeDef MPU6050_ReadReg(uint8_t reg, uint8_t *value)
{
    return HAL_I2C_Mem_Read(&hi2c2,
                            MPU6050_ADDR,
                            reg,
                            I2C_MEMADD_SIZE_8BIT,
                            value,
                            1,
                            100);
}

/* 初始化 */
void MPU6050_Init(void)
{
    MPU6050_WriteReg(PWR_MGMT_1, 0x00);   // 唤醒
    HAL_Delay(100);

    MPU6050_WriteReg(SMPLRT_DIV, 0x07);   // 采样分频
    MPU6050_WriteReg(CONFIG, 0x06);       // DLPF 5Hz

    MPU6050_WriteReg(INT_PIN_CFG, 0x02);  // 旁路模式使能（给磁力计用）
    MPU6050_WriteReg(USER_CTRL, 0x00);    // 关闭I2C主机模式

    MPU6050_WriteReg(GYRO_CONFIG, 0x18);  // ±2000 dps
    MPU6050_WriteReg(ACCEL_CONFIG, 0x01);  // ±4g
}

/* 一次性读取全部原始数据 */
HAL_StatusTypeDef MPU6050_ReadAllRaw(MPU6050_RawData_t *data)
{
    uint8_t buf[14];

    if (HAL_I2C_Mem_Read(&hi2c2,
                         MPU6050_ADDR,
                         ACCEL_XOUT_H,
                         I2C_MEMADD_SIZE_8BIT,
                         buf,
                         14,
                         100) != HAL_OK)
    {
        return HAL_ERROR;
    }

    data->ax   = (int16_t)((buf[0] << 8) | buf[1]);
    data->ay   = (int16_t)((buf[2] << 8) | buf[3]);
    data->az   = (int16_t)((buf[4] << 8) | buf[5]);
    data->temp = (int16_t)((buf[6] << 8) | buf[7]);
    data->gx   = (int16_t)((buf[8] << 8) | buf[9]);
    data->gy   = (int16_t)((buf[10] << 8) | buf[11]);
    data->gz   = (int16_t)((buf[12] << 8) | buf[13]);

    return HAL_OK;
}

/* 只读陀螺仪 */
HAL_StatusTypeDef MPU6050_ReadGyroRaw(MPU6050_RawData_t *data)
{
    uint8_t buf[8];

    if (HAL_I2C_Mem_Read(&hi2c2,
                         MPU6050_ADDR,
                         GYRO_XOUT_H,
                         I2C_MEMADD_SIZE_8BIT,
                         buf,
                         8,
                         100) != HAL_OK)
    {
        return HAL_ERROR;
    }

    data->gx   = (int16_t)((buf[0] << 8) | buf[1]);
    data->gy   = (int16_t)((buf[2] << 8) | buf[3]);
    data->gz   = (int16_t)((buf[4] << 8) | buf[5]);
    data->temp = (int16_t)((buf[6] << 8) | buf[7]);

    return HAL_OK;
}

/* 只读加速度计 */
HAL_StatusTypeDef MPU6050_ReadAccelRaw(MPU6050_RawData_t *data)
{
    uint8_t buf[8];

    if (HAL_I2C_Mem_Read(&hi2c2,
                         MPU6050_ADDR,
                         ACCEL_XOUT_H,
                         I2C_MEMADD_SIZE_8BIT,
                         buf,
                         8,
                         100) != HAL_OK)
    {
        return HAL_ERROR;
    }

    data->ax   = (int16_t)((buf[0] << 8) | buf[1]);
    data->ay   = (int16_t)((buf[2] << 8) | buf[3]);
    data->az   = (int16_t)((buf[4] << 8) | buf[5]);
    data->temp = (int16_t)((buf[6] << 8) | buf[7]);

    return HAL_OK;
}
float MPU6050_GetTemperature(int16_t raw_temp)
{
    return (float)raw_temp / 340.0f + 36.53f;
}




