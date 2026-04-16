  #ifndef _Pid_h_
#define _Pid_h_


typedef struct {  
    float Kp;    //比例系数
    float Ki;   //积分系数
    float Kd;      //微分系数
    float SetPoint;  //目标值
    float ProcessVariable; //过程变量 当前速度
    float ErrorSum;  //误差和
    float LastError;//上一次的误差
    float Output;  //PID输出
} PIDController;
  
// 增量式PID结构体
typedef struct {
    float Kp;           // 比例系数
    float Ki;           // 积分系数
    float Kd;           // 微分系数
    float prev_error;    // 上一次误差 e(k-1)
    float prev2_error;   // 上上次误差 e(k-2)
} IncrementalPID;


void PID_Init(PIDController *pid, float kp, float ki, float kd, float setPoint);
float PID_Calculate(PIDController *pid, float processVariable);
void PID_Init_Moter(IncrementalPID *pid, float Kp, float Ki, float Kd);
float PID_Update_Moter(IncrementalPID *pid, float error);
#endif 