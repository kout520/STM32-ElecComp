#include "headfile.h"

//按键参数
uint8_t key0_state;
uint8_t key0_last_state;
uint8_t key1_state;
uint8_t key1_last_state;
uint8_t key2_state;
uint8_t key2_last_state;

//lcd参数
uint8_t lcd_flag=0;
char text[30];
uint8_t x1,x2,x3,x4,x5,y1,y2,y3,y4,y5;
uint32_t z1,z2,z3,z4,z5;
uint8_t page_flag;
uint8_t reg_flag;

//光标闪烁控制（本次实验不用闪烁标注）
//uint32_t blink_timer = 0;
//uint8_t blink_state = 0;

// 定义最大显示位数
#define MAX_DIGITS 2  // 假设最大显示3位数

void key_scan(void)
{
    key0_state = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0);
    key1_state = HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_3);
    key2_state = HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_4);
    if(key0_state == 0 && key0_last_state == 1)
    {
        lcd_flag++;
        lcd_flag %=5;
        reg_flag = 0;
        HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_5);
    }
    if(key1_state == 0 && key1_last_state == 1)
    {
        HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_5);
            page_flag++;
            page_flag %=2;
            switch (page_flag)
            {
                case 0:
                    reg_flag = 0;
                    break;
                case 1:
                    reg_flag = 1;
                    break;
                default:
                    break;
        }
    }
    if(key2_state == 0 && key2_last_state == 1)
    {

        switch (reg_flag)
        {
            case 0:
                if(lcd_flag==0)
                {
                    x1++;
                    if(x1 > 99) x1 = 0;  // 限制最大值
                }
                else if(lcd_flag==1)
                {
                    x2++;
                    if(x2 > 99) x2 = 0;  // 限制最大值
                }
                else if(lcd_flag==2)
                {
                    x3++;
                    if(x3 > 99) x3 = 0;  // 限制最大值
                }
                else if(lcd_flag==3)
                {
                    x4++;
                    if(x4 > 99) x4 = 0;  // 限制最大值
                }
                else if(lcd_flag==4)
                {
                    x5++;
                    if(x5 > 99) x5 = 0;  // 限制最大值
                }
                break;
            case 1:
                if(lcd_flag==0)
                {
                    y1++;
                    if(y1 > 99) y1 = 0;  // 限制最大值
                }
                else if(lcd_flag==1)
                {
                    y2++;
                    if(y2 > 99) y2 = 0;  // 限制最大值
                }
                else if(lcd_flag==2)
                {
                    y3++;
                    if(y3 > 99) y3 = 0;  // 限制最大值
                }
                else if(lcd_flag==3)
                {
                    y4++;
                    if(y4 > 99) y4 = 0;  // 限制最大值
                }
                else if(lcd_flag==4)
                {
                    y5++;
                    if(y5 > 99) y5 = 0;  // 限制最大值
                }
                break;
            default:
                break;
        }
        
        HAL_GPIO_TogglePin(GPIOE, GPIO_PIN_5);
    }

    key0_last_state = key0_state;
    key1_last_state = key1_state;
    key2_last_state = key2_state;
}

void lcd_show(void)
{

    page_show();

}

