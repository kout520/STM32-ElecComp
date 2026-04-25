#include "imu.h"
#include "MPU6050.h"
#include "HMC5883.h"
#include <math.h>

#ifndef M_PI
#define M_PI 3.141592653589f
#endif

/* Mahony 参数，Kp 越大收敛越快但越抖，Ki 用于消除陀螺仪零偏 */
#define Kp  2.5f
#define Ki  0.008f

/* 陀螺仪 ±2000dps => 16.4 LSB/dps，转 rad/s */
#define GYRO_SCALE  (1.0f / 16.4f / 57.2957795f)

/* 加速度计 ±4g => 8192 LSB/g */
#define ACCEL_SCALE (1.0f / 8192.0f)

/* 四元数 */
static float q0 = 1.0f, q1 = 0.0f, q2 = 0.0f, q3 = 0.0f;

/* 积分误差 */
static float ex_int = 0.0f, ey_int = 0.0f, ez_int = 0.0f;



/* 快速反平方根 */
static float inv_sqrt(float x)
{
    float halfx = 0.5f * x;
    float y = x;
    long i = *(long *)&y;
    i = 0x5f3759df - (i >> 1);
    y = *(float *)&i;
    y = y * (1.5f - halfx * y * y);
    return y;
}

void IMU_CalcEulerAngles(EulerAngle_t *euler)
{
    MPU6050_RawData_t mpu;
    int16_t mx_raw, my_raw, mz_raw;

    float ax, ay, az;
    float gx, gy, gz;
    float mx, my, mz;
    float norm;
    float dt;


    /* 由四元数推导的参考方向 */
    float hx, hy, bx, bz;
    float vx, vy, vz;
    float wx, wy, wz;
    float ex, ey, ez;

    if (euler == NULL) return;
    if (MPU6050_ReadAllRaw(&mpu) != HAL_OK) return;
    if (HMC5883L_ReadRaw(&mx_raw, &my_raw, &mz_raw) != HAL_OK) return;

    /* 单位换算 */
    ax = (float)mpu.ax * ACCEL_SCALE;
    ay = (float)mpu.ay * ACCEL_SCALE;
    az = (float)mpu.az * ACCEL_SCALE;

    gx = (float)mpu.gx * GYRO_SCALE;
    gy = (float)mpu.gy * GYRO_SCALE;
    gz = (float)mpu.gz * GYRO_SCALE;

    mx = (float)mx_raw;
    my = (float)my_raw;
    mz = (float)mz_raw;

    /* dt */
    dt = 0.01f;

    /* 加速度归一化 */
    norm = inv_sqrt(ax*ax + ay*ay + az*az);
    ax *= norm; ay *= norm; az *= norm;

    /* 磁力计归一化 */
    norm = inv_sqrt(mx*mx + my*my + mz*mz);
    mx *= norm; my *= norm; mz *= norm;

    /* 把磁力计投影到水平面（参考方向） */
    hx = 2.0f * (mx*(0.5f - q2*q2 - q3*q3) + my*(q1*q2 - q0*q3) + mz*(q1*q3 + q0*q2));
    hy = 2.0f * (mx*(q1*q2 + q0*q3) + my*(0.5f - q1*q1 - q3*q3) + mz*(q2*q3 - q0*q1));
    bx = sqrtf(hx*hx + hy*hy);
    bz = 2.0f * (mx*(q1*q3 - q0*q2) + my*(q2*q3 + q0*q1) + mz*(0.5f - q1*q1 - q2*q2));

    /* 估计重力和磁场方向 */
    vx = 2.0f * (q1*q3 - q0*q2);
    vy = 2.0f * (q0*q1 + q2*q3);
    vz = q0*q0 - q1*q1 - q2*q2 + q3*q3;

    wx = 2.0f * (bx*(0.5f - q2*q2 - q3*q3) + bz*(q1*q3 - q0*q2));
    wy = 2.0f * (bx*(q1*q2 - q0*q3) + bz*(q0*q1 + q2*q3));
    wz = 2.0f * (bx*(q0*q2 + q1*q3) + bz*(0.5f - q1*q1 - q2*q2));

    /* 误差：测量值和估计值的叉积 */
    ex = (ay*vz - az*vy) + (my*wz - mz*wy);
    ey = (az*vx - ax*vz) + (mz*wx - mx*wz);
    ez = (ax*vy - ay*vx) + (mx*wy - my*wx);

    /* PI 修正陀螺仪 */
    ex_int += Ki * ex * dt;
    ey_int += Ki * ey * dt;
    ez_int += Ki * ez * dt;

    gx += Kp * ex + ex_int;
    gy += Kp * ey + ey_int;
    gz += Kp * ez + ez_int;

    /* 四元数更新（一阶龙格库塔） */
    q0 += (-q1*gx - q2*gy - q3*gz) * (0.5f * dt);
    q1 += ( q0*gx + q2*gz - q3*gy) * (0.5f * dt);
    q2 += ( q0*gy - q1*gz + q3*gx) * (0.5f * dt);
    q3 += ( q0*gz + q1*gy - q2*gx) * (0.5f * dt);

    /* 四元数归一化 */
    norm = inv_sqrt(q0*q0 + q1*q1 + q2*q2 + q3*q3);
    q0 *= norm; q1 *= norm; q2 *= norm; q3 *= norm;

    /* 四元数转欧拉角 */
    euler->roll  = atan2f(2.0f*(q0*q1 + q2*q3), 1.0f - 2.0f*(q1*q1 + q2*q2)) * 180.0f / M_PI;
    euler->pitch = asinf(2.0f*(q0*q2 - q3*q1)) * 180.0f / M_PI;
    euler->yaw   = atan2f(2.0f*(q0*q3 + q1*q2), 1.0f - 2.0f*(q2*q2 + q3*q3)) * 180.0f / M_PI;

    if (euler->yaw < 0.0f) euler->yaw += 360.0f;

    euler->temperature = MPU6050_GetTemperature(mpu.temp);
}
