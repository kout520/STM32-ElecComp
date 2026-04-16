#include "headfile.h"

int8_t oled_flag=0;
#define RX_BUF_SIZE 32  // 缓冲区大小（根据需求调整）

char rx_buf[32];  // 接收缓冲区
uint8_t rx_index = 0;      // 缓冲区索引
uint8_t coordinate_ready = 0;  // 坐标接收完成标志
int x = 0, y = 0;          // 解析后的坐标

uint16_t a,b,c,d;
char text1[20];
uint8_t lcd_flag= 0;   
uint8_t distant_waring = 30;
void OLEDINIT(void)
{
    OLED_Clear();
    // 在 OLED 上显示“程序”
    //OLED_ShowStr(0,0,"->",2);
    
    OLED_ShowStr(0,0,"front:",1);

    //OLED_ShowStr(0,0,"  ",2);

    OLED_ShowStr(0,1,"left:",1);


    OLED_ShowStr(0,2,"right:",1);
 
    OLED_ShowStr(0,3,"beind:",1);
    OLED_ShowStr(0,5,"point:",1);
    OLED_ShowStr(0,6,"angle:",1);

    
}
void show(void)
{
    switch(lcd_flag)
    {
        case 0:
            OLED_show();
            break;
        case 1:
            break;
        
    }
}

void OLED_show(void)
{
    sprintf(text1,"%.1f   ",distant);
    OLED_ShowStr(64,0,(uint8_t *)text1,1);    
        sprintf(text1,"%.1f   ",distant1);
    OLED_ShowStr(64,1,(uint8_t *)text1,1);    
        sprintf(text1,"%.1f   ",distant2);
    OLED_ShowStr(64,2,(uint8_t *)text1,1);    
        sprintf(text1,"%.1f   ",distant3);
    OLED_ShowStr(64,3,(uint8_t *)text1,1);    
        sprintf(text1,"(%d,%d)   ",x ,y);
    OLED_ShowStr(64,5,(uint8_t *)text1,1);    
}




MotorState_t motor_state = MOTOR_MOVE;

uint8_t bufferIndex = 0;
void motor_scan_non_blocking(void) {
    switch (motor_state) {
        case MOTOR_MOVE:

            motor_state = MOTOR_WAIT;
            break;

        case MOTOR_WAIT:
                motor_state = MOTOR_MEASURE;

            break;

        case MOTOR_MEASURE:
            SR04_GetData();
            if(sr04_flag)
            {
                a = 10 * distant;
            b = 10 * distant1;
            c = 10 * distant2;
            d = 10 * distant3;
                // 测量距离并存入缓冲区
//                printf("@(%d,%d,%d,%d)\r\n",a,b,c,d);
                motor_state = MOTOR_MOVE;
            }
            break;
    }
}


// 串口接收完成（收到2个字节）中断回调函数
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
    if (huart->Instance == USART1) {
        char received_char = rx_buf[rx_index];  // 获取当前接收的字符

        // 检查是否收到完整坐标（以 '\n' 结尾）
        if (received_char == '\n') {
            rx_buf[rx_index] = '\0';  // 字符串终止符
            coordinate_ready = 1;      // 标记坐标可解析
            rx_index = 0;              // 重置索引

        } 
        else {
            rx_index++;
            if (rx_index >= RX_BUF_SIZE) {
                rx_index = 0;  // 防止缓冲区溢出
            }
        }
        
        // 重新启动接收（单字节模式）
        HAL_UART_Receive_IT(&huart1, (uint8_t *)&rx_buf[rx_index], 1);
    }
}
void ParseCoordinate(void) {
    if (!coordinate_ready) return;  // 未收到完整数据

    // 检查格式是否合法（以 '@' 开头，包含 ','）
    if (rx_buf[0] == '@' && strchr(rx_buf, ',') != NULL) {
        // 提取 x 和 y
        sscanf(rx_buf, "@%d,%d", &x, &y);

    }

    coordinate_ready = 0;  // 清除标志
}

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
                printf(/*"roll = %f   pitch = %f */  "@(%.2f)\r\n",/*roll,pitch,*/yaw);
            
        }
			}
}
