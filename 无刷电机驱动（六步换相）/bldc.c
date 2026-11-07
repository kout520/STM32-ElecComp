/*
 * 无刷电机（BLDC）六步换相驱动 —— STM32F1 + HAL 库
 *
 * ============================ 使用说明 ============================
 * 1. CubeMX 配置 TIM1（高级定时器）为 PWM 输出：
 *    - 打开 3 个通道 CH1/CH2/CH3，PWM 模式 1，向上计数（边沿对齐）
 *    - 打开互补输出 CH1N/CH2N/CH3N，死区时间建议 0.5us ~ 2us
 *    - PWM 频率建议 10kHz ~ 20kHz（按电机与驱动板参数调整 ARR/PSC）
 * 2. 霍尔三路 GPIO（默认 PB5/PB6/PB7）配为外部中断（双边沿触发），
 *    在 HAL_GPIO_EXTI_Callback 中调用 BLDC_Commutation()；
 *    也可以不配 EXTI，在主循环里高频调用 BLDC_Commutation()。
 * 3. 接线：
 *    - TIM1 的 CH1/CH1N、CH2/CH2N、CH3/CH3N 共 6 路接驱动板的
 *      UH/UL、VH/VL、WH/WL 六个栅极输入
 *    - 霍尔模块三路输出接 HA/HB/HC
 * 4. 调用顺序：
 *    BLDC_Init();
 *    BLDC_Set_Direction(BLDC_DIR_FORWARD);
 *    BLDC_Set_Speed(30.0f);   // 30% 占空比起转
 *    BLDC_Start();
 *
 * ============================ 排错提示 ============================
 * - 电机不转或抖动：检查霍尔是否接好、供电是否正常；尝试交换任意两根
 *   电机相线，或交换任意两路霍尔线后重试。
 * - 转向相反：交换任意两根电机相线，或调用 BLDC_Set_Direction 反转。
 * ===================================================================
 */

#include "bldc.h"

/* 定时器句柄在 main.c 中由 CubeMX 定义，这里引用即可 */
extern TIM_HandleTypeDef BLDC_TIM_HANDLE;

/* ---------------- 内部变量 ---------------- */
static uint16_t duty = 0;                             /* 当前占空比（CCR 值） */
static BLDC_Direction direction = BLDC_DIR_FORWARD;   /* 当前转向 */
static volatile uint32_t hall_change_count = 0;       /* 霍尔变化累计次数 */
static uint8_t last_hall = 0xFF;                      /* 上一次霍尔状态 */

/* ---------------- 相状态 ---------------- */
typedef enum {
    PHASE_FLOAT = 0,  /* 悬空：上下桥臂都关断 */
    PHASE_HIGH  = 1,  /* 高侧导通：上桥臂 PWM 斩波 */
    PHASE_LOW   = 2,  /* 低侧导通：下桥臂常通 */
} PhaseState;

typedef struct {
    PhaseState u;
    PhaseState v;
    PhaseState w;
} CommutationStep;

/*
 * 正转换相表（默认方向），下标为霍尔编码对应的步序
 *  H = 高侧(PWM)，L = 低侧(常通)，- = 悬空
 *   步序   霍尔(HA HB HC)   U   V   W
 *    0         001          -   H   L     电流 V -> W
 *    1         011          L   H   -     电流 V -> U
 *    2         010          L   -   H     电流 W -> U
 *    3         110          -   L   H     电流 W -> V
 *    4         100          H   L   -     电流 U -> V
 *    5         101          H   -   L     电流 U -> W
 */
static const CommutationStep comm_table[6] = {
    { PHASE_FLOAT, PHASE_HIGH, PHASE_LOW  },   /* 霍尔 001 -> 步 0 */
    { PHASE_LOW,   PHASE_HIGH, PHASE_FLOAT },   /* 霍尔 011 -> 步 1 */
    { PHASE_LOW,   PHASE_FLOAT, PHASE_HIGH },   /* 霍尔 010 -> 步 2 */
    { PHASE_FLOAT, PHASE_LOW,  PHASE_HIGH },   /* 霍尔 110 -> 步 3 */
    { PHASE_HIGH,  PHASE_LOW,  PHASE_FLOAT },   /* 霍尔 100 -> 步 4 */
    { PHASE_HIGH,  PHASE_FLOAT, PHASE_LOW  },   /* 霍尔 101 -> 步 5 */
};

/* 霍尔编码(0~7) -> 换相步序；000/111 为无效状态，返回 0xFF */
static const uint8_t hall_to_step[8] = {
    0xFF, 0, 2, 1, 4, 5, 3, 0xFF
};

/* CCER 中 6 路输出的使能位掩码（用于保留极性位、只改使能位） */
#define CCER_EN_MASK  (TIM_CCER_CC1E  | TIM_CCER_CC1NE | \
                       TIM_CCER_CC2E  | TIM_CCER_CC2NE | \
                       TIM_CCER_CC3E  | TIM_CCER_CC3NE)

