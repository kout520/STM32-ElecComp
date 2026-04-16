#include "headfile.h"
//本次使用4个超声波模块
//使用方法
/*1.开始TIM2全通道输入捕获，中断打开
2.设置配置为72-1(APP)其他可以按需配置
3.设置4个Trig引脚（都为输出引脚）（注意表明标签Trig,Trig1,Trig2,Trig3）
4.定时器1ms调用
SR04_GetData();
if(sr04_flag)
{
//这里面才会获得距离
a = 10 * distant;
b = 10 * distant1;
c = 10 * distant2;
d = 10 * distant3;
// 测量距离并存入缓冲区
printf("@(%d,%d,%d,%d)\r\n",a,b,c1,d);

}
*/

float distant;      //测量距离
float distant1;      //测量距离
float distant2;      //测量距离
float distant3;      //测量距离
uint32_t measure_Buf[3] = {0};   //存放定时器计数值的数组
uint32_t measure_Buf1[3] = {0};   //存放定时器计数值的数组
uint32_t measure_Buf2[3] = {0};   //存放定时器计数值的数组
uint32_t measure_Buf3[3] = {0};   //存放定时器计数值的数组
uint8_t  measure_Cnt = 0;    //状态标志位
uint32_t high_time;   //超声波模块返回的高电平时间
uint32_t high_time1;   //超声波模块返回的高电平时间
uint32_t high_time2;   //超声波模块返回的高电平时间
uint32_t high_time3;   //超声波模块返回的高电平时间
uint8_t sr_flag = 0;
uint8_t sr04_flag;
//===============================================读取距离
void SR04_GetData(void)
{
        sr04_flag = 0;
switch (measure_Cnt){
	case 0:
        sr04_flag = 1;
                switch(sr_flag)
                {
                    case 0:
                        TRIG_H;
                    delay_us(30);
                    TRIG_L;
                    __HAL_TIM_SET_CAPTUREPOLARITY(&htim2, TIM_CHANNEL_1, TIM_INPUTCHANNELPOLARITY_RISING);
                    HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_1);
                        break;
                    case 1:
                        TRIG1_H;
                    delay_us(30);
                    TRIG1_L;
                    __HAL_TIM_SET_CAPTUREPOLARITY(&htim2, TIM_CHANNEL_2, TIM_INPUTCHANNELPOLARITY_RISING);
                    HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_2);
                        break;
                    case 2:
                        TRIG2_H;
                    delay_us(15);
                    TRIG2_L;
                     __HAL_TIM_SET_CAPTUREPOLARITY(&htim2, TIM_CHANNEL_3, TIM_INPUTCHANNELPOLARITY_RISING);
                    HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_3);
                        break;
                    case 3:
                        TRIG3_H;
                    delay_us(15);
                    TRIG3_L;
                    __HAL_TIM_SET_CAPTUREPOLARITY(&htim2, TIM_CHANNEL_4, TIM_INPUTCHANNELPOLARITY_RISING);
                    HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_4);
                        break;
                }
//        TRIG_H;
//        TRIG1_H;
//        TRIG2_H;
        //TRIG3_H;
//         delay_us(15);
//         TRIG_L;
//    TRIG1_L;
//    TRIG2_L;
    //TRIG3_L;
            
		measure_Cnt++;
//		__HAL_TIM_SET_CAPTUREPOLARITY(&htim2, TIM_CHANNEL_1, TIM_INPUTCHANNELPOLARITY_RISING);
//    __HAL_TIM_SET_CAPTUREPOLARITY(&htim2, TIM_CHANNEL_2, TIM_INPUTCHANNELPOLARITY_RISING);
//    __HAL_TIM_SET_CAPTUREPOLARITY(&htim2, TIM_CHANNEL_3, TIM_INPUTCHANNELPOLARITY_RISING);
    //__HAL_TIM_SET_CAPTUREPOLARITY(&htim2, TIM_CHANNEL_4, TIM_INPUTCHANNELPOLARITY_RISING);
    
    
    
