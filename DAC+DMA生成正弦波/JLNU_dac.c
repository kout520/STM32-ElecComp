#include "headfile.h"

/*
宏定义 PI：π的近似值

宏定义 TIME_INTERVAL_US：两次转换之间的时间间隔，单位是us

结构体 SinWavePar：正弦波的结构体，用于储存采样点数量，数组地址

linearmapping：线性映射，用于将时基映射到0~2π，其本质是求坐标系中过 (as,bs),(ae,be)两个点的直线解析式，然后将val代入求值

TIM_Config：定时器参数的配置，单独写出来是为了方便后续修改正弦波参数
使用情况：在主函数只需调用一次：
  SinWavePar Par = GenerateSinWave(1000);
  TIM_Config();
  HAL_TIM_Base_Start_IT(&htim2);
  HAL_DAC_Start_DMA(&hdac,DAC1_CHANNEL_1,(uint32_t*)Par.DACbuf,Par.length,DAC_ALIGN_12B_R);
*/

#define PI      3.14159265358979
 
#define TIME_INTERVAL_US      10
#define TIM_FREQUENCY_MHZ     72
 

double linearmapping(double as,double ae,double bs,double be,double val){
  return(val*(be-bs)/(ae-as)+bs-as*(be-bs)/(ae-as));
}
 
void TIM_Config(void){
  __HAL_TIM_SET_PRESCALER(&htim2,0);
  __HAL_TIM_SET_AUTORELOAD(&htim2,(uint32_t)((TIME_INTERVAL_US)*(TIM_FREQUENCY_MHZ)-1));
}
 
SinWavePar GenerateSinWave(int f){
  SinWavePar Par;
  Par.length = 1000000/(TIME_INTERVAL_US)/f;
  Par.DACbuf = malloc(sizeof(uint16_t)*Par.length);
  for(int i=0;i<Par.length;i++){
    Par.DACbuf[i]=(uint16_t)linearmapping(-1,1,0,0xFFF,sin(linearmapping(0,Par.length,0,2*PI,i)));
  }
  return Par;
}
 
