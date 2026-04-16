#include "headfile.h"


#define OUTPUT_LIMIT 200.0f  // 输出限幅示例，根据实际需求调整

/**
 * @brief 初始化增量式 PID 控制器
 * @param pid PID 控制器指针
 * @param kp 比例系数
 * @param ki 积分系数
 * @param kd 微分系数
 * @param setPoint 目标值
 */
void PID_Incremental_Init(PIDController *pid, float kp, float ki, float kd, float setPoint) {
    pid->Kp = kp;
    pid->Ki = ki;
    pid->Kd = kd;
    pid->SetPoint = setPoint;
    pid->ProcessVariable = 0.0f;
    pid->LastError = 0.0f;
    pid->LastLastError = 0.0f;
    pid->Output = 0.0f;
}

/**
 * @brief 计算增量式 PID 输出增量
 * @param pid PID 控制器指针
 * @param processVariable 当前过程变量
 * @return 当前输出值（累加后的控制量）
 */
// 增量式 PID 计算（修正版）
float PID_Incremental_Calculate(PIDController *pid, float processVariable, float dt) {
    float error = pid->SetPoint - processVariable;
    
    // 修正增量式 PID 公式
    float deltaOutput = pid->Kp * (error - pid->LastError)
                      + pid->Ki * error * dt  // 修正积分项
                      + pid->Kd * (error - 2.0f * pid->LastError + pid->LastLastError) / dt;

    // 累加输出并限幅
    pid->Output += deltaOutput;
    if (pid->Output > OUTPUT_LIMIT) pid->Output = OUTPUT_LIMIT;
    else if (pid->Output < -OUTPUT_LIMIT) pid->Output = -OUTPUT_LIMIT;

    // 更新误差历史
    pid->LastLastError = pid->LastError;
    pid->LastError = error;
    pid->ProcessVariable = processVariable;

    return pid->Output;
}


/**
 * @brief 重置增量式 PID 控制器状态
 * @param pid PID 控制器指针
 */
void PID_Incremental_Reset(PIDController *pid) {
    pid->LastError = 0.0f;
    pid->LastLastError = 0.0f;
    pid->Output = 0.0f;
    pid->ProcessVariable = 0.0f;
}

/**
 * @brief 设置 PID 控制器的目标值
 * @param pid PID 控制器指针
 * @param setPoint 新目标值
 */
void PID_SetTarget(PIDController *pid, float setPoint) {
    
    pid->SetPoint = setPoint;
}

