//使用方式
/*
1.调用初始中断到主函数，本次使用TIM4CH4，输入捕获，PSC =  72-1，ARR = 10000 - 1，滤波4，使能中断
HAL_TIM_IC_Start_IT(&htim4, TIM_CHANNEL_4);
HAL_TIM_Base_Start_IT(&htim4);
2.直接调用（尽量在10-20ms里间隔调用，a的值根据实测改变）
remote_app();
*/

#include "headfile.h"

uint8_t a;//防止多触限制
uint8_t key;
uint8_t *str=0;//映射按键

//遥控器接收状态
//[7]:收到了引导码标志
//[6]:得到了一个按键的所有信息
//[5]:保留	
//[4]:标记上升沿是否已经被捕获								   
//[3:0]:溢出计时器
uint8_t 	RmtSta=0;	  	  
uint16_t Dval;		//下降沿时计数器的值
uint32_t RmtRec=0;	//红外接收到的数据	   		    
uint8_t  RmtCnt=0;	//按键按下的次数	 

//定时器更新（溢出）中断回调函数
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if(htim->Instance==TIM4)
	{
 		if(RmtSta&0x80)//上次有数据被接收到了
		{	
			RmtSta&=~0X10;						//取消上升沿已经被捕获标记
			if((RmtSta&0X0F)==0X00)RmtSta|=1<<6;//标记已经完成一次按键的键值信息采集
			if((RmtSta&0X0F)<14)RmtSta++;
			else
			{
				RmtSta&=~(1<<7);//清空引导标识
				RmtSta&=0XF0;	//清空计数器	
			}						 	   	
		}	
	}
}

//定时器输入捕获中断回调函数
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)//捕获中断发生时执行
{
	if(htim->Instance==TIM4)
	{
		if(RDATA)//上升沿捕获
		{
			TIM_RESET_CAPTUREPOLARITY(&htim4,TIM_CHANNEL_4);   					//一定要先清除原来的设置！！
			TIM_SET_CAPTUREPOLARITY(&htim4,TIM_CHANNEL_4,TIM_ICPOLARITY_FALLING);//CC4P=1 设置为下降沿捕获
			__HAL_TIM_SET_COUNTER(&htim4,0);  	//清空定时器值   	  
		  	RmtSta|=0X10;							//标记上升沿已经被捕获
		}else //下降沿捕获
		{
			Dval=HAL_TIM_ReadCapturedValue(&htim4,TIM_CHANNEL_4);//读取CCR4也可以清CC4IF标志位
			TIM_RESET_CAPTUREPOLARITY(&htim4,TIM_CHANNEL_4);   //一定要先清除原来的设置！！
			TIM_SET_CAPTUREPOLARITY(&htim4,TIM_CHANNEL_4,TIM_ICPOLARITY_RISING);//配置TIM4通道4上升沿捕获
			if(RmtSta&0X10)							//完成一次高电平捕获 
			{
 				if(RmtSta&0X80)//接收到了引导码
				{
					
					if(Dval>300&&Dval<800)			//560为标准值,560us
					{
						RmtRec<<=1;					//左移一位.
						RmtRec|=0;					//接收到0	   
					}else if(Dval>1400&&Dval<1800)	//1680为标准值,1680us
					{
						RmtRec<<=1;					//左移一位.
						RmtRec|=1;					//接收到1
					}else if(Dval>2200&&Dval<2600)	//得到按键键值增加的信息 2500为标准值2.5ms
					{
						RmtCnt++; 					//按键次数增加1次
						RmtSta&=0XF0;				//清空计时器		
					}
 				}else if(Dval>4200&&Dval<4700)		//4500为标准值4.5ms
				{
					RmtSta|=1<<7;					//标记成功接收到了引导码
					RmtCnt=0;						//清除按键次数计数器
				}						 
			}
			RmtSta&=~(1<<4);
		}
	}
}

//处理红外键盘
//返回值:
//	 0,没有任何按键按下
//其他,按下的按键键值.
uint8_t Remote_Scan(void)
{        
	uint8_t sta=0;       
	uint8_t t1,t2;  
	if(RmtSta&(1<<6))//得到一个按键的所有信息了
	{ 
	    t1=RmtRec>>24;			//得到地址码
	    t2=(RmtRec>>16)&0xff;	//得到地址反码 
 	    if((t1==(uint8_t)~t2)&&t1==REMOTE_ID)//检验遥控识别码(ID)及地址 
	    { 
	        t1=RmtRec>>8;
	        t2=RmtRec; 	
	        if(t1==(uint8_t)~t2)sta=t1;//键值正确	 
		}   
		if((sta==0)||((RmtSta&0X80)==0))//按键数据错误/遥控已经没有按下了
		{
		 	RmtSta&=~(1<<6);//清除接收到有效按键标识
			RmtCnt=0;		//清除按键次数计数器
		}
	}  
    return sta;
}

