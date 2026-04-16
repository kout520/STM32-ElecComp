#include "headfile.h"

//按键参数
uint8_t key0;
uint8_t key0_last;
uint8_t key1;
uint8_t key1_last;
uint8_t key2;
uint8_t key2_last;

uint8_t oled_flag=0;
uint8_t a,b,c,d;
char text1[20];
void OLEDINIT(void)
{
    OLED_Init();
    OLED_Clear();
    // 在 OLED 上显示“程序”
    OLED_ShowStr(0,0,"->",2);
    OLED_ShowStr(16,0,"kp",2);
//    OLED_ShowCN(16, 0, 0); // 从坐标 (0,0) 开始，显示 2 个中文字符
//    OLED_ShowCN(32, 0, 1); // 从坐标 (0,0) 开始，显示 2 个中文字符
    OLED_ShowStr(48,0,":",2);

    //OLED_ShowStr(0,0,"  ",2);
    OLED_ShowStr(16,2,"ki",2);
//    OLED_ShowCN(16, 2, 2); // 从坐标 (0,16) 开始，显示 2 个中文字符
//    OLED_ShowCN(32, 2, 3); // 从坐标 (0,16) 开始，显示 2 个中文字符
    OLED_ShowStr(48,2,":",2);

    
//    OLED_ShowCN(16, 4, 4); // 从坐标 (0,32) 开始，显示 2 个中文字符
//    OLED_ShowCN(32, 4, 5); // 从坐标 (0,32) 开始，显示 2 个中文字符    
    OLED_ShowStr(16,4,"kd",2);
    OLED_ShowStr(48,4,":",2);
    
    OLED_ShowStr(16,6,"spd",2);
//    OLED_ShowCN(16, 6, 6); // 从坐标 (0,0) 开始，显示 2 个中文字符
//    OLED_ShowCN(32, 6, 7); // 从坐标 (0,0) 开始，显示 2 个中文字符    
    OLED_ShowStr(48,6,":",2);

    
}

void OLED_show(void)
{
    sprintf(text1,"%.3f   ",kp);
    OLED_ShowStr(64,0,(uint8_t *)text1,2);
    sprintf(text1,"%.3f   ",ki);
    OLED_ShowStr(64,2,(uint8_t *)text1,2);
    sprintf(text1,"%.3f   ",kd);
    OLED_ShowStr(64,4,(uint8_t *)text1,2);
    sprintf(text1,"%.1f   ",target_speed_L);
    OLED_ShowStr(64,6,(uint8_t *)text1,2);
}
void key_scan1(void)
{
    key0 = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0);
    key1 = HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_3);
    key2 = HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_4);
    if(key0 == 0 && key0_last == 1)
    {
        oled_flag++;if(oled_flag>=4)oled_flag=0;
        
        if(oled_flag==1)
        {
            OLED_ShowStr(0,0,"  ",2);
            OLED_ShowStr(0,2,"->",2);
        }
        else if(oled_flag==2)
        {
            OLED_ShowStr(0,2,"  ",2);
            OLED_ShowStr(0,4,"->",2);            
        }
        else if(oled_flag==3)
        {
            OLED_ShowStr(0,4,"  ",2);
            OLED_ShowStr(0,6,"->",2);            
        }
        else 
        {
            OLED_ShowStr(0,6,"  ",2);
            OLED_ShowStr(0,0,"->",2);            
        }
    }
    if(key1 == 0 && key1_last == 1)
    {
        switch(oled_flag)
        {
            case 0:
                kp+=0.01;
                break;
            case 1:
                ki+=0.01;
                break;
            case 2:
                kd+=0.01;
                break;
            case 3:
                target_speed_L +=2;
                break;
            default:
                break;
        }
    }
    if(key2 == 0 && key2_last== 1)
    {
        switch(oled_flag)
        {
            case 0:
                kp-=0.01;
                break;
            case 1:
                ki-=0.01;
                break;
            case 2:
                kd-=0.01;
                break;
            case 3:
                target_speed_L -=2;
                break;
            default:
                break;
        }

    }

    key0_last = key0;
    key1_last = key1;
    key2_last = key2;
}







