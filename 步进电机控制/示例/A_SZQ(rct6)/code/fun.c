#include "headfile.h"

void lcd_show(void)
{
    OLED_NewFrame();
    OLED_PrintString(0, 0, "123", &font16x16, OLED_COLOR_NORMAL);
    OLED_PrintString(0, 16, "123", &font16x16, OLED_COLOR_NORMAL);
    OLED_PrintString(0, 32, "123", &font16x16, OLED_COLOR_NORMAL);
    OLED_PrintString(0, 48,"123" , &font16x16, OLED_COLOR_NORMAL);
    OLED_ShowFrame();
}

//黑棋位置
void black_points(uint8_t point)
{
    switch(point)
    {
        case 1:
            //黑棋1
            BJMotor_distant(1,20);
            BJMotor_distant(2,-5.5);
            BJMotor_distant(3,4);
            HAL_Delay(15000);
            HAL_Delay(3000);
            BJMotor_distant(3,-4);
            BJMotor_distant(1,-20);
            BJMotor_distant(2,5.5);            
            break;
        case 2:
            //黑棋2
            BJMotor_distant(1,20);
            BJMotor_distant(2,-13);
            BJMotor_distant(3,4);
            HAL_Delay(15000);
            HAL_Delay(3000);
            BJMotor_distant(3,-4);
            BJMotor_distant(1,-20);
            BJMotor_distant(2,13);           
            break;
        case 3:
            //黑棋3
            BJMotor_distant(1,20);
            BJMotor_distant(2,-20.5);
            BJMotor_distant(3,4);
            HAL_Delay(16000);
            HAL_Delay(3000);
            BJMotor_distant(3,-4);
            BJMotor_distant(1,-20);
            BJMotor_distant(2,20.5);
            break;
        case 4:
            //黑棋4
            BJMotor_distant(1,20);
            BJMotor_distant(2,-28);
            BJMotor_distant(3,4);
            HAL_Delay(20000);
            HAL_Delay(3000);
            BJMotor_distant(3,-4);
            BJMotor_distant(1,-20);
            BJMotor_distant(2,28);            
            break;
        case 5:
            //黑棋4
            BJMotor_distant(1,20);
            BJMotor_distant(2,-35.5);
            BJMotor_distant(3,4);
            HAL_Delay(30000);
            HAL_Delay(3000);
            BJMotor_distant(3,-4);
            BJMotor_distant(1,-20);
            BJMotor_distant(2,35.5);             
            break;
        default:
        break;
    }
}
void QZ_points(uint8_t times)
{
    if(times==1)
    {
        //取棋
        BJMotor_distant(1,20);
        BJMotor_distant(2,-5.5);
        HAL_Delay(15000);

        HAL_Delay(5000);
        //（0，0）
        if(usart1_RecvBuf[1] == '0' && usart1_RecvBuf[3] == '0')
        {
        //下棋
        BJMotor_distant(1,-11.2);
        BJMotor_distant(2,-19.5);
        HAL_Delay(20000);
            
        //看棋
        BJMotor_distant(1,-8.8);
        BJMotor_distant(2,25);  
        }
        //（0，1）
        if(usart1_RecvBuf[1] == '0' && usart1_RecvBuf[3] == '1')
        {
        //下棋
        BJMotor_distant(1,-20);
        BJMotor_distant(2,-19.5);
        HAL_Delay(22000);
        //看棋
        BJMotor_distant(2,25);   
        }
        //（0，2）
        if(usart1_RecvBuf[1] == '0' && usart1_RecvBuf[3] == '2')
        {
        //下棋
        BJMotor_distant(1,-28.8);
        BJMotor_distant(2,-19.5);
        HAL_Delay(28000);
        //看棋
        BJMotor_distant(1,8.8);
        BJMotor_distant(2,25);   
        }
        //（1，0）
        if(usart1_RecvBuf[1] == '1' && usart1_RecvBuf[3] == '0')
        {
        //下棋
        BJMotor_distant(1,-11.2);
        BJMotor_distant(2,-11.5);
        HAL_Delay(19000);
        //看棋
        BJMotor_distant(1,-8.8);
        BJMotor_distant(2,17); 
        }
        //（1，1）
        if(usart1_RecvBuf[1] == '1' && usart1_RecvBuf[3] == '1')
        {
        //下棋
        BJMotor_distant(1,-20);
        BJMotor_distant(2,-11.5);
        HAL_Delay(18000);
        //看棋
        BJMotor_distant(2,17);  
        }
        //（1，2）
        if(usart1_RecvBuf[1] == '1' && usart1_RecvBuf[3] == '2')
        {
        //下棋
        BJMotor_distant(1,-28.8);
        BJMotor_distant(2,-11.5);
        HAL_Delay(25000);
        //看棋
        BJMotor_distant(1,8.8);
        BJMotor_distant(2,17);   
        }
        //（2，0）
        if(usart1_RecvBuf[1] == '2' && usart1_RecvBuf[3] == '0')
        {
        //下棋
        BJMotor_distant(1,-11.2);
        BJMotor_distant(2,-3.5);
        HAL_Delay(12000);
        //看棋
        BJMotor_distant(1,-8.8);
        BJMotor_distant(2,9);    
        }
        //（2，1）
        if(usart1_RecvBuf[1] == '2' && usart1_RecvBuf[3] == '1')
        {
        //下棋
        BJMotor_distant(1,-20);
        BJMotor_distant(2,-3.5);
        HAL_Delay(18000);
        //看棋
        BJMotor_distant(2,9); 
        }
        //（2，2）
        if(usart1_RecvBuf[1] == '2' && usart1_RecvBuf[3] == '2')
        {
        //下棋
        BJMotor_distant(1,-28.8);
        BJMotor_distant(2,-3.5);
        HAL_Delay(25000);
        //看棋
        BJMotor_distant(1,8.8);
        BJMotor_distant(2,9);   
        }
    }
    else if(times==2)
    {
        //取棋
        BJMotor_distant(1,20);
        BJMotor_distant(2,-13);
        HAL_Delay(15000);
        //(0,0)
        if(usart1_RecvBuf[1] == '0' && usart1_RecvBuf[3] == '0')
        {
        //下棋
        BJMotor_distant(1,-11.2);
        BJMotor_distant(2,-12);
        HAL_Delay(20000);
        //看棋
        BJMotor_distant(1,-8.8);
        BJMotor_distant(2,25);  
        }
        //（0，1）
        if(usart1_RecvBuf[1] == '0' && usart1_RecvBuf[3] == '1')
        {
        //下棋
        BJMotor_distant(1,-20);
        BJMotor_distant(2,-12);
        HAL_Delay(22000);
        //看棋
        BJMotor_distant(2,25);   
        }
        //（0，2）
        if(usart1_RecvBuf[1] == '0' && usart1_RecvBuf[3] == '2')
        {
        //下棋
        BJMotor_distant(1,-28.8);
        BJMotor_distant(2,-12);
        HAL_Delay(28000);
        //看棋
        BJMotor_distant(1,8.8);
        BJMotor_distant(2,25);   
        }
        //（1，0）
        if(usart1_RecvBuf[1] == '1' && usart1_RecvBuf[3] == '0')
        {
        //下棋
        BJMotor_distant(1,-11.2);
        BJMotor_distant(2,-4);
        HAL_Delay(19000);
        //看棋
        BJMotor_distant(1,-8.8);
        BJMotor_distant(2,17); 
        }
        //（1，1）
        if(usart1_RecvBuf[1] == '1' && usart1_RecvBuf[3] == '1')
        {
        //下棋
        BJMotor_distant(1,-20);
        BJMotor_distant(2,-4);
        HAL_Delay(18000);
        //看棋
        BJMotor_distant(2,17);  
        }
        //（1，2）
        if(usart1_RecvBuf[1] == '1' && usart1_RecvBuf[3] == '2')
        {
        //下棋
        BJMotor_distant(1,-28.8);
        BJMotor_distant(2,-4);
        HAL_Delay(25000);
        //看棋
        BJMotor_distant(1,8.8);
        BJMotor_distant(2,17);   
        }
        //（2，0）
        if(usart1_RecvBuf[1] == '2' && usart1_RecvBuf[3] == '0')
        {
        //下棋
        BJMotor_distant(1,-11.2);
        BJMotor_distant(2,4);
        HAL_Delay(12000);
        //看棋
        BJMotor_distant(1,-8.8);
        BJMotor_distant(2,9);    
        }
        //（2，1）
        if(usart1_RecvBuf[1] == '2' && usart1_RecvBuf[3] == '1')
        {
        //下棋
        BJMotor_distant(1,-20);
        BJMotor_distant(2,4);
        HAL_Delay(18000);
        //看棋
        BJMotor_distant(2,9); 
        }
        //（2，2）
        if(usart1_RecvBuf[1] == '2' && usart1_RecvBuf[3] == '2')
        {
        //下棋
        BJMotor_distant(1,-28.8);
        BJMotor_distant(2,4);
        HAL_Delay(25000);
        //看棋
        BJMotor_distant(1,8.8);
        BJMotor_distant(2,9);   
        }        
    }
    else if(times==3)
    {
        //取棋
        BJMotor_distant(1,20);
        BJMotor_distant(2,-20.5);        
        HAL_Delay(18000);
        //(0,0)
        if(usart1_RecvBuf[1] == '0' && usart1_RecvBuf[3] == '0')
        {
        //下棋
        BJMotor_distant(1,-11.2);
        BJMotor_distant(2,-4.5);
        HAL_Delay(20000);
        //看棋
        BJMotor_distant(1,-8.8);
        BJMotor_distant(2,25);  
        }
        //（0，1）
        if(usart1_RecvBuf[1] == '0' && usart1_RecvBuf[3] == '1')
        {
        //下棋
        BJMotor_distant(1,-20);
        BJMotor_distant(2,-4.5);
        HAL_Delay(22000);
        //看棋
        BJMotor_distant(2,25);   
        }
        //（0，2）
        if(usart1_RecvBuf[1] == '0' && usart1_RecvBuf[3] == '2')
        {
        //下棋
        BJMotor_distant(1,-28.8);
        BJMotor_distant(2,-4.5);
        HAL_Delay(28000);
        //看棋
        BJMotor_distant(1,8.8);
        BJMotor_distant(2,25);   
        }
        //（1，0）
        if(usart1_RecvBuf[1] == '1' && usart1_RecvBuf[3] == '0')
        {
        //下棋
        BJMotor_distant(1,-11.2);
        BJMotor_distant(2,3.5);
        HAL_Delay(19000);
        //看棋
        BJMotor_distant(1,-8.8);
        BJMotor_distant(2,17); 
        }
        //（1，1）
        if(usart1_RecvBuf[1] == '1' && usart1_RecvBuf[3] == '1')
        {
        //下棋
        BJMotor_distant(1,-20);
        BJMotor_distant(2,3.5);
        HAL_Delay(18000);
        //看棋
        BJMotor_distant(2,17);  
        }
        //（1，2）
        if(usart1_RecvBuf[1] == '1' && usart1_RecvBuf[3] == '2')
        {
        //下棋
        BJMotor_distant(1,-28.8);
        BJMotor_distant(2,3.5);
        HAL_Delay(25000);
        //看棋
        BJMotor_distant(1,8.8);
        BJMotor_distant(2,17);   
        }
        //（2，0）
        if(usart1_RecvBuf[1] == '2' && usart1_RecvBuf[3] == '0')
        {
        //下棋
        BJMotor_distant(1,-11.2);
        BJMotor_distant(2,11.5);
        HAL_Delay(12000);
        //看棋
        BJMotor_distant(1,-8.8);
        BJMotor_distant(2,9);    
        }
        //（2，1）
        if(usart1_RecvBuf[1] == '2' && usart1_RecvBuf[3] == '1')
        {
        //下棋
        BJMotor_distant(1,-20);
        BJMotor_distant(2,11.5);
        HAL_Delay(18000);
        //看棋
        BJMotor_distant(2,9); 
        }
        //（2，2）
        if(usart1_RecvBuf[1] == '2' && usart1_RecvBuf[3] == '2')
        {
        //下棋
        BJMotor_distant(1,-28.8);
        BJMotor_distant(2,11.5);
        HAL_Delay(25000);
        //看棋
        BJMotor_distant(1,8.8);
        BJMotor_distant(2,9);   
        }
    }
    else if(times==4)
    {
        //取棋
        BJMotor_distant(1,20);
        BJMotor_distant(2,-28);          
        HAL_Delay(24000);
        //(0,0)
        if(usart1_RecvBuf[1] == '0' && usart1_RecvBuf[3] == '0')
        {
        //下棋
        BJMotor_distant(1,-11.2);
        BJMotor_distant(2,3);
        HAL_Delay(20000);
        //看棋
        BJMotor_distant(1,-8.8);
        BJMotor_distant(2,25);  
        }
        //（0，1）
        if(usart1_RecvBuf[1] == '0' && usart1_RecvBuf[3] == '1')
        {
        //下棋
        BJMotor_distant(1,-20);
        BJMotor_distant(2,3);
        HAL_Delay(22000);
        //看棋
        BJMotor_distant(2,25);   
        }
        //（0，2）
        if(usart1_RecvBuf[1] == '0' && usart1_RecvBuf[3] == '2')
        {
        //下棋
        BJMotor_distant(1,-28.8);
        BJMotor_distant(2,3);
        HAL_Delay(28000);
        //看棋
        BJMotor_distant(1,8.8);
        BJMotor_distant(2,25);   
        }
        //（1，0）
        if(usart1_RecvBuf[1] == '1' && usart1_RecvBuf[3] == '0')
        {
        //下棋
        BJMotor_distant(1,-11.2);
        BJMotor_distant(2,11);
        HAL_Delay(19000);
        //看棋
        BJMotor_distant(1,-8.8);
        BJMotor_distant(2,17); 
        }
        //（1，1）
        if(usart1_RecvBuf[1] == '1' && usart1_RecvBuf[3] == '1')
        {
        //下棋
        BJMotor_distant(1,-20);
        BJMotor_distant(2,11);
        HAL_Delay(18000);
        //看棋
        BJMotor_distant(2,17);  
        }
        //（1，2）
        if(usart1_RecvBuf[1] == '1' && usart1_RecvBuf[3] == '2')
        {
        //下棋
        BJMotor_distant(1,-28.8);
        BJMotor_distant(2,11);
        HAL_Delay(25000);
        //看棋
        BJMotor_distant(1,8.8);
        BJMotor_distant(2,17);   
        }
        //（2，0）
        if(usart1_RecvBuf[1] == '2' && usart1_RecvBuf[3] == '0')
        {
        //下棋
        BJMotor_distant(1,-11.2);
        BJMotor_distant(2,19);
        HAL_Delay(20000);
        //看棋
        BJMotor_distant(1,-8.8);
        BJMotor_distant(2,9);    
        }
        //（2，1）
        if(usart1_RecvBuf[1] == '2' && usart1_RecvBuf[3] == '1')
        {
        //下棋
        BJMotor_distant(1,-20);
        BJMotor_distant(2,19);
        HAL_Delay(18000);
        //看棋
        BJMotor_distant(2,9); 
        }
        //（2，2）
        if(usart1_RecvBuf[1] == '2' && usart1_RecvBuf[3] == '2')
        {
        //下棋
        BJMotor_distant(1,-28.8);
        BJMotor_distant(2,19);
        HAL_Delay(25000);
        //看棋
        BJMotor_distant(1,8.8);
        BJMotor_distant(2,9);   
        }
    }
    else if(times==5)
    {
        //取棋
        BJMotor_distant(1,20);
        BJMotor_distant(2,-35.5);      
        HAL_Delay(30000);
        //(0,0)
        if(usart1_RecvBuf[1] == '0' && usart1_RecvBuf[3] == '0')
        {
        //下棋
        BJMotor_distant(1,-11.2);
        BJMotor_distant(2,10.5);
        HAL_Delay(20000);
        //看棋
        BJMotor_distant(1,-8.8);
        BJMotor_distant(2,25);  
        }
        //（0，1）
        if(usart1_RecvBuf[1] == '0' && usart1_RecvBuf[3] == '1')
        {
        //下棋
        BJMotor_distant(1,-20);
        BJMotor_distant(2,10.5);
        HAL_Delay(22000);
        //看棋
        BJMotor_distant(2,25);   
        }
        //（0，2）
        if(usart1_RecvBuf[1] == '0' && usart1_RecvBuf[3] == '2')
        {
        //下棋
        BJMotor_distant(1,-28.8);
        BJMotor_distant(2,10.5);
        HAL_Delay(28000);
        //看棋
        BJMotor_distant(1,8.8);
        BJMotor_distant(2,25);   
        }
        //（1，0）
        if(usart1_RecvBuf[1] == '1' && usart1_RecvBuf[3] == '0')
        {
        //下棋
        BJMotor_distant(1,-11.2);
        BJMotor_distant(2,18.5);
        HAL_Delay(19000);
        //看棋
        BJMotor_distant(1,-8.8);
        BJMotor_distant(2,17); 
        }
        //（1，1）
        if(usart1_RecvBuf[1] == '1' && usart1_RecvBuf[3] == '1')
        {
        //下棋
        BJMotor_distant(1,-20);
        BJMotor_distant(2,18.5);
        HAL_Delay(18000);
        //看棋
        BJMotor_distant(2,17);  
        }
        //（1，2）
        if(usart1_RecvBuf[1] == '1' && usart1_RecvBuf[3] == '2')
        {
        //下棋
        BJMotor_distant(1,-28.8);
        BJMotor_distant(2,18.5);
        HAL_Delay(25000);
        //看棋
        BJMotor_distant(1,8.8);
        BJMotor_distant(2,17);   
        }
        //（2，0）
        if(usart1_RecvBuf[1] == '2' && usart1_RecvBuf[3] == '0')
        {
        //下棋
        BJMotor_distant(1,-11.2);
        BJMotor_distant(2,26.5);
        HAL_Delay(30000);
        //看棋
        BJMotor_distant(1,-8.8);
        BJMotor_distant(2,9);    
        }
        //（2，1）
        if(usart1_RecvBuf[1] == '2' && usart1_RecvBuf[3] == '1')
        {
        //下棋
        BJMotor_distant(1,-20);
        BJMotor_distant(2,26.5);
        HAL_Delay(25000);
        //看棋
        BJMotor_distant(2,9); 
        }
        //（2，2）
        if(usart1_RecvBuf[1] == '2' && usart1_RecvBuf[3] == '2')
        {
        //下棋
        BJMotor_distant(1,-28.8);
        BJMotor_distant(2,26.5);
        HAL_Delay(25000);
        //看棋
        BJMotor_distant(1,8.8);
        BJMotor_distant(2,9);   
        }
    }

}