//应用函数
void remote_app(void)//尽量调用间隔10-20ms
{
		key=Remote_Scan();	
		if(key)
		{	 
            
			switch(key)
			{
				case 162:str="1";
                    a++;
                    if(a>16)
                    {
                    HAL_GPIO_TogglePin(GPIOG, GPIO_PIN_3);
                        a = 0;
                    }
                    
                break;	    
				case 98:
                    str="2";
                    a++;
                    if(a>16)
                    {
                    HAL_GPIO_TogglePin(GPIOE, GPIO_PIN_5);
                        a = 0;
                    }
                    
                break;	    
				case 2: 
                    str="5";
                    a++;
                    if(a>16)
                    {
                    HAL_GPIO_TogglePin(GPIOE, GPIO_PIN_5);
                        a = 0;
                    }
                    
                break;		 
				case 226:
                    str="3";
                a++;
                    if(a>16)
                    {
                    HAL_GPIO_TogglePin(GPIOE, GPIO_PIN_5);
                        a = 0;
                    }
                break;		  
				case 194:
                    str="6";
                a++;
                    if(a>16)
                    {
                    HAL_GPIO_TogglePin(GPIOE, GPIO_PIN_5);
                        a = 0;
                    }
                break;	   
				case 34:
                    str="4";
                a++;
                    if(a>16)
                    {
                    HAL_GPIO_TogglePin(GPIOE, GPIO_PIN_5);
                        a = 0;
                    }
                break;		  
				case 224:
                    str="7";
                a++;
                    if(a>16)
                    {
                    HAL_GPIO_TogglePin(GPIOE, GPIO_PIN_5);
                        a = 0;
                    }
                break;		  
				case 168:
                    str="8";
                a++;
                    if(a>16)
                    {
                    HAL_GPIO_TogglePin(GPIOE, GPIO_PIN_5);
                        a = 0;
                    }
                break;		   
				case 144:
                    str="9";
                a++;
                    if(a>16)
                    {
                    HAL_GPIO_TogglePin(GPIOE, GPIO_PIN_5);
                        a = 0;
                    }
                break;		    
				case 104:
                    str="*";
                a++;
                    if(a>16)
                    {
                    HAL_GPIO_TogglePin(GPIOE, GPIO_PIN_5);
                        a = 0;
                    }
                break;		  
				case 152:
                    str="0";
                a++;
                    if(a>16)
                    {
                    HAL_GPIO_TogglePin(GPIOE, GPIO_PIN_5);
                        a = 0;
                    }
                break;	   
				case 176:
                    str="#";
                a++;
                    if(a>16)
                    {
                    HAL_GPIO_TogglePin(GPIOE, GPIO_PIN_5);
                        a = 0;
                    }
                break;	    
				case 24:
                    str="up";
                a++;
                    if(a>16)
                    {
                    HAL_GPIO_TogglePin(GPIOE, GPIO_PIN_5);
                        a = 0;
                    }
                break;		    
				case 122:
                    str="6";
                a++;
                    if(a>16)
                    {
                    HAL_GPIO_TogglePin(GPIOE, GPIO_PIN_5);
                        a = 0;
                    }
                break;		  
				case 16:
                    str="left";
                a++;
                    if(a>16)
                    {
                    HAL_GPIO_TogglePin(GPIOE, GPIO_PIN_5);
                        a = 0;
                    }
                break;			   					
				case 56:
                    str="ok";
                a++;
                    if(a>16)
                    {
                    HAL_GPIO_TogglePin(GPIOE, GPIO_PIN_5);
                        a = 0;
                    }
                break;	 
				case 90:
                    str="right";
                a++;
                    if(a>16)
                    {
                    HAL_GPIO_TogglePin(GPIOE, GPIO_PIN_5);
                        a = 0;
                    }
                break;
				case 74:
                    str="down";
                a++;
                    if(a>16)
                    {
                    HAL_GPIO_TogglePin(GPIOE, GPIO_PIN_5);
                        a = 0;
                    }
                break;
			}
        }

}