/*
 * 根据相状态设置三相输出。
 * 直接操作寄存器（CCMR/CCER），速度快，适合放在中断里调用。
 * 说明：
 *   - 高侧 PWM：通道设为 PWM 模式 1，使能 OCx（高侧），关闭 OCxN
 *   - 低侧常通：通道设为强制无效，使能 OCxN（低侧），关闭 OCx
 *     （OCx 参考强制为低 -> OCxN 互补为高 -> 低侧 MOSFET 常通）
 *   - 悬空：OCx 与 OCxN 都关闭
 */
static void BLDC_Apply_Step(const CommutationStep *step)
{
    TIM_TypeDef *TIMx = BLDC_TIM_HANDLE.Instance;
    uint32_t ccmr1 = TIMx->CCMR1;
    uint32_t ccmr2 = TIMx->CCMR2;
    uint32_t ccer  = 0;

    /* 清除三个通道的 OC 模式位，稍后按需重写 */
    ccmr1 &= ~(TIM_CCMR1_OC1M | TIM_CCMR1_OC2M);
    ccmr2 &= ~TIM_CCMR2_OC3M;

    /* ---- U 相 (CH1) ---- */
    if (step->u == PHASE_HIGH) {
        ccmr1 |= TIM_OCMODE_PWM1;                     /* 高侧 PWM 斩波 */
        ccer  |= TIM_CCER_CC1E;
        __HAL_TIM_SET_COMPARE(&BLDC_TIM_HANDLE, BLDC_CH_U, duty);
    } else if (step->u == PHASE_LOW) {
        ccmr1 |= TIM_OCMODE_FORCED_INACTIVE;          /* 低侧常通 */
        ccer  |= TIM_CCER_CC1NE;
    }

    /* ---- V 相 (CH2) ---- */
    if (step->v == PHASE_HIGH) {
        ccmr1 |= (TIM_OCMODE_PWM1 << 8);              /* OC2M 位于 CCMR1[14:12] */
        ccer  |= TIM_CCER_CC2E;
        __HAL_TIM_SET_COMPARE(&BLDC_TIM_HANDLE, BLDC_CH_V, duty);
    } else if (step->v == PHASE_LOW) {
        ccmr1 |= (TIM_OCMODE_FORCED_INACTIVE << 8);
        ccer  |= TIM_CCER_CC2NE;
    }

    /* ---- W 相 (CH3) ---- */
    if (step->w == PHASE_HIGH) {
        ccmr2 |= TIM_OCMODE_PWM1;                     /* OC3M 位于 CCMR2[6:4] */
        ccer  |= TIM_CCER_CC3E;
        __HAL_TIM_SET_COMPARE(&BLDC_TIM_HANDLE, BLDC_CH_W, duty);
    } else if (step->w == PHASE_LOW) {
        ccmr2 |= TIM_OCMODE_FORCED_INACTIVE;
        ccer  |= TIM_CCER_CC3NE;
    }

    /* 写回模式位；输出使能位只更新使能部分，保留 CCxP/CCxNP 极性位 */
    TIMx->CCMR1 = ccmr1;
    TIMx->CCMR2 = ccmr2;
    TIMx->CCER  = (TIMx->CCER & ~CCER_EN_MASK) | ccer;
}

/* ============================ 对外函数 ============================ */

/* 初始化 */
void BLDC_Init(void)
{
    /* 启动三路 PWM（会启动计数器，并对 TIM1 这类带刹车的定时器使能 MOE） */
    HAL_TIM_PWM_Start(&BLDC_TIM_HANDLE, BLDC_CH_U);
    HAL_TIM_PWM_Start(&BLDC_TIM_HANDLE, BLDC_CH_V);
    HAL_TIM_PWM_Start(&BLDC_TIM_HANDLE, BLDC_CH_W);

    /* 进入安全状态：占空比 0、方向默认正转、所有输出关断 */
    duty = 0;
    direction = BLDC_DIR_FORWARD;
    last_hall = 0xFF;
    hall_change_count = 0;

    BLDC_TIM_HANDLE.Instance->CCER &= ~CCER_EN_MASK;
    __HAL_TIM_SET_COMPARE(&BLDC_TIM_HANDLE, BLDC_CH_U, 0);
    __HAL_TIM_SET_COMPARE(&BLDC_TIM_HANDLE, BLDC_CH_V, 0);
    __HAL_TIM_SET_COMPARE(&BLDC_TIM_HANDLE, BLDC_CH_W, 0);

    __HAL_TIM_MOE_ENABLE(&BLDC_TIM_HANDLE);   /* 使能主输出 */
}

/*
 * 可选：用代码初始化霍尔 GPIO + 外部中断（双边沿触发）。
 * 若已在 CubeMX 中配置好霍尔引脚与 EXTI，可跳过本函数。
 */
void BLDC_Hall_Init(void)
{
    GPIO_InitTypeDef gpio = {0};

    __HAL_RCC_GPIOB_CLK_ENABLE();   /* 默认 PB 口；改端口时对应修改 */

    gpio.Pin  = HALL_A_PIN | HALL_B_PIN | HALL_C_PIN;
    gpio.Mode = GPIO_MODE_IT_RISING_FALLING;   /* 双边沿触发换相 */
    gpio.Pull = GPIO_PULLUP;                   /* 根据霍尔模块输出类型调整 */
    HAL_GPIO_Init(HALL_GPIO_PORT, &gpio);

    /* PB5/PB6/PB7 对应 EXTI5/6/7，共用 EXTI9_5 中断；
       若改用引脚 10~15，请改为 EXTI15_10_IRQn */
    HAL_NVIC_SetPriority(EXTI9_5_IRQn, 1, 0);
    HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);
}

