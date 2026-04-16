#ifndef __HC_SR04_H__
#define __HC_SR04_H__

#define TRIG_H  HAL_GPIO_WritePin(Trig_GPIO_Port,Trig_Pin,GPIO_PIN_SET)
#define TRIG_L  HAL_GPIO_WritePin(Trig_GPIO_Port,Trig_Pin,GPIO_PIN_RESET)
 
#define TRIG1_H  HAL_GPIO_WritePin(Trig1_GPIO_Port,Trig1_Pin,GPIO_PIN_SET)
#define TRIG1_L  HAL_GPIO_WritePin(Trig1_GPIO_Port,Trig1_Pin,GPIO_PIN_RESET)

#define TRIG2_H  HAL_GPIO_WritePin(Trig2_GPIO_Port,Trig2_Pin,GPIO_PIN_SET)
#define TRIG2_L  HAL_GPIO_WritePin(Trig2_GPIO_Port,Trig2_Pin,GPIO_PIN_RESET)

#define TRIG3_H  HAL_GPIO_WritePin(Trig3_GPIO_Port,Trig3_Pin,GPIO_PIN_SET)
#define TRIG3_L  HAL_GPIO_WritePin(Trig3_GPIO_Port,Trig3_Pin,GPIO_PIN_RESET)

void delay_us(uint32_t us);
void SR04_GetData(void);
extern float distant;
extern float distant1;      //≤‚¡øæ‡¿Î
extern float distant2;      //≤‚¡øæ‡¿Î
extern uint8_t sr_flag;
extern float distant3;      //≤‚¡øæ‡¿Î
extern uint8_t sr04_flag;

#endif