void page_show(void)
{
    uint8_t i,c;
    z1 = x1 * y1;
    z2 = x2 * y2;
    z3 = x3 * y3;
    z4 = x4 * y4;
    z5 = x5 * y5;
    

    // 创建固定宽度的字符串缓冲区
    char x1_str[MAX_DIGITS + 1] = {0};
    char y1_str[MAX_DIGITS + 1] = {0};
    char x2_str[MAX_DIGITS + 1] = {0};
    char y2_str[MAX_DIGITS + 1] = {0};
    char x3_str[MAX_DIGITS + 1] = {0};
    char y3_str[MAX_DIGITS + 1] = {0};
    char x4_str[MAX_DIGITS + 1] = {0};
    char y4_str[MAX_DIGITS + 1] = {0};
    char x5_str[MAX_DIGITS + 1] = {0};
    char y5_str[MAX_DIGITS + 1] = {0};
    
    // 格式化数字为固定宽度
    sprintf(x1_str, "%*d", MAX_DIGITS, x1);
    sprintf(y1_str, "%*d", MAX_DIGITS, y1);
    sprintf(x2_str, "%*d", MAX_DIGITS, x2);
    sprintf(y2_str, "%*d", MAX_DIGITS, y2);
    sprintf(x3_str, "%*d", MAX_DIGITS, x3);
    sprintf(y3_str, "%*d", MAX_DIGITS, y3);
    sprintf(x4_str, "%*d", MAX_DIGITS, x4);
    sprintf(y4_str, "%*d", MAX_DIGITS, y4);
    sprintf(x5_str, "%*d", MAX_DIGITS, x5);
    sprintf(y5_str, "%*d", MAX_DIGITS, y5);
    
    
    switch (lcd_flag)
    {
        case 0:
            
            if(!reg_flag) //x被选中
            {   LCD_Fill(70,84, 100, 84, WHITE);
                i=16;c=46;
                sprintf(text, "%s X %s = %d", x1_str, y1_str, z1);
                
            }
            else 
            {
                LCD_Fill(16,84, 46, 84, WHITE);
                i=70;c=100;
                sprintf(text, "%s X %s = %d", x1_str, y1_str, z1);
                
            }
            LCD_DrawLine(i, 84, c, 84);//画线选择
            LCD_Draw_Circle(4, 72, 4);  //画圆标注
            LCD_DrawRectangle(10, 59, 200, 85);   // 画矩形标注
            LCD_ShowString(16,60,210,24,24,(uint8_t *)text);//数据显示
            //去除上一级
            LCD_Fill(10,204, 100, 204, WHITE);//下标线 
            LCD_Fill(10,179, 200, 179, WHITE);//上水平
            LCD_Fill(10,205, 200, 205, WHITE);//下水平
            LCD_Fill(10,179, 10, 205, WHITE);//左垂直
            LCD_Fill(200,179, 200, 205, WHITE);//右垂直
            
            break;
        case 1:
            if(!reg_flag) //x被选中
            {   LCD_Fill(70,114, 100, 114, WHITE);
                i=16;c=46;
                sprintf(text, "%s X %s = %d", x2_str, y2_str, z2);
                
            }
            else 
            {
                LCD_Fill(16,114, 46, 114, WHITE);
                i=70;c=100;
                sprintf(text, "%s X %s = %d", x2_str, y2_str, z2);
                
            }
            LCD_DrawLine(i, 114, c, 114);//画线选择 
            LCD_Draw_Circle(4, 102, 4);  //画圆标注
            LCD_DrawRectangle(10, 89, 200, 115);   // 画矩形标注
            LCD_ShowString(16,90,210,24,24,(uint8_t *)text);//数据显示            
            //去除上一级
            LCD_Fill(10,59, 200, 59, WHITE);//上水平
            LCD_Fill(10,85, 200, 85, WHITE);//下水平
            LCD_Fill(10,59, 10, 85, WHITE);//左垂直
            LCD_Fill(200,59, 200, 85, WHITE);//右垂直
            LCD_Fill(10,84, 100, 84, WHITE);//下标线 

            break;
        case 2:
            if(!reg_flag) //x被选中
            {   LCD_Fill(70,144, 100, 144, WHITE);
                i=16;c=46;
                sprintf(text, "%s X %s = %d", x3_str, y3_str, z3);
                
            }
            else 
            {
                LCD_Fill(16,144, 46, 144, WHITE);
                i=70;c=100;
                sprintf(text, "%s X %s = %d", x3_str, y3_str, z3);
                
            }
            LCD_Draw_Circle(4, 134, 4);  //画圆标注
            LCD_DrawRectangle(10, 119, 200, 145);   // 画矩形标注
            LCD_DrawLine(i, 144, c, 144);//画线选择
            LCD_ShowString(16,120,210,24,24,(uint8_t *)text);//数据显示
            //去除上一级
            LCD_Fill(10,89, 200, 89, WHITE);//上水平
            LCD_Fill(10,115, 200, 115, WHITE);//下水平
            LCD_Fill(10,89, 10, 115, WHITE);//左垂直
            LCD_Fill(200,89, 200, 115, WHITE);//右垂直
            LCD_Fill(10,114, 100, 114, WHITE);//下标线 
            break;
        case 3:
            if(!reg_flag) //x被选中
            {   LCD_Fill(70,174, 100, 174, WHITE);
                i=16;c=46;
                sprintf(text, "%s X %s = %d", x4_str, y4_str, z4);
                
            }
            else 
            {
                LCD_Fill(16,174, 46, 174, WHITE);
                i=70;c=100;
                sprintf(text, "%s X %s = %d", x4_str, y4_str, z4);
                
            }
            LCD_DrawLine(i, 174, c, 174);//画线选择
            LCD_Draw_Circle(4, 162, 4);  //画圆标注
            LCD_DrawRectangle(10, 149, 200, 175);   // 画矩形标注
            LCD_ShowString(16,150,210,24,24,(uint8_t *)text);//数据显示            
            //去除上一级
            LCD_Fill(10,144, 100, 144, WHITE);//下标线 
            LCD_Fill(10,119, 200, 119, WHITE);//上水平
            LCD_Fill(10,145, 200, 145, WHITE);//下水平
            LCD_Fill(10,119, 10, 145, WHITE);//左垂直
            LCD_Fill(200,119, 200, 145, WHITE);//右垂直

            break;
        case 4:

            if(!reg_flag) //x被选中
            {   LCD_Fill(70,204, 100, 204, WHITE);
                i=16;c=46;
                sprintf(text, "%s X %s = %d", x5_str, y5_str, z5);
                
            }
            else 
            {
                LCD_Fill(10,204, 46, 204, WHITE);
                i=70;c=100;
                sprintf(text, "%s X %s = %d", x5_str, y5_str, z5);
                
            }
            LCD_DrawLine(i, 204, c, 204);//画线选择
            LCD_Draw_Circle(4, 192, 4);  //画圆标注
            LCD_DrawRectangle(10, 179, 200, 205);   // 画矩形标注
            LCD_ShowString(16,180,210,24,24,(uint8_t *)text);//数据显示            
            //去除上一级
            LCD_Fill(10,174, 100, 174, WHITE);//下标线           
            LCD_Fill(10,149, 200, 149, WHITE);//上水平
            LCD_Fill(10,175, 200, 175, WHITE);//下水平
            LCD_Fill(10,149, 10, 175, WHITE);//左垂直
            LCD_Fill(200,149, 200, 175, WHITE);//右垂直

            break;
        default:
            break;
    }
}
/*测试代码
    //根据当前选中的变量
    if(reg_flag == 0) //x被选中
    {   LCD_Fill(76,144, 100, 144, WHITE);
        i=22;c=46;
        sprintf(text, "%s X %s = %d", x_str, y_str, z);
        LCD_DrawLine(22, 144, 46, 144);//画线选择
    }
    else if(reg_flag == 1) //y被选中
    {
        LCD_Fill(22,144, 46, 144, WHITE);
        i=76;c=100;
        sprintf(text, "%s X %s = %d", x_str, y_str, z);
        LCD_DrawLine(76, 144, 100, 144);//画线选择
    }
    //LCD_ShowxNum(100, 150, x, 5, 24, 0x80); // 单纯数字显示显示
    
    LCD_ShowString(16,120,210,24,24,(uint8_t *)text);//数据显示
    LCD_DrawLine(i, 144, c, 144);//画线选择
    LCD_Draw_Circle(4, 134, 4);  //画圆标注
    LCD_DrawRectangle(10, 120, 200, 150);   // 画矩形标注
}
*/

