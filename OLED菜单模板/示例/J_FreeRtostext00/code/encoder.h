#ifndef __ENCODER_H__
#define __ENCODER_H__



typedef struct {
    int32_t pulse_count;          // 当前脉冲计数
    int32_t last_pulse_count;     // 上一次脉冲计数
    float speed;                  // 计算的速度
    float wheel_circumference;    // 轮子周长
    uint16_t pulses_per_revolution; // 每转脉冲数
} Encoder;

// 函数声明
void Encoder_Init(Encoder *encoder, float wheel_circumference, uint16_t pulses_per_revolution);
void Encoder_Update(Encoder *encoder, int32_t pulse_count, float time_interval);
float Encoder_Get_Speed(Encoder *encoder);
float Encoder_Get_Distance(Encoder *encoder);

#endif // ENCODER_H


