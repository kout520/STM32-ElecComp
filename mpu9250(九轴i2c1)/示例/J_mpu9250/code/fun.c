#include "headfile.h"

uint8_t init_flag=0,rec_count=0;

void mpu9250_Start(void)
{
	
	float pitch,roll,yaw; 	        //欧拉角
	short aacx,aacy,aacz;	        //加速度传感器原始数据
	short gyrox,gyroy,gyroz;        //陀螺仪原始数据 
    short temp;                     //温度
    
    uint8_t err = mpu_dmp_init();
    while(err)         
	{   
        printf("mpu_init_err:%d\r\n",err);
		HAL_Delay(200);
        err = mpu_dmp_init();
	}
	init_flag=1;
    printf("mpu9250 Ok\r\n");
//    while(1)
//    {
//        err = mpu_mpl_get_data(&pitch,&roll,&yaw);
//        if(err == 0)
//        {
//                temp = MPU_Get_Temperature();	                                     //得到温度值（扩大了100倍）
//                MPU_Get_Accelerometer(&aacx,&aacy,&aacz);	                         //得到加速度传感器数据
//                MPU_Get_Gyroscope(&gyrox,&gyroy,&gyroz);	                         //得到陀螺仪数据
//                //mpu6050_send_data(aacx,aacy,aacz,gyrox,gyroy,gyroz);               //发送加速度+陀螺仪原始数据
//                printf(/*"roll = %f   pitch = %f */  "yaw = %f (T = %d)\r\n",/*roll,pitch,*/yaw,temp);
//					
//                
//        }
//		HAL_Delay(50);
//    }
 }

 
void mpu9250go(void)
{
    	float pitch,roll,yaw; 	        //欧拉角
	short aacx,aacy,aacz;	        //加速度传感器原始数据
	short gyrox,gyroy,gyroz;        //陀螺仪原始数据 
    short temp;                     //温度
				 uint8_t err=0;
	if(init_flag){				
		err = mpu_mpl_get_data(&pitch,&roll,&yaw);
        if(err == 0)
        {
                temp = MPU_Get_Temperature();	                                     //得到温度值（扩大了100倍）
                MPU_Get_Accelerometer(&aacx,&aacy,&aacz);	                         //得到加速度传感器数据
                MPU_Get_Gyroscope(&gyrox,&gyroy,&gyroz);	                         //得到陀螺仪数据
                //mpu6050_send_data(aacx,aacy,aacz,gyrox,gyroy,gyroz);               //发送加速度+陀螺仪原始数据
                printf(/*"roll = %f   pitch = %f */  "yaw = %f (T = %d)\r\n",/*roll,pitch,*/yaw,temp);
					
                
        }
			}
}
