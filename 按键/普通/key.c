#include "headfile.h"

uint8_t B1_state;
uint8_t B1_last_state;
uint8_t B2_state;
uint8_t B2_last_state;
uint8_t B3_state;
uint8_t B3_last_state;
uint8_t B4_state;
uint8_t B4_last_state;

uint8_t error_count;
uint8_t error_flag;
uint16_t error_time_5s;

void key_scan(void)
{
    static uint32_t last_tick;
    uint32_t current_tick = HAL_GetTick();
    if(current_tick - last_tick <20)return;
    last_tick = current_tick;
    B1_state = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_0);
    B2_state = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_1);
    B3_state = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_2);
    B4_state = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0);
    if(B1_state == 0 && B1_last_state == 1)
    {
        if(lcd_flag)
        {
            if(psd_B1 == 10)
            {
                psd_B1 = 0;
            }
            else
            {
                psd_B1 ++;
                if(psd_B1 ==10)psd_B1 = 0;
            }                      
            
        }
    }
    if(B2_state == 0 && B2_last_state == 1)
    {
        if(lcd_flag)
        {
            if(psd_B2 == 10)
            {
                psd_B2 = 0;
            }
            else
            {
                psd_B2 ++;
                if(psd_B2 ==10)psd_B2 = 0;
            }
            
        }
    }    
    if(B3_state == 0 && B3_last_state == 1)
    {
        if(lcd_flag)
        {
            if(psd_B3 == 10)
            {
                psd_B3 = 0;
            }
            else
            {
                psd_B3 ++;
                if(psd_B3 ==10)psd_B3 = 0;
            }
            
        }
    }    
    if(B4_state == 0 && B4_last_state == 1)
    {
        if(lcd_flag)
        {
            if((right_B1 == psd_B1) && (right_B2 == psd_B2) && (right_B3 == psd_B3))
            {
                lcd_flag = 0;
                TIM2->PSC = 400-1;//频率为2000HZ
                TIM2->CCR2 = 10;//占空比为10/100 = 10%
                led1_state = 1;
                error_count = 0;
            }
            else
            {
                 psd_B1 = 10;
                 psd_B2 = 10;
                 psd_B3 = 10;
                error_count++;
                if(error_count >= 3)
                {
                    error_flag = 1;
                    
                }
            }
        }
    }    
    
    B1_last_state  = B1_state;
    B2_last_state  = B2_state;
    B3_last_state  = B3_state;
    B4_last_state  = B4_state;
}













