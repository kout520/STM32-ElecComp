/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    motor_ctrl.c
  * @brief   电机伺服控制库实现：编码器 + 3 种电机驱动 + 正弦 S 曲线运动控制
  * @note    对外只暴露 Servo_* 接口；内部由 MOTOR_DRIVER_TYPE 编译期切换驱动。
  ******************************************************************************
  */
/* USER CODE END Header */
#include "motor_ctrl.h"

/* ====================================================================== */
/*                              编码器                                    */
/* ====================================================================== */

static int32_t  g_position = 0;      /* 累计绝对位置 */
static uint16_t g_last_cnt = 0;      /* 上次读取的计数器 */

static void Encoder_Init(void)
{
  /* CubeMX 常生成 2 倍频（TI1），这里运行时切到 4 倍频（TI12） */
  __HAL_TIM_DISABLE(&ENCODER_TIM);
  ENCODER_TIM.Instance->SMCR = (ENCODER_TIM.Instance->SMCR & ~TIM_SMCR_SMS) | TIM_ENCODERMODE_TI12;
  __HAL_TIM_ENABLE(&ENCODER_TIM);

  __HAL_TIM_SET_COUNTER(&ENCODER_TIM, 0);
  g_last_cnt = 0;
  g_position = 0;
}

/* 读自上次调用以来的带符号增量（16 位回绕安全），并累加进位置 */
static int16_t Encoder_GetDelta(void)
{
  uint16_t now   = (uint16_t)__HAL_TIM_GET_COUNTER(&ENCODER_TIM);
  int16_t  delta = (int16_t)(now - g_last_cnt);   /* 模 65536 相减再转有符号 */

  g_last_cnt = now;
  g_position += delta;
  return delta;
}

/* 读累计绝对位置 */
static int32_t Encoder_GetPosition(void)
{
  return g_position;
}

/* ====================================================================== */
/*                         电机驱动（3 种切换）                           */
/* ====================================================================== */

#define MOTOR_MODE_COAST    0U
#define MOTOR_MODE_FORWARD  1U
#define MOTOR_MODE_REVERSE  2U
#define MOTOR_MODE_BRAKE    3U

static uint8_t  g_mode = MOTOR_MODE_COAST;
static uint16_t g_duty = 0;   /* 速度 0..MOTOR_PWM_PERIOD（越大越快） */

#if MOTOR_DRIVER_TYPE == 1
/* ---------------- 类型1：IN1/IN2 双 PWM ---------------- */
#define M_IO_HIGH  (MOTOR_PWM_PERIOD + 1U)   /* CCR=ARR+1 → 恒高 */
#define M_IO_LOW   0U

static void Motor_Apply(void)
{
  uint16_t pwm = MOTOR_PWM_PERIOD - g_duty;   /* 速度越大 → 刹车占比越小 */

  switch (g_mode)
  {
    case MOTOR_MODE_FORWARD:
      __HAL_TIM_SET_COMPARE(&M1_TIM, M1_CH_IN1, M_IO_HIGH);
      __HAL_TIM_SET_COMPARE(&M1_TIM, M1_CH_IN2, pwm);
      break;
    case MOTOR_MODE_REVERSE:
      __HAL_TIM_SET_COMPARE(&M1_TIM, M1_CH_IN1, pwm);
      __HAL_TIM_SET_COMPARE(&M1_TIM, M1_CH_IN2, M_IO_HIGH);
      break;
    case MOTOR_MODE_BRAKE:
      __HAL_TIM_SET_COMPARE(&M1_TIM, M1_CH_IN1, M_IO_HIGH);
      __HAL_TIM_SET_COMPARE(&M1_TIM, M1_CH_IN2, M_IO_HIGH);
      break;
    case MOTOR_MODE_COAST:
    default:
      __HAL_TIM_SET_COMPARE(&M1_TIM, M1_CH_IN1, M_IO_LOW);
      __HAL_TIM_SET_COMPARE(&M1_TIM, M1_CH_IN2, M_IO_LOW);
      break;
  }
}

static void Motor_Init(void)
{
  g_mode = MOTOR_MODE_COAST;
  g_duty = 0;
  Motor_Apply();
  HAL_TIM_PWM_Start(&M1_TIM, M1_CH_IN1);
  HAL_TIM_PWM_Start(&M1_TIM, M1_CH_IN2);
}

