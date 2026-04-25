#include "BMP180.h"
#include "main.h"

extern I2C_HandleTypeDef hi2c2;

#define OSS 0   // 超采样设置：0~3，0为最简单模式

static int16_t  ac1, ac2, ac3;
static uint16_t ac4, ac5, ac6;
static int16_t  b1, b2, mb, mc, md;

static long temperature;
static long pressure;

/* 读2字节寄存器，返回有符号16位 */
static HAL_StatusTypeDef BMP180_ReadS16(uint8_t reg, int16_t *value)
{
    uint8_t buf[2];

    if (HAL_I2C_Mem_Read(&hi2c2,
                         BMP180_ADDR,
                         reg,
                         I2C_MEMADD_SIZE_8BIT,
                         buf,
                         2,
                         100) != HAL_OK)
    {
        return HAL_ERROR;
    }

    *value = (int16_t)((buf[0] << 8) | buf[1]);
    return HAL_OK;
}

/* 读2字节寄存器，返回无符号16位 */
static HAL_StatusTypeDef BMP180_ReadU16(uint8_t reg, uint16_t *value)
{
    uint8_t buf[2];

    if (HAL_I2C_Mem_Read(&hi2c2,
                         BMP180_ADDR,
                         reg,
                         I2C_MEMADD_SIZE_8BIT,
                         buf,
                         2,
                         100) != HAL_OK)
    {
        return HAL_ERROR;
    }

    *value = (uint16_t)((buf[0] << 8) | buf[1]);
    return HAL_OK;
}

/* 写单字节 */
static HAL_StatusTypeDef BMP180_WriteReg(uint8_t reg, uint8_t data)
{
    return HAL_I2C_Mem_Write(&hi2c2,
                             BMP180_ADDR,
                             reg,
                             I2C_MEMADD_SIZE_8BIT,
                             &data,
                             1,
                             100);
}
void BMP180_Initial(void)
{
    BMP180_ReadS16(0xAA, &ac1);
    BMP180_ReadS16(0xAC, &ac2);
    BMP180_ReadS16(0xAE, &ac3);

    BMP180_ReadU16(0xB0, &ac4);
    BMP180_ReadU16(0xB2, &ac5);
    BMP180_ReadU16(0xB4, &ac6);

    BMP180_ReadS16(0xB6, &b1);
    BMP180_ReadS16(0xB8, &b2);
    BMP180_ReadS16(0xBA, &mb);
    BMP180_ReadS16(0xBC, &mc);
    BMP180_ReadS16(0xBE, &md);
}

long BMP180_Read_Temperature(void)
{
    uint8_t buf[2];
    uint16_t temp_ut;

    BMP180_WriteReg(0xF4, 0x2E);   // 启动温度转换
    HAL_Delay(5);

    if (HAL_I2C_Mem_Read(&hi2c2,
                         BMP180_ADDR,
                         0xF6,
                         I2C_MEMADD_SIZE_8BIT,
                         buf,
                         2,
                         100) != HAL_OK)
    {
        return 0;
    }

    temp_ut = (uint16_t)((buf[0] << 8) | buf[1]);
    return (long)temp_ut;
}

long BMP180_Read_Pressure(void)
{
    uint8_t buf[3];
    uint32_t up;

    BMP180_WriteReg(0xF4, 0x34 + (OSS << 6));   // 启动气压转换
    HAL_Delay(5);

    if (HAL_I2C_Mem_Read(&hi2c2,
                         BMP180_ADDR,
                         0xF6,
                         I2C_MEMADD_SIZE_8BIT,
                         buf,
                         3,
                         100) != HAL_OK)
    {
        return 0;
    }

    up = ((uint32_t)buf[0] << 16) | ((uint32_t)buf[1] << 8) | buf[2];
    up >>= (8 - OSS);

    return (long)up;
}

long BMP180_Read(void)
{
    long ut;
    long up;
    long x1, x2, x3, b3, b5, b6, p;
    unsigned long b4, b7;

    ut = BMP180_Read_Temperature();
    up = BMP180_Read_Pressure();

    x1 = ((long)ut - ac6) * ac5 >> 15;
    x2 = ((long)mc << 11) / (x1 + md);
    b5 = x1 + x2;
    temperature = (b5 + 8) >> 4;   // 0.1°C

    b6 = b5 - 4000;
    x1 = (b2 * (b6 * b6 >> 12)) >> 11;
    x2 = (ac2 * b6) >> 11;
    x3 = x1 + x2;
    b3 = ((((long)ac1 * 4 + x3) << OSS) + 2) / 4;

    x1 = (ac3 * b6) >> 13;
    x2 = (b1 * (b6 * b6 >> 12)) >> 16;
    x3 = ((x1 + x2) + 2) >> 2;

    b4 = ((unsigned long)ac4 * (unsigned long)(x3 + 32768)) >> 15;
    b7 = ((unsigned long)up - b3) * (50000 >> OSS);

    if (b7 < 0x80000000)
    {
        p = (b7 * 2) / b4;
    }
    else
    {
        p = (b7 / b4) * 2;
    }

    x1 = (p >> 8) * (p >> 8);
    x1 = (x1 * 3038) >> 16;
    x2 = (-7357 * p) >> 16;

    pressure = p + ((x1 + x2 + 3791) >> 4);

    //LCD_Int(COLUMN1, LINE5, pressure);

    return pressure;
}





