#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "led.h"
#include "key.h"
#include "lcd.h"
#include "remote.h"
/************************************************
 ALIENTEK 精英STM32F103开发板 实验27
 红外遥控器实验-HAL库函数版
 技术支持：www.openedv.com
 淘宝店铺： http://eboard.taobao.com 
 关注微信公众平台微信号："正点原子"，免费获取STM32资料。
 广州市星翼电子科技有限公司  
 作者：正点原子 @ALIENTEK
************************************************/
	
int main(void)
{	
    u16 a=0;
	u8 key;
	u8 t=0;	
	u8 *str=0;
	
    HAL_Init();                    	 	//初始化HAL库    
    Stm32_Clock_Init(RCC_PLL_MUL9);   	//设置时钟,72M
	delay_init(72);               		//初始化延时函数
	uart_init(115200);					//初始化串口	
	LED_Init();							//初始化LED	
	KEY_Init();							//初始化按键
 	LCD_Init();           				//初始化LCD FSMC接口
    
	Remote_Init();				    	//初始化	红外接收	
  	POINT_COLOR=RED;
	LCD_ShowString(30,50,200,16,16,"ELITE STM32");	
	LCD_ShowString(30,70,200,16,16,"REMOTE TEST");	
	LCD_ShowString(30,90,200,16,16,"ATOM@ALIENTEK");
	LCD_ShowString(30,110,200,16,16,"2019/9/19");
   	LCD_ShowString(30,130,200,16,16,"KEYVAL:");	
   	LCD_ShowString(30,150,200,16,16,"KEYCNT:");	
   	LCD_ShowString(30,170,200,16,16,"SYMBOL:");	     
    
	POINT_COLOR=BLUE;//设置字体为蓝色  														 	  		    							  
	while(1)
	{
		key=Remote_Scan();	
		if(key)
		{	 
            LCD_ShowNum(86,190,a,3,16);		//显示键值
			LCD_ShowNum(86,130,key,3,16);		//显示键值
			LCD_ShowNum(86,150,RmtCnt,3,16);	//显示按键次数		  
			switch(key)
			{
				case 0:str="ERROR";break;			   
				case 162:str="1";break;	    
				case 98:str="2";break;	    
				case 2:
                    t++;
                    str="5";
                if(t > 20)
                {
                    a ++;
                    t = 0;
                }
                
                break;		 
				case 226:str="3";break;		  
				case 194:str="6";break;	   
				case 34:str="4";break;		  
				case 224:str="7";break;		  
				case 168:str="8";break;		   
				case 144:str="9";break;		    
				case 104:str="*";break;		  
				case 152:str="0";break;	   
				case 176:str="#";break;	    
				case 48:str="4";break;		    
				case 24:str="up";break;		    
				case 122:str="6";break;		  
				case 16:str="left";break;			   					
				case 56:str="ok";break;	 
				case 90:str="right";break;
				case 74:str="down";break;
				case 82:str="DELETE";break;		 
			}
				LCD_Fill(86,170,116+8*8,170+16,WHITE);	//清楚之前的显示
				LCD_ShowString(86,170,200,16,16,str);	//显示SYMBOL
		}
        else delay_ms(20);

	}


}

