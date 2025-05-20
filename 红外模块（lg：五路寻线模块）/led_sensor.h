#ifndef __LED_SENSOR_H__
#define __LED_SENSOR_H__

extern void Sensor(uint16_t value,uint16_t pos);
void Change_speed(uint16_t speed);

#define  L1 HAL_GPIO_ReadPin(GPIOF,GPIO_PIN_1)//×ó IN4 B4
#define  L2 HAL_GPIO_ReadPin(GPIOF,GPIO_PIN_2)//×ó IN3 B3
#define  M0 HAL_GPIO_ReadPin(GPIOF,GPIO_PIN_3) //ÖÐ¼ä
#define  R1 HAL_GPIO_ReadPin(GPIOF,GPIO_PIN_4)//ÓÒ IN2 B2
#define  R2 HAL_GPIO_ReadPin(GPIOF,GPIO_PIN_5) //ÓÒ IN1 B1




#endif

