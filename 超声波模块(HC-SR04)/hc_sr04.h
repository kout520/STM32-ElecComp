#ifndef __HC_SR04_H__
#define __HC_SR04_H__

#define TRIG_H  HAL_GPIO_WritePin(Trig_GPIO_Port,Trig_Pin,GPIO_PIN_SET)
#define TRIG_L  HAL_GPIO_WritePin(Trig_GPIO_Port,Trig_Pin,GPIO_PIN_RESET)
 
void delay_us(uint32_t us);
void SR04_GetData(void);




#endif