#else
/* -------- 类型2/3：1 路 PWM + IN1/IN2 两个方向 GPIO -------- */
#if MOTOR_DRIVER_TYPE == 2
#define M_TIM       M2_TIM
#define M_CH_PWM    M2_CH_PWM
#define M_IN1_PORT  M2_IN1_PORT
#define M_IN1_PIN   M2_IN1_PIN
#define M_IN2_PORT  M2_IN2_PORT
#define M_IN2_PIN   M2_IN2_PIN
#else
#define M_TIM       M3_TIM
#define M_CH_PWM    M3_CH_PWM
#define M_IN1_PORT  M3_IN1_PORT
#define M_IN1_PIN   M3_IN1_PIN
#define M_IN2_PORT  M3_IN2_PORT
#define M_IN2_PIN   M3_IN2_PIN
#endif

static void Motor_Apply(void)
{
  /* 方向脚 + PWM 占空比直接写 */
  switch (g_mode)
  {
    case MOTOR_MODE_FORWARD:
      HAL_GPIO_WritePin(M_IN1_PORT, M_IN1_PIN, GPIO_PIN_SET);
      HAL_GPIO_WritePin(M_IN2_PORT, M_IN2_PIN, GPIO_PIN_RESET);
      break;
    case MOTOR_MODE_REVERSE:
      HAL_GPIO_WritePin(M_IN1_PORT, M_IN1_PIN, GPIO_PIN_RESET);
      HAL_GPIO_WritePin(M_IN2_PORT, M_IN2_PIN, GPIO_PIN_SET);
      break;
    case MOTOR_MODE_BRAKE:
      HAL_GPIO_WritePin(M_IN1_PORT, M_IN1_PIN, GPIO_PIN_SET);
      HAL_GPIO_WritePin(M_IN2_PORT, M_IN2_PIN, GPIO_PIN_SET);
      break;
    case MOTOR_MODE_COAST:
    default:
      HAL_GPIO_WritePin(M_IN1_PORT, M_IN1_PIN, GPIO_PIN_RESET);
      HAL_GPIO_WritePin(M_IN2_PORT, M_IN2_PIN, GPIO_PIN_RESET);
      break;
  }
  __HAL_TIM_SET_COMPARE(&M_TIM, M_CH_PWM, g_duty);
}

static void Motor_Init(void)
{
  GPIO_InitTypeDef s = {0};

  /* 方向脚配成推挽输出 */
  s.Mode  = GPIO_MODE_OUTPUT_PP;
  s.Pull  = GPIO_NOPULL;
  s.Speed = GPIO_SPEED_FREQ_LOW;
  s.Pin   = M_IN1_PIN;
  HAL_GPIO_Init(M_IN1_PORT, &s);
  s.Pin   = M_IN2_PIN;
  HAL_GPIO_Init(M_IN2_PORT, &s);

  g_mode = MOTOR_MODE_COAST;
  g_duty = 0;
  Motor_Apply();
  HAL_TIM_PWM_Start(&M_TIM, M_CH_PWM);
}
#endif

/* 设置速度（0..MOTOR_PWM_PERIOD，越大越快） */
static void Motor_SetSpeed(uint16_t duty)
{
  if (duty > MOTOR_PWM_PERIOD)
  {
    duty = MOTOR_PWM_PERIOD;
  }
  g_duty = duty;
  Motor_Apply();
}

/* 设置模式（只改方向/刹车/滑行，速度保持） */
static void Motor_SetMode(uint8_t mode)
{
  g_mode = mode;
  Motor_Apply();
}

/* ====================================================================== */
/*                    正弦查表 + S 曲线权重生成                           */
/* ====================================================================== */

