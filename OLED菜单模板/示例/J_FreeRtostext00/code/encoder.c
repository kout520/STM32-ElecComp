#include "headfile.h"
/**
 * @brief 初始化编码器
 * @param encoder 编码器结构体指针
 * @param wheel_circumference 轮子周长（单位：米）
 * @param pulses_per_revolution 每转脉冲数
 */
void Encoder_Init(Encoder *encoder, float wheel_circumference, uint16_t pulses_per_revolution) {
    encoder->pulse_count = 0;
    encoder->last_pulse_count = 0;
    encoder->speed = 0.0f;
    encoder->wheel_circumference = wheel_circumference;
    encoder->pulses_per_revolution = pulses_per_revolution;
}
/**
 * @brief 更新编码器脉冲计数并计算速度
 * @param encoder 编码器结构体指针
 * @param pulse_count 当前脉冲计数
 * @param time_interval 时间间隔（单位：秒）
 */
void Encoder_Update(Encoder *encoder, int32_t pulse_count, float time_interval) {
    if (time_interval <= 0.0f) {
        time_interval = 0.01f; // 默认值
    }
    
    int32_t delta_pulse = pulse_count - encoder->last_pulse_count;
    encoder->last_pulse_count = pulse_count;
    // 处理方向变化
    if (delta_pulse > 32768) {
        delta_pulse -= 65536; // 处理正向溢出
    } else if (delta_pulse < -32768) {
        delta_pulse += 65536; // 处理负向溢出
    }
    encoder->pulse_count += delta_pulse; // 累加脉冲计数
    encoder->speed = (delta_pulse / (float)encoder->pulses_per_revolution) * encoder->wheel_circumference / time_interval;
}

/**
 * @brief 获取编码器计算的速度
 * @param encoder 编码器结构体指针
 * @return 速度（单位：米/秒）
 */
float Encoder_Get_Speed(Encoder *encoder) {
    return encoder->speed;
}
/**
 * @brief 获取编码器记录的行驶距离
 * @param encoder 编码器结构体指针
 * @return 行驶距离（单位：米）
 */
float Encoder_Get_Distance(Encoder *encoder) {
    float distance = (float)encoder->pulse_count / (float)encoder->pulses_per_revolution * encoder->wheel_circumference;
    return distance;
}

