//使用说明
//通过情况改变.h文件定义即可
//

#include "headfile.h"
 void Change_speed(uint16_t speed)
{/* 变速 */
    // 设置电机速度范围（60-100），越大越快

}
void Sensor(uint16_t value,uint16_t pos)
{//value是小车正常行驶时候的速度，POS是转弯时候的速度
    	if(L1==0&&L2==0&&M0==1&&R1==0&&R2==0)//如果4个全部没有读取黑线，直走;   Forward();
		{

		}
		
		if(L1==0&&L2==1&&R1==1&&R2==0)//如果2，3读取黑线，直走;   Forward();
		{

		}
		
		if(L1==0&&L2==1&&R1==0&&R2==0)//如果2读取黑线，左转;   Turn_Left();
		{

		}
		
		if(L1==0&&L2==0&&R1==1&&R2==0)//如果3读取黑线，右转;   Turn_Right();
		{

		}
		      //大左转
     if(L1==1&&(L2==1)&&(R1==0)&&R2==0)
     {
         HAL_Delay(20);//等待响应
         while(1)
         {
             //等待出弯

             HAL_Delay(40);//等待响应
            if(L1==0&&(L2==0)&&(R1==1)&&R2==0)
            break;                
         } 
     }
     //大右转
     if(L1==0&&(L2==0)&&(R1==1)&&R2==1)
     {
         HAL_Delay(20);//等待响应
         while(1)
         {
             //等待出弯

             HAL_Delay(40);//等待响应
            if(L1==0&&(L2==1)&&(R1==0)&&R2==0)
            break;                
         }
     }

}

