#include "HMC5883.h"
#include "main.h"
#include <math.h>

extern I2C_HandleTypeDef hi2c2;

static HAL_StatusTypeDef HMC5883L_WriteReg(uint8_t reg, uint8_t data)
{
    return HAL_I2C_Mem_Write(&hi2c2,
                             HMC5883L_ADDR,
                             reg,
                             I2C_MEMADD_SIZE_8BIT,
                             &data,
                             1,
                             100);
}

void HMC5883L_Initial(void)
{
    HMC5883L_WriteReg(HMC5883L_CONFIG_A, 0x14);  // 采样1次，75Hz，正常
    HMC5883L_WriteReg(HMC5883L_CONFIG_B, 0x20);  // 增益设置
    HMC5883L_WriteReg(HMC5883L_MODE, 0x00);      // 连续测量模式
}

HAL_StatusTypeDef HMC5883L_ReadRaw(int16_t *x, int16_t *y, int16_t *z)
{
    uint8_t buf[6];

    if (HAL_I2C_Mem_Read(&hi2c2,
                         HMC5883L_ADDR,
                         HMC5883L_DATA_X_H,
                         I2C_MEMADD_SIZE_8BIT,
                         buf,
                         6,
                         100) != HAL_OK)
    {
        return HAL_ERROR;
    }

    *x = (int16_t)((buf[0] << 8) | buf[1]);
    *z = (int16_t)((buf[2] << 8) | buf[3]);
    *y = (int16_t)((buf[4] << 8) | buf[5]);

    return HAL_OK;
}

/* 只返回未补偿航向角 */
float HMC5883L_ReadHeading(void)
{
    int16_t x, y, z;
    float heading;

    HAL_Delay(5);

    if (HMC5883L_ReadRaw(&x, &y, &z) != HAL_OK)
    {
        return 0.0f;
    }

    heading = atan2f((float)y, (float)x) * 180.0f / 3.1415926f;
    if (heading < 0.0f)
    {
        heading += 360.0f;
    }

    return heading;
}
