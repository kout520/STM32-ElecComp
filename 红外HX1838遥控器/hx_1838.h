#ifndef __HX_1838_H
#define __HX_1838_H


#define REMOTE_ID 0      		   
#define RDATA HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_9)


uint8_t Remote_Scan(void);
void remote_app(void);

#endif