//		HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_1);	//启动输入捕获       或者: __HAL_TIM_ENABLE(&htim5);            
//        HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_2);
//    HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_3);
    //HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_4);
        break;
	case 3:
        sr04_flag = 1;
                switch(sr_flag)
                {
                    case 0:
                        high_time = measure_Buf[1]- measure_Buf[0];    //高电平时间
                    distant=(high_time*0.034)/2;  //单位cm 
                        break;
                    case 1:
                        high_time1 = measure_Buf1[1]- measure_Buf1[0];    //高电平时间
                    distant1=(high_time1*0.034)/2;  //单位cm   
                        break;
                    case 2:
                    high_time2 = measure_Buf2[1]- measure_Buf2[0];    //高电平时间
                    distant2=(high_time2*0.034)/2;  //单位cm        
                        break;
                    case 3:
                    high_time3 = measure_Buf3[1]- measure_Buf3[0];    //高电平时间
                    distant3=(high_time3*0.034)/2;  //单位cm    
                        break;
                }
//		high_time = measure_Buf[1]- measure_Buf[0];    //高电平时间
//    high_time1 = measure_Buf1[1]- measure_Buf1[0];    //高电平时间
//    high_time2 = measure_Buf2[1]- measure_Buf2[0];    //高电平时间
    //high_time3 = measure_Buf3[1]- measure_Buf3[0];    //高电平时间
//		distant=(high_time*0.034)/2;  //单位cm        
//    distant1=(high_time1*0.034)/2;  //单位cm        
//    distant2=(high_time2*0.034)/2;  //单位cm        
    //distant3=(high_time3*0.034)/2;  //单位cm        
		measure_Cnt = 0;  //清空标志位
        TIM2->CNT=0;     //清空计时器计数
                sr_flag++;
                sr_flag %= 2;
		break;
				
	}
}
 
 
//===============================================us延时函数
    void delay_us(uint32_t us)//主频72M
{
    uint32_t delay = (HAL_RCC_GetHCLKFreq() / 4000000 * us);
    while (delay--)
	{
		;
	}
}
 