/* 启动：使能主输出，等待换相 */
void BLDC_Start(void)
{
    __HAL_TIM_MOE_ENABLE(&BLDC_TIM_HANDLE);
}

/* 停止：关闭所有输出，电机自由滑行 */
void BLDC_Stop(void)
{
    BLDC_TIM_HANDLE.Instance->CCER &= ~CCER_EN_MASK;
    __HAL_TIM_MOE_DISABLE(&BLDC_TIM_HANDLE);
    __HAL_TIM_SET_COMPARE(&BLDC_TIM_HANDLE, BLDC_CH_U, 0);
    __HAL_TIM_SET_COMPARE(&BLDC_TIM_HANDLE, BLDC_CH_V, 0);
    __HAL_TIM_SET_COMPARE(&BLDC_TIM_HANDLE, BLDC_CH_W, 0);
}

/* 设置占空比（CCR 值，0 ~ ARR） */
void BLDC_Set_Duty(uint16_t duty_ccr)
{
    uint32_t arr = __HAL_TIM_GET_AUTORELOAD(&BLDC_TIM_HANDLE);

    if (duty_ccr > arr) {
        duty_ccr = (uint16_t)arr;   /* 限幅，防止超过 100% */
    }
    duty = duty_ccr;

    /* 同时更新三路比较值；只有处于 PWM 的那相才真正受 CCR 影响 */
    __HAL_TIM_SET_COMPARE(&BLDC_TIM_HANDLE, BLDC_CH_U, duty);
    __HAL_TIM_SET_COMPARE(&BLDC_TIM_HANDLE, BLDC_CH_V, duty);
    __HAL_TIM_SET_COMPARE(&BLDC_TIM_HANDLE, BLDC_CH_W, duty);
}

/* 设置占空比（百分比 0.0 ~ 100.0） */
void BLDC_Set_Speed(float percent)
{
    uint32_t arr = __HAL_TIM_GET_AUTORELOAD(&BLDC_TIM_HANDLE);

    if (percent < 0.0f) {
        percent = 0.0f;
    }
    if (percent > 100.0f) {
        percent = 100.0f;
    }
    BLDC_Set_Duty((uint16_t)((float)arr * percent / 100.0f));
}

/* 设置转向 */
void BLDC_Set_Direction(BLDC_Direction dir)
{
    direction = dir;
}

/* 读取霍尔状态：0b(HA HB HC)，HA 最高位 */
uint8_t BLDC_Read_Hall(void)
{
    uint8_t hall = 0;

    if (HAL_GPIO_ReadPin(HALL_GPIO_PORT, HALL_A_PIN) != GPIO_PIN_RESET) {
        hall |= 0x04;
    }
    if (HAL_GPIO_ReadPin(HALL_GPIO_PORT, HALL_B_PIN) != GPIO_PIN_RESET) {
        hall |= 0x02;
    }
    if (HAL_GPIO_ReadPin(HALL_GPIO_PORT, HALL_C_PIN) != GPIO_PIN_RESET) {
        hall |= 0x01;
    }
    return hall;
}

/* 换相处理：在霍尔 EXTI 中断或主循环高频轮询中调用 */
void BLDC_Commutation(void)
{
    uint8_t hall = BLDC_Read_Hall();
    uint8_t step;

    if (hall == last_hall) {
        return;                     /* 状态未变化，无需换相 */
    }
    last_hall = hall;

    step = hall_to_step[hall];
    if (step >= 6) {
        return;                     /* 无效霍尔状态(000/111)，保持当前输出 */
    }

    hall_change_count++;

    if (direction == BLDC_DIR_REVERSE) {
        /* 反转：交换 U、V 两相即可反向 */
        CommutationStep rev = comm_table[step];
        PhaseState tmp = rev.u;
        rev.u = rev.v;
        rev.v = tmp;
        BLDC_Apply_Step(&rev);
    } else {
        BLDC_Apply_Step(&comm_table[step]);
    }
}

/* 测速：周期性调用，interval_sec 为两次调用之间的实际间隔（秒） */
float BLDC_Get_RPM(float interval_sec)
{
    float rpm;

    if (interval_sec <= 0.0f) {
        interval_sec = 0.001f;
    }

    /* 机械转速(RPM) = 电转速 / 极对数
       电转速(转/秒) = (霍尔变化次数 / 6) / 间隔秒 */
    rpm = ((float)hall_change_count / 6.0f) /
          (float)MOTOR_POLE_PAIRS / interval_sec * 60.0f;

    hall_change_count = 0;          /* 计算后清零，重新累计 */
    return rpm;
}

/* 霍尔变化累计次数 */
uint32_t BLDC_Get_Hall_Count(void)
{
    return hall_change_count;
}
