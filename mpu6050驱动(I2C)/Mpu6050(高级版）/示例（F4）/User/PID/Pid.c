#include "Pid.h"

void PID_Init(PIDController *pid, float kp, float ki, float kd, float setPoint)
	{  
    pid->Kp = kp;             // 设置比例系数  
    pid->Ki = ki;             // 设置积分系数  
    pid->Kd = kd;             // 设置微分系数  
    pid->SetPoint = setPoint; // 设置目标值  
    pid->ProcessVariable = 0.0f; // 初始化过程变量  
    pid->ErrorSum = 0.0f;     // 初始化误差累积和  
    pid->LastError = 0.0f;     // 初始化上一次误差  
    pid->Output = 0.0f;       // 初始化输出值  
} 
float PID_Calculate(PIDController *pid, float processVariable)//现在的速度
{  
float error ;
	error= pid->SetPoint - processVariable; // 计算当前误差  
    pid->ErrorSum += error;                       // 更新误差累积和  
    pid->Output = (pid->Kp*error )+ (pid->Ki*pid->ErrorSum) + (pid->Kd*(error-pid->LastError));
    pid->LastError = error;                     // 更新上一次误差  
    pid->ProcessVariable = processVariable;     // 更新过程变量   
return pid->Output;	
}


// 初始化PID控制器
void PID_Init_Moter(IncrementalPID *pid, float Kp, float Ki, float Kd) {
    pid->Kp = Kp;
    pid->Ki = Ki;
    pid->Kd = Kd;
    pid->prev_error = 0.0f;
    pid->prev2_error = 0.0f;
}

// 计算PID输出增量（需定期调用）
float PID_Update_Moter(IncrementalPID *pid, float error) 
	{
    // 计算控制增量
    float delta = 
        pid->Kp * (error - pid->prev_error) +          // 比例项
        pid->Ki * error +                               // 积分项
        pid->Kd * (error - 2*pid->prev_error + pid->prev2_error); // 微分项

    // 更新误差记录
    pid->prev2_error = pid->prev_error;
    pid->prev_error = error;

    return delta;
}