/* 四分之一余弦表：cos_quarter[i] = round(65536 × cos(π/2 × i/256))，i=0..256 */
static const uint32_t cos_quarter[257] = {
    65536, 65535, 65531, 65525, 65516, 65505, 65492, 65476, 65457, 65436, 65413, 65387, 65358, 65328, 65294, 65259,
    65220, 65180, 65137, 65091, 65043, 64993, 64940, 64884, 64827, 64766, 64704, 64639, 64571, 64501, 64429, 64354,
    64277, 64197, 64115, 64031, 63944, 63854, 63763, 63668, 63572, 63473, 63372, 63268, 63162, 63054, 62943, 62830,
    62714, 62596, 62476, 62353, 62228, 62101, 61971, 61839, 61705, 61568, 61429, 61288, 61145, 60999, 60851, 60700,
    60547, 60392, 60235, 60075, 59914, 59750, 59583, 59415, 59244, 59071, 58896, 58718, 58538, 58356, 58172, 57986,
    57798, 57607, 57414, 57219, 57022, 56823, 56621, 56418, 56212, 56004, 55794, 55582, 55368, 55152, 54934, 54714,
    54491, 54267, 54040, 53812, 53581, 53349, 53114, 52878, 52639, 52398, 52156, 51911, 51665, 51417, 51166, 50914,
    50660, 50404, 50146, 49886, 49624, 49361, 49095, 48828, 48559, 48288, 48015, 47741, 47464, 47186, 46906, 46624,
    46341, 46056, 45769, 45480, 45190, 44898, 44604, 44308, 44011, 43713, 43412, 43110, 42806, 42501, 42194, 41886,
    41576, 41264, 40951, 40636, 40320, 40002, 39683, 39362, 39040, 38716, 38391, 38064, 37736, 37407, 37076, 36744,
    36410, 36075, 35738, 35401, 35062, 34721, 34380, 34037, 33692, 33347, 33000, 32652, 32303, 31952, 31600, 31248,
    30893, 30538, 30182, 29824, 29466, 29106, 28745, 28383, 28020, 27656, 27291, 26925, 26558, 26190, 25821, 25451,
    25080, 24708, 24335, 23961, 23586, 23210, 22834, 22457, 22078, 21699, 21320, 20939, 20557, 20175, 19792, 19409,
    19024, 18639, 18253, 17867, 17479, 17091, 16703, 16314, 15924, 15534, 15143, 14751, 14359, 13966, 13573, 13180,
    12785, 12391, 11996, 11600, 11204, 10808, 10411, 10014, 9616, 9218, 8820, 8421, 8022, 7623, 7224, 6824,
    6424, 6023, 5623, 5222, 4821, 4420, 4019, 3617, 3216, 2814, 2412, 2010, 1608, 1206, 804, 402,
    0
};

/* cos(π/2 × num/den) 查表 + 线性插值，返回 0..65536 */
static uint32_t Motion_CosQ(uint32_t num, uint32_t den)
{
  uint32_t scaled = num * 256U;
  uint32_t idx    = scaled / den;
  uint32_t rem    = scaled % den;
  uint32_t c0     = cos_quarter[idx];
  uint32_t c1     = (idx < 256U) ? cos_quarter[idx + 1U] : c0;
  int32_t  dc     = (int32_t)c1 - (int32_t)c0;

  return (uint32_t)((int32_t)c0 + (dc * (int32_t)rem) / (int32_t)den);
}

/* sin(θ)，θ 为自然角（度），返回定点 -65536..65536（±1.0） */
static int32_t Motion_SinDeg(int32_t deg)
{
  uint32_t d = (uint32_t)(((deg % 360) + 360) % 360);
  uint32_t q = d / 90U;
  uint32_t r = d % 90U;
  int32_t  mag = (q == 0U || q == 2U) ? (int32_t)Motion_CosQ(90U - r, 90U)
                                      : (int32_t)Motion_CosQ(r, 90U);
  return (q == 0U || q == 1U) ? mag : -mag;
}

/* 原始计数 → 自然角（0..359） */
static uint16_t AngleFromRaw(int32_t raw)
{
  int32_t a = (int32_t)(((int64_t)raw * 360) / (int64_t)ENCODER_CPR);
  a %= 360;
  if (a < 0)
  {
    a += 360;
  }
  return (uint16_t)a;
}

/* ====================================================================== */
/*                          运动控制状态                                  */
/* ====================================================================== */

static uint32_t g_weight[MOTION_SEGMENTS + 1];   /* S 曲线目标权重表 */
static uint32_t g_peak_inc;                      /* 峰值增量（速度前馈归一化） */

static int32_t  g_start;                         /* 起始位置 */
static int32_t  g_prev_pos;                      /* 上一拍位置（算速度） */
static int32_t  g_delta;                         /* 总位移（可正可负） */
static uint32_t g_duration;                      /* 运动时长 ms */
static uint32_t g_elapsed;                       /* 已过时间 ms */
static uint8_t  g_running;
static uint8_t  g_cur_mode = MOTOR_MODE_COAST;
static uint16_t g_stall_cnt;                     /* 收尾阶段编码器连续不动拍数 */

/* 生成目标权重表：正弦加速 + 正弦减速（加速/减速段数比例可调）。
 * 加速段 k∈[0,A]：p = (A/N)·(1 - cos(π·k/(2A)))
 * 减速段 k∈[A,N]：p = (A/N) + ((N-A)/N)·cos(π·(N-k)/(2(N-A))) */