//===============================================中断回调函数
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)//
{
	
	if(TIM2 == htim->Instance)// 判断触发的中断的定时器为TIM2
	{
		switch(measure_Cnt){
			case 1:
                switch(sr_flag)
                {
                    case 0:
                        measure_Buf[0] = HAL_TIM_ReadCapturedValue(&htim2,TIM_CHANNEL_1);//获取当前的捕获值.
                    __HAL_TIM_SET_CAPTUREPOLARITY(&htim2,TIM_CHANNEL_1,TIM_ICPOLARITY_FALLING);  //设置为下降沿捕获
                        break;
                    case 1:
                        measure_Buf1[0] = HAL_TIM_ReadCapturedValue(&htim2,TIM_CHANNEL_2);//获取当前的捕获值.
                    __HAL_TIM_SET_CAPTUREPOLARITY(&htim2,TIM_CHANNEL_2,TIM_ICPOLARITY_FALLING);  //设置为下降沿捕获
                        break;
                    case 2:
                        measure_Buf2[0] = HAL_TIM_ReadCapturedValue(&htim2,TIM_CHANNEL_3);//获取当前的捕获值.
                    __HAL_TIM_SET_CAPTUREPOLARITY(&htim2,TIM_CHANNEL_3,TIM_ICPOLARITY_FALLING);  //设置为下降沿捕获
                        break;
                    case 3:
                        measure_Buf3[0] = HAL_TIM_ReadCapturedValue(&htim2,TIM_CHANNEL_4);//获取当前的捕获值.
                    __HAL_TIM_SET_CAPTUREPOLARITY(&htim2,TIM_CHANNEL_4,TIM_ICPOLARITY_FALLING);  //设置为下降沿捕获
                        break;
                }
//				measure_Buf[0] = HAL_TIM_ReadCapturedValue(&htim2,TIM_CHANNEL_1);//获取当前的捕获值.
//            measure_Buf1[0] = HAL_TIM_ReadCapturedValue(&htim2,TIM_CHANNEL_2);//获取当前的捕获值.
//            measure_Buf2[0] = HAL_TIM_ReadCapturedValue(&htim2,TIM_CHANNEL_3);//获取当前的捕获值.
            //measure_Buf3[0] = HAL_TIM_ReadCapturedValue(&htim2,TIM_CHANNEL_4);//获取当前的捕获值.
//				__HAL_TIM_SET_CAPTUREPOLARITY(&htim2,TIM_CHANNEL_1,TIM_ICPOLARITY_FALLING);  //设置为下降沿捕获
//            __HAL_TIM_SET_CAPTUREPOLARITY(&htim2,TIM_CHANNEL_2,TIM_ICPOLARITY_FALLING);  //设置为下降沿捕获
//            __HAL_TIM_SET_CAPTUREPOLARITY(&htim2,TIM_CHANNEL_3,TIM_ICPOLARITY_FALLING);  //设置为下降沿捕获
            //__HAL_TIM_SET_CAPTUREPOLARITY(&htim2,TIM_CHANNEL_4,TIM_ICPOLARITY_FALLING);  //设置为下降沿捕获
				measure_Cnt++;                                            
				break;              
			case 2:
                switch(sr_flag)
                {
                    case 0:
                        measure_Buf[1] = HAL_TIM_ReadCapturedValue(&htim2,TIM_CHANNEL_1);//获取当前的捕获值.
                        HAL_TIM_IC_Stop_IT(&htim2,TIM_CHANNEL_1); //停止捕获   或者: __HAL_TIM_DISABLE(&htim5);
                        break;
                    case 1:
                        measure_Buf1[1] = HAL_TIM_ReadCapturedValue(&htim2,TIM_CHANNEL_2);//获取当前的捕获值.
                    HAL_TIM_IC_Stop_IT(&htim2,TIM_CHANNEL_2); //停止捕获   或者: __HAL_TIM_DISABLE(&htim5);
                        break;
                    case 2:
                        measure_Buf2[1] = HAL_TIM_ReadCapturedValue(&htim2,TIM_CHANNEL_3);//获取当前的捕获值.
                    HAL_TIM_IC_Stop_IT(&htim2,TIM_CHANNEL_3); //停止捕获   或者: __HAL_TIM_DISABLE(&htim5);
                        break;
                    case 3:
                        measure_Buf3[1] = HAL_TIM_ReadCapturedValue(&htim2,TIM_CHANNEL_4);//获取当前的捕获值.
                    HAL_TIM_IC_Stop_IT(&htim2,TIM_CHANNEL_4); //停止捕获   或者: __HAL_TIM_DISABLE(&htim5);
                        break;
                }
//				measure_Buf[1] = HAL_TIM_ReadCapturedValue(&htim2,TIM_CHANNEL_1);//获取当前的捕获值.
//            measure_Buf1[1] = HAL_TIM_ReadCapturedValue(&htim2,TIM_CHANNEL_2);//获取当前的捕获值.
//            measure_Buf2[1] = HAL_TIM_ReadCapturedValue(&htim2,TIM_CHANNEL_3);//获取当前的捕获值.
            //measure_Buf3[1] = HAL_TIM_ReadCapturedValue(&htim2,TIM_CHANNEL_4);//获取当前的捕获值.
//				HAL_TIM_IC_Stop_IT(&htim2,TIM_CHANNEL_1); //停止捕获   或者: __HAL_TIM_DISABLE(&htim5);
//            HAL_TIM_IC_Stop_IT(&htim2,TIM_CHANNEL_2); //停止捕获   或者: __HAL_TIM_DISABLE(&htim5);
//            HAL_TIM_IC_Stop_IT(&htim2,TIM_CHANNEL_3); //停止捕获   或者: __HAL_TIM_DISABLE(&htim5);
            //HAL_TIM_IC_Stop_IT(&htim2,TIM_CHANNEL_4); //停止捕获   或者: __HAL_TIM_DISABLE(&htim5);
				measure_Cnt++;  
                         
		}
	
	}
	
}
 



 