void QZ(void)
{
    //坐标（0，0）
    if(usart1_RecvBuf[1] == '0' && usart1_RecvBuf[3] == '0')
    {
    BJMotor_distant(1,8.8);
    BJMotor_distant(2,-25);
    HAL_Delay(24000);
    BJMotor_distant(3,-2);
    HAL_Delay(3000);
    BJMotor_distant(3,2);
    BJMotor_distant(1,-8.8);
    BJMotor_distant(2,25);
    }
    //坐标（0，1）
    if(usart1_RecvBuf[1] == '0' && usart1_RecvBuf[3] == '1')
    {
    BJMotor_distant(2,-25);
    HAL_Delay(25000);
    BJMotor_distant(3,4);
    HAL_Delay(3000);
    BJMotor_distant(3,-4);
    BJMotor_distant(2,25);                 
    }
    //坐标（0，2）
    if(usart1_RecvBuf[1] == '0' && usart1_RecvBuf[3] == '2')
    {
    BJMotor_distant(2,-25);
    BJMotor_distant(1,-8.8);
    HAL_Delay(24000);
    BJMotor_distant(3,4);
    HAL_Delay(3000);
    BJMotor_distant(3,-4);
    BJMotor_distant(1,8.8);
    BJMotor_distant(2,25);                 
    }
    //坐标（1，0）
    if(usart1_RecvBuf[1] == '1' && usart1_RecvBuf[3] == '0')
    {
    BJMotor_distant(2,-17);
    BJMotor_distant(1,8.8);
    HAL_Delay(16000);
    BJMotor_distant(3,4);
    HAL_Delay(3000);
    BJMotor_distant(3,-4);
    BJMotor_distant(1,-8.8);
    BJMotor_distant(2,17);                 
    }
    //坐标（1，1）
    if(usart1_RecvBuf[1] == '1' && usart1_RecvBuf[3] == '1')
    {
    BJMotor_distant(2,-17);
    HAL_Delay(16000);
    BJMotor_distant(3,4);
    HAL_Delay(3000);
    BJMotor_distant(3,-4);
    BJMotor_distant(2,17);                  
    }
    //坐标（1，2）
    if(usart1_RecvBuf[1] == '1' && usart1_RecvBuf[3] == '2')
    {
    BJMotor_distant(2,-17);
    BJMotor_distant(1,-8.8);
    HAL_Delay(16000);
    BJMotor_distant(3,4);
    HAL_Delay(3000);
    BJMotor_distant(3,-4);
    BJMotor_distant(1,8.8);
    BJMotor_distant(2,17);                
    }
    //坐标（2，0）
    if(usart1_RecvBuf[1] == '2' && usart1_RecvBuf[3] == '0')
    {
    BJMotor_distant(1,8.8);
    BJMotor_distant(2,-9);
    BJMotor_distant(3,4);
    HAL_Delay(7000);

    HAL_Delay(3000);
    BJMotor_distant(3,-4);
    BJMotor_distant(1,-8.8);
    BJMotor_distant(2,9);

    }
    //坐标（2，1）
    if(usart1_RecvBuf[1] == '2' && usart1_RecvBuf[3] == '1')
    {
    BJMotor_distant(2,-9);
    HAL_Delay(6000);
    BJMotor_distant(3,4);
    HAL_Delay(3000);
    BJMotor_distant(3,-4);
    BJMotor_distant(2,9);                
    }
    //坐标（2，2）
    if(usart1_RecvBuf[1] == '2' && usart1_RecvBuf[3] == '2')
    {
    BJMotor_distant(1,-8.8);
    BJMotor_distant(2,-9);
    BJMotor_distant(3,4);
    HAL_Delay(7000);

    BJMotor_distant(3,-4);
    BJMotor_distant(1,8.8);
    BJMotor_distant(2,9);                
    }       
}