static void Motion_GenWeight(void)
{
  uint32_t A = MOTION_ACCEL_SEGMENTS;
  uint32_t N = MOTION_SEGMENTS;
  uint32_t fa, fd, k;

  if (A < 1U)      { A = 1U; }
  if (A > N - 1U)  { A = N - 1U; }
  fa = A * 65536U / N;
  fd = (N - A) * 65536U / N;

  for (k = 0; k <= N; k++)
  {
    if (k <= A)
    {
      uint32_t c = Motion_CosQ(k, A);
      g_weight[k] = (uint32_t)(((uint64_t)fa * (uint64_t)(65536U - c)) / 65536U);
    }
    else
    {
      uint32_t c = Motion_CosQ(N - k, N - A);
      g_weight[k] = fa + (uint32_t)(((uint64_t)fd * (uint64_t)c) / 65536U);
    }
  }

  g_peak_inc = 0;
  for (k = 0; k < N; k++)
  {
    uint32_t d = g_weight[k + 1] - g_weight[k];
    if (d > g_peak_inc)
    {
      g_peak_inc = d;
    }
  }
}

/* ====================================================================== */
/*                          公共 API 实现                                 */
/* ====================================================================== */

/**
  * @brief  初始化编码器和电机驱动，上电默认滑行 + 零速度
  */
void Servo_Init(void)
{
  Encoder_Init();
  Motor_Init();
}

/**
  * @brief  1ms 定时中断里调用：读编码器 + 跑位置闭环
  */
