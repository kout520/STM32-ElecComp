#ifndef __FUN_H
#define __FUN_H


void OLED_show(void);

void OLEDINIT(void);
void motor_scan_non_blocking(void);
void show(void);
void ParseCoordinate(void);
void mpu9250_Start(void);
void mpu9250go(void);
extern int8_t oled_flag;
extern uint16_t a,b,c,d;
extern uint8_t lcd_flag;
extern uint8_t distant_waring;
extern uint8_t coordinate_ready;  // 坐标接收完成标志
extern char rx_buf[32];  // 接收缓冲区;
typedef enum {
    MOTOR_MOVE,      // 舵机转动中
    MOTOR_WAIT,      // 等待舵机稳定
    MOTOR_MEASURE    // 触发测距
} MotorState_t;


#endif
