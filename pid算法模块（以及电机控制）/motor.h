#ifndef __MOTOR_H__
#define __MOTOR_H__




// º¯ÊýÉùÃ÷
void Motor_Control_Init(void);
void Motor_Control_Update(void);
void Set_Motor_Direction(int8_t left_dir);
void Turn_Right(void);
void Turn_Left(void);
void Rotate_180(void);
void Stop_Motors(void);
void Move_Forward(void);
void Differential_Control(float error);
void distance_unite_1(float TARGET_DISTANCE, int numder);
void Send_Data_To_SerialPlot(float speed_L, float target_speed_L, float output_L) ;


#endif


