//使用方法
//记得给输入引脚拉高电阻

#include "headfile.h"

typedef struct {
    uint8_t current_state;
    uint8_t last_state;
    uint32_t last_press_time;
    uint8_t click_count;
    uint8_t debounce_flag;
} KeyState;

KeyState B1 = {0}, B2 = {0}, B3 = {0}, B4 = {0};
uint8_t error_count, error_flag;
uint16_t error_time_5s;

// Click detection parameters
#define DEBOUNCE_TIME_MS    20
#define DOUBLE_CLICK_TIME_MS 250  // Max time between clicks for double-click
#define MULTI_CLICK_TIME_MS  400  // Max time for multi-click detection

void key_scan(void) {
    static uint32_t last_tick;
    uint32_t current_tick = HAL_GetTick();
    
    if(current_tick - last_tick < DEBOUNCE_TIME_MS) return;
    last_tick = current_tick;

    // Read all buttons
    B1.current_state = HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_3);
    B2.current_state = HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_4);
    B3.current_state = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0);
//    B4.current_state = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0);

    if(B1.current_state == 0 && B1.last_state == 1) { 
        if(!B1.debounce_flag) {
            B1.debounce_flag = 1;
            if(current_tick - B1.last_press_time < MULTI_CLICK_TIME_MS) {
                B1.click_count++;
            } else {
                B1.click_count = 1;  
            }
            B1.last_press_time = current_tick;
        }
    } 
    else if(B1.current_state == 1 && B1.last_state == 0) {  
        B1.debounce_flag = 0;
    }

    if((current_tick - B1.last_press_time >= MULTI_CLICK_TIME_MS) && (B1.click_count > 0)) {
        switch(B1.click_count) {
            case 1: // Single click
                HAL_GPIO_TogglePin(GPIOE, GPIO_PIN_5);
                break;
                
            case 2: // Double click
                HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_5);
                break;
                
            case 3: // Triple click
                HAL_GPIO_TogglePin(GPIOE, GPIO_PIN_5);
                break;
        }
        B1.click_count = 0; 
    }
    if(B2.current_state == 0 && B2.last_state == 1) { 
        HAL_GPIO_TogglePin(GPIOE, GPIO_PIN_5);
    }

    B1.last_state = B1.current_state;
    B2.last_state = B2.current_state;
    B3.last_state = B3.current_state;
//    B4.last_state = B4.current_state;
}