void Servo_Tick1ms(void)
{
  int32_t  pos, desired, error, vel;
  uint32_t duty;
  uint32_t x10, k, f, s, inc;
  int32_t  dir;
  uint8_t  kick_on;

  if (!g_running)
  {
    return;
  }

  g_elapsed++;
  Encoder_GetDelta();             /* 读编码器计数器，累加位置 */
  pos = Encoder_GetPosition();
  vel = pos - g_prev_pos;
  g_prev_pos = pos;

  /* ---- S 曲线轨迹 ---- */
  x10 = g_elapsed * (uint32_t)MOTION_SEGMENTS * 1024U / g_duration;
  if (x10 > (uint32_t)MOTION_SEGMENTS * 1024U)
  {
    x10 = (uint32_t)MOTION_SEGMENTS * 1024U;
  }
  k = x10 / 1024U;
  f = x10 % 1024U;

  if (k >= MOTION_SEGMENTS)
  {
    s   = g_weight[MOTION_SEGMENTS];
    inc = 0;
  }
  else
  {
    int32_t inc_cur  = (int32_t)(g_weight[k + 1] - g_weight[k]);
    int32_t inc_next = (k + 1U < MOTION_SEGMENTS)
                       ? (int32_t)(g_weight[k + 2] - g_weight[k + 1]) : 0;
    s = g_weight[k] + (uint32_t)(((uint64_t)inc_cur * (uint64_t)f) / 1024U);
    inc = (uint32_t)(inc_cur + (int32_t)(((int64_t)(inc_next - inc_cur) * (int64_t)f) / 1024));
  }

  desired = g_start + (int32_t)(((int64_t)g_delta * (int64_t)s) / 65536);
  error   = desired - pos;

  /* ---- 到位判断：误差进死区，或编码器堵转不动，则完成 ---- */
  if (g_elapsed >= g_duration)
  {
    int32_t ferr  = (g_start + g_delta) - pos;
    int32_t mferr = (ferr >= 0) ? ferr : -ferr;
    uint8_t done  = 0;

    if (mferr <= MOTION_DEADBAND)
    {
      done = 1;   /* 正常到位 */
    }
    else if (vel == 0)
    {
      g_stall_cnt++;
      if (g_stall_cnt >= MOTION_STALL_MS)
      {
        done = 1;   /* 堵转判定完成 */
      }
    }
    else
    {
      g_stall_cnt = 0;
    }

    if (done)
    {
      Motor_SetMode(MOTOR_MODE_BRAKE);   /* 到位短刹车保持 */
      g_cur_mode = MOTOR_MODE_BRAKE;
      g_running  = 0;
      return;
    }
  }

  kick_on = (g_elapsed <= (uint32_t)MOTION_KICK_MS) ? 1U : 0U;

  /* ---- 合成控制量 = 粘性前馈 + 库仑摩擦 + 重力前馈 + P 纠偏 - D 阻尼 ---- */
  {
    uint32_t dist = (g_delta >= 0) ? (uint32_t)g_delta : (uint32_t)(-g_delta);
    uint32_t peak = (uint32_t)(((uint64_t)MOTION_KV * (uint64_t)dist) / g_duration);
    if (peak > (uint32_t)MOTION_PEAK_DUTY)
    {
      peak = (uint32_t)MOTION_PEAK_DUTY;
    }

    /* 摩擦补偿方向：巡航跟运动方向，收尾跟误差方向 */
    if (inc > 0U)
    {
      dir = (g_delta >= 0) ? 1 : -1;
    }
    else if (g_elapsed >= g_duration)
    {
      dir = (error > 0) ? 1 : ((error < 0) ? -1 : 0);
    }
    else
    {
      dir = 0;
    }

    int32_t cmd = (g_peak_inc > 0U) ? (int32_t)(peak * inc / g_peak_inc) : 0;
    if (g_delta < 0)
    {
      cmd = -cmd;
    }
    cmd += dir * (int32_t)MOTION_FF_FRICTION;
    cmd += (int32_t)(((int64_t)MOTION_GRAV_AMP *
                      Motion_SinDeg((int32_t)AngleFromRaw(pos) - (int32_t)MOTION_GRAV_PHASE)) / 65536);
    cmd += (int32_t)MOTION_KP * error;
    cmd -= (int32_t)MOTION_KD * vel;

    /* ---- 方向选择 + 换向滞回 ---- */
    {
      uint8_t want_forward = ((cmd * MOTION_DIR_SIGN) >= 0) ? 1U : 0U;
      uint8_t want_mode    = want_forward ? MOTOR_MODE_FORWARD : MOTOR_MODE_REVERSE;

      if (g_cur_mode != want_mode)
      {
        if (g_cur_mode != MOTOR_MODE_FORWARD && g_cur_mode != MOTOR_MODE_REVERSE)
        {
          /* 从滑行/刹车起步：直接选方向 */
          Motor_SetSpeed(0);
          Motor_SetMode(want_mode);
          g_cur_mode = want_mode;
        }
        else
        {
          /* 已驱动想反向：误差超差才换向，否则滞回带内不驱动 */
          uint8_t do_switch = (want_forward && error <= -(int32_t)MOTION_DIR_HYST) ||
                              (!want_forward && error >= (int32_t)MOTION_DIR_HYST);
          if (do_switch)
          {
            Motor_SetSpeed(0);
            Motor_SetMode(want_mode);
            g_cur_mode = want_mode;
          }
          else
          {
            cmd = 0;
          }
        }
      }
    }

    /* 占空比 = |cmd|，起转踢脚兜底 */
    duty = (cmd >= 0) ? (uint32_t)cmd : (uint32_t)(-cmd);
    if (duty > MOTOR_PWM_PERIOD)
    {
      duty = MOTOR_PWM_PERIOD;
    }
    if (kick_on && duty < MOTION_KICK_DUTY)
    {
      duty = MOTION_KICK_DUTY;
    }
    Motor_SetSpeed((uint16_t)duty);
  }
}

/**
  * @brief  运动到目标计数，指定时长（S 曲线平滑加减速）
  * @param  target    目标绝对位置（编码器计数）
  * @param  time_ms   运动时长（1~30000ms）
  */
void Servo_MoveTo(int32_t target, uint32_t time_ms)
{
  Motion_GenWeight();
  g_start    = Encoder_GetPosition();
  g_prev_pos = g_start;
  g_delta    = target - g_start;
  g_duration = (time_ms < 1U) ? 1U : time_ms;
  g_elapsed  = 0;
  g_stall_cnt = 0;
  g_running  = 1;
}

/**
  * @brief  停止：短刹车保持当前位置
  */
void Servo_Stop(void)
{
  g_running  = 0;
  g_cur_mode = MOTOR_MODE_BRAKE;
  Motor_SetMode(MOTOR_MODE_BRAKE);
}

/**
  * @brief  停止：滑行释放
  */
void Servo_Coast(void)
{
  g_running  = 0;
  g_cur_mode = MOTOR_MODE_COAST;
  Motor_SetMode(MOTOR_MODE_COAST);
}

/**
  * @brief  运动是否进行中
  */
uint8_t Servo_IsRunning(void)
{
  return g_running;
}

/**
  * @brief  设置当前位置（回零/设零点用）
  */
void Servo_SetPosition(int32_t position)
{
  g_position = position;
}

/**
  * @brief  读取当前位置
  */
int32_t Servo_GetPosition(void)
{
  return g_position;
}
