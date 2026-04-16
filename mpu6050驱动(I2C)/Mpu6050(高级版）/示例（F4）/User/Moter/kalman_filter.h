#ifndef __KALMAN_FILTER_H
#define __KALMAN_FILTER_H

#include <math.h>
#include <stdio.h>

// 2x2矩阵结构体
typedef struct {
    double data[2][2];
} Matrix2x2;

// 2维向量结构体
typedef struct {
    double data[2];
} Vector2;

// 卡尔曼滤波器结构体
typedef struct {
    Vector2 x;          // 状态向量 [位置, 速度]
    Matrix2x2 P;        // 误差协方差矩阵
    Matrix2x2 F;        // 状态转移矩阵
    Matrix2x2 Q;        // 过程噪声协方差
    double R;           // 观测噪声协方差
} KalmanFilter;

// 函数声明
void matrix_mult_2x2(const Matrix2x2* A, const Matrix2x2* B, Matrix2x2* result);
void matrix_transpose_2x2(const Matrix2x2* A, Matrix2x2* result);
void matrix_add_2x2(const Matrix2x2* A, const Matrix2x2* B, Matrix2x2* result);
void matrix_sub_2x2(const Matrix2x2* A, const Matrix2x2* B, Matrix2x2* result);
void matrix_scale_2x2(double scalar, const Matrix2x2* A, Matrix2x2* result);

void kalman_init(KalmanFilter* kf, const Vector2* initial_state, 
                 const Matrix2x2* initial_covariance, const Matrix2x2* F, 
                 const Matrix2x2* Q, double R);
void kalman_predict(KalmanFilter* kf);
void kalman_update(KalmanFilter* kf, double measurement);

void print_matrix(const char* name, const Matrix2x2* m);
void print_vector(const char* name, const Vector2* v);

#endif /* KALMAN_FILTER_H */