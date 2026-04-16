#include "kalman_filter.h"

// 2x2矩阵乘法: C = A * B
void matrix_mult_2x2(const Matrix2x2* A, const Matrix2x2* B, Matrix2x2* result) {
    int i, j, k;
    for (i = 0; i < 2; i++) {
        for (j = 0; j < 2; j++) {
            result->data[i][j] = 0.0;
            for (k = 0; k < 2; k++) {
                result->data[i][j] += A->data[i][k] * B->data[k][j];
            }
        }
    }
}

// 2x2矩阵转置
void matrix_transpose_2x2(const Matrix2x2* A, Matrix2x2* result) {
    result->data[0][0] = A->data[0][0];
    result->data[0][1] = A->data[1][0];
    result->data[1][0] = A->data[0][1];
    result->data[1][1] = A->data[1][1];
}

// 2x2矩阵加法: C = A + B
void matrix_add_2x2(const Matrix2x2* A, const Matrix2x2* B, Matrix2x2* result) {
    int i, j;
    for (i = 0; i < 2; i++) {
        for (j = 0; j < 2; j++) {
            result->data[i][j] = A->data[i][j] + B->data[i][j];
        }
    }
}

// 2x2矩阵减法: C = A - B
void matrix_sub_2x2(const Matrix2x2* A, const Matrix2x2* B, Matrix2x2* result) {
    int i, j;
    for (i = 0; i < 2; i++) {
        for (j = 0; j < 2; j++) {
            result->data[i][j] = A->data[i][j] - B->data[i][j];
        }
    }
}

// 矩阵乘以标量: B = scalar * A
void matrix_scale_2x2(double scalar, const Matrix2x2* A, Matrix2x2* result) {
    int i, j;
    for (i = 0; i < 2; i++) {
        for (j = 0; j < 2; j++) {
            result->data[i][j] = scalar * A->data[i][j];
        }
    }
}

// 初始化卡尔曼滤波器
void kalman_init(KalmanFilter* kf, const Vector2* initial_state, 
                 const Matrix2x2* initial_covariance, const Matrix2x2* F, 
                 const Matrix2x2* Q, double R) {
    kf->x = *initial_state;
    kf->P = *initial_covariance;
    kf->F = *F;
    kf->Q = *Q;
    kf->R = R;
}

// 卡尔曼滤波预测步骤
void kalman_predict(KalmanFilter* kf) {
    Vector2 x_pred;
    Matrix2x2 F_P, F_T, F_P_FT;
    
    // 预测状态: x = F * x
    x_pred.data[0] = kf->F.data[0][0] * kf->x.data[0] + kf->F.data[0][1] * kf->x.data[1];
    x_pred.data[1] = kf->F.data[1][0] * kf->x.data[0] + kf->F.data[1][1] * kf->x.data[1];
    
    // 预测协方差: P = F * P * F^T + Q
    matrix_mult_2x2(&kf->F, &kf->P, &F_P);           // F * P
    matrix_transpose_2x2(&kf->F, &F_T);              // F^T
    matrix_mult_2x2(&F_P, &F_T, &F_P_FT);            // F * P * F^T
    matrix_add_2x2(&F_P_FT, &kf->Q, &kf->P);         // F*P*F^T + Q
    
    // 更新滤波器状态
    kf->x = x_pred;
}

// 卡尔曼滤波更新步骤
void kalman_update(KalmanFilter* kf, double measurement) {
    // 先声明所有变量
    double H[2] = {1.0, 0.0};  // 观测矩阵 H = [1, 0]
    Vector2 P_HT;
    double H_P_HT_R;
    Vector2 K;
    double H_x;
    double residual;
    Vector2 x_updated;
    Matrix2x2 K_H;
    Matrix2x2 I = {{{1.0, 0.0}, {0.0, 1.0}}};  // 单位矩阵
    Matrix2x2 I_KH;
    Matrix2x2 P_updated;
    
    // ========================================================================
    // 计算卡尔曼增益: K = P * H^T / (H * P * H^T + R)
    // ========================================================================
    
    // 计算 P * H^T
    P_HT.data[0] = kf->P.data[0][0] * H[0] + kf->P.data[0][1] * H[1];
    P_HT.data[1] = kf->P.data[1][0] * H[0] + kf->P.data[1][1] * H[1];
    
    // 计算 H * P * H^T + R
    H_P_HT_R = H[0] * P_HT.data[0] + H[1] * P_HT.data[1] + kf->R;
    
    // 计算卡尔曼增益 K
    K.data[0] = P_HT.data[0] / H_P_HT_R;
    K.data[1] = P_HT.data[1] / H_P_HT_R;
    
    // ========================================================================
    // 更新状态估计: x = x + K * (z - H * x)
    // ========================================================================
    
    // 计算预测的观测值: H * x
    H_x = H[0] * kf->x.data[0] + H[1] * kf->x.data[1];
    
    // 计算残差（新息）: z - H * x
    residual = measurement - H_x;
    
    // 使用卡尔曼增益和残差更新状态估计
    x_updated.data[0] = kf->x.data[0] + K.data[0] * residual;  // 更新位置
    x_updated.data[1] = kf->x.data[1] + K.data[1] * residual;  // 更新速度
    
    // ========================================================================
    // 更新协方差: P = (I - K * H) * P
    // ========================================================================
    
    // 计算 K * H
    K_H.data[0][0] = K.data[0] * H[0];
    K_H.data[0][1] = K.data[0] * H[1];
    K_H.data[1][0] = K.data[1] * H[0];
    K_H.data[1][1] = K.data[1] * H[1];
    
    // 计算 I - K * H
    matrix_sub_2x2(&I, &K_H, &I_KH);
    
    // 计算更新后的协方差
    matrix_mult_2x2(&I_KH, &kf->P, &P_updated);
    
    // 更新滤波器状态
    kf->x = x_updated;
    kf->P = P_updated;
}

// 打印矩阵
void print_matrix(const char* name, const Matrix2x2* m) {
    printf("%s:\n", name);
    printf("[%8.4f, %8.4f]\n", m->data[0][0], m->data[0][1]);
    printf("[%8.4f, %8.4f]\n", m->data[1][0], m->data[1][1]);
    printf("\n");
}

// 打印向量
void print_vector(const char* name, const Vector2* v) {
    printf("%s: [%8.4f, %8.4f]\n", name, v->data[0], v->data[1]);
}