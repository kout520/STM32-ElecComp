#include "headfile.h"

uint8_t B1_state;
uint8_t B1_last_state;
//uint8_t B2_state;
//uint8_t B2_last_state;
//uint8_t B3_state;
//uint8_t B3_last_state;
//uint8_t B4_state;
//uint8_t B4_last_state;
uint8_t key_flag = 0;


uint8_t e = 0;


void key_scan(void)
{
    static uint32_t last_tick;
    uint32_t current_tick = HAL_GetTick();
    if(current_tick - last_tick <20)return;
    last_tick = current_tick;
    B1_state = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_4);
//    B2_state = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_1);
//    B3_state = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_2);
//    B4_state = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0);
    if(B1_state == 0 && B1_last_state == 1)
    {

        
    }
//    if(B2_state == 0 && B2_last_state == 1)
//    {

//    }    
//    if(B3_state == 0 && B3_last_state == 1)
//    {

//    }    
//    if(B4_state == 0 && B4_last_state == 1)
//    {
//    }    
    
    B1_last_state  = B1_state;
//    B2_last_state  = B2_state;
//    B3_last_state  = B3_state;
//    B4_last_state  = B4_state;
}













