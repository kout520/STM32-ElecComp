#ifndef __PID_H__
#define __PID_H__

#define INTEGRAL_LIMIT 100.0f  // 积分项限幅值，防止积分饱和
// PID 控制器结构体

typedef struct {
    float Kp;
    float Ki;
    float Kd;
    float SetPoint;
    float ProcessVariable;
    float LastError;
    float LastLastError;  // 新增
    float Output;
    // float ErrorSum; // 增量式 PID 可不需要积分累加
} PIDController;

// 函数声明
void PID_Incremental_Init(PIDController *pid, float kp, float ki, float kd, float setPoint);
float PID_Incremental_Calculate(PIDController *pid, float processVariable, float dt);
void PID_Incremental_Reset(PIDController *pid);
void PID_SetTarget(PIDController *pid, float setPoint);






#endif

