/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    motor_ctrl.h
  * @brief   电机伺服控制库：编码器 + 3 种电机驱动 + 正弦 S 曲线运动控制
  * @note    适配 3 种电机驱动，用 MOTOR_DRIVER_TYPE 切换：
  *             1 = IN1/IN2 双 PWM（CP2139C 正反转/刹车模式）
  *             2 = TB6612（1 路 PWM + IN1/IN2 两个方向 GPIO）
  *             3 = YB2419（PWM 调压）+ RZ7899（IN1/IN2 两个方向 GPIO）
  *          用法：
  *             1. 在下面「用户配置区」选驱动类型、填引脚、填一圈计数；
  *             2. CubeMX 里配好编码器定时器（Encoder 模式）和 PWM 定时器；
  *             3. Servo_Init() 初始化；
  *             4. 1ms 定时中断里调 Servo_Tick1ms()；
  *             5. Servo_MoveTo(目标计数, 时间ms) 启动一次 S 曲线运动。
  ******************************************************************************
  */
/* USER CODE END Header */
#ifndef __MOTOR_CTRL_H
#define __MOTOR_CTRL_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include "tim.h"

/* ====================================================================== */
/*                        用户配置区（只改这里）                          */
/* ====================================================================== */

/* 电机驱动类型：1 / 2 / 3 */
#define MOTOR_DRIVER_TYPE   1

/* 编码器：电机转一圈（360°）的总计数（含 4 倍频 × 减速比） */
#define ENCODER_CPR         4400
/* 编码器定时器句柄（CubeMX 里配成 Encoder 模式，句柄名填 htim 那个） */
#define ENCODER_TIM         htim2

/* 电机方向符号：+1 = FORWARD 使编码器计数增加；-1 = FORWARD 使计数减小。
 * 若电机转向反了，改这里的正负号即可（不用动线）。 */
#define MOTION_DIR_SIGN     (-1)

/* 电机 PWM 频率：ARR = 定时器时钟 / 频率 - 1（例 72MHz/20kHz → 3599） */
#define MOTOR_PWM_PERIOD    3599

#if MOTOR_DRIVER_TYPE == 1
/* ---- 类型1：IN1/IN2 双 PWM（CP2139C Forward/Reverse/Brake 模式）---- */
/* 两个 PWM 通道必须在同一个定时器上 */
#define M1_TIM              htim3
#define M1_CH_IN1           TIM_CHANNEL_1
#define M1_CH_IN2           TIM_CHANNEL_2

#elif MOTOR_DRIVER_TYPE == 2
/* ---- 类型2：TB6612 = PWM 调速 + IN1/IN2 方向 ---- */
#define M2_TIM              htim3
#define M2_CH_PWM           TIM_CHANNEL_1
#define M2_IN1_PORT         GPIOA
#define M2_IN1_PIN          GPIO_PIN_6
#define M2_IN2_PORT         GPIOA
#define M2_IN2_PIN          GPIO_PIN_7

#elif MOTOR_DRIVER_TYPE == 3
/* ---- 类型3：YB2419（PWM 调压）+ RZ7899（IN1/IN2 方向）---- */
#define M3_TIM              htim1
#define M3_CH_PWM           TIM_CHANNEL_1
#define M3_IN1_PORT         GPIOA
#define M3_IN1_PIN          GPIO_PIN_5
#define M3_IN2_PORT         GPIOB
#define M3_IN2_PIN          GPIO_PIN_4

#else
#error "MOTOR_DRIVER_TYPE 必须是 1 / 2 / 3"
#endif

/* ====================================================================== */
/*                     运动控制参数（可调）                               */
/* ====================================================================== */

/* S 曲线分段数（10 = 默认 5 加速 + 5 减速） */
#define MOTION_SEGMENTS         10
/* ★ 加速/减速比例：加速段数（1~9），减速段 = 10 - 这个。
 *   5 = 对称；3 = 早加速长减速；7 = 长加速短减速。 */
#define MOTION_ACCEL_SEGMENTS   5

/* 位置环 P 增益：duty = KP × 位置误差（越大越紧，太大会震荡） */
#define MOTION_KP               10
/* 速度阻尼 D：duty -= KD × 实际速度(计数/ms)，抑制过冲震荡 */
#define MOTION_KD               50
/* 库仑摩擦补偿：运动方向上的固定占空比（低速粘滑时调大） */
#define MOTION_FF_FRICTION      400
/* 起转踢脚：起步前几 ms 拉高占空比突破静摩擦 */
#define MOTION_KICK_DUTY        720
#define MOTION_KICK_MS          8
/* 速度前馈：峰值占空比 = KV × |位移| / 时间(ms)，封顶 PEAK_DUTY */
#define MOTION_KV               1150
#define MOTION_PEAK_DUTY        3599
/* 换向滞回（编码器计数）：反向误差超过该值才换向 */
#define MOTION_DIR_HYST         6
/* 到位死区（计数） + 堵转判定（ms）：编码器不动这么久判定完成 */
#define MOTION_DEADBAND         4
#define MOTION_STALL_MS         60
/* 重力前馈（旋转臂吊重物用）：AMP=0 关闭；PHASE=臂竖直向下时的自然角(0~359) */
#define MOTION_GRAV_AMP         0
#define MOTION_GRAV_PHASE       0

/* ====================================================================== */
/*                           公共 API                                     */
/* ====================================================================== */

void     Servo_Init(void);                                /* 初始化编码器 + 电机驱动 */
void     Servo_Tick1ms(void);                             /* 1ms 定时中断里调用 */
void     Servo_MoveTo(int32_t target, uint32_t time_ms);  /* 目标计数 + 时间，S 曲线到位 */
void     Servo_Stop(void);                                /* 停止（短刹车保持位置） */
void     Servo_Coast(void);                               /* 停止（滑行释放） */
uint8_t  Servo_IsRunning(void);                           /* 运动是否进行中 */
void     Servo_SetPosition(int32_t position);             /* 设置当前位置（回零用） */
int32_t  Servo_GetPosition(void);                         /* 读当前位置 */

#ifdef __cplusplus
}
#endif

#endif /* __MOTOR_CTRL_H */
