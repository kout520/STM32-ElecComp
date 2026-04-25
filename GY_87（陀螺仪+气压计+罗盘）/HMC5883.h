#ifndef __HMC5883_H
#define __HMC5883_H

#include "stm32f4xx_hal.h"
#include <stdint.h>

#define HMC5883L_ADDR      (0x1E << 1)

/* ¼Ä´æÆ÷ */
#define HMC5883L_CONFIG_A  0x00
#define HMC5883L_CONFIG_B  0x01
#define HMC5883L_MODE      0x02
#define HMC5883L_DATA_X_H  0x03

void HMC5883L_Initial(void);
HAL_StatusTypeDef HMC5883L_ReadRaw(int16_t *x, int16_t *y, int16_t *z);
float HMC5883L_ReadHeading(void);

#endif
