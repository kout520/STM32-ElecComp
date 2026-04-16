#include "headfile.h"

int8_t oled_flag=0;
#define RX_BUF_SIZE 32  // 缓冲区大小（根据需求调整）

char rx_buf[32];  // 接收缓冲区
uint8_t rx_index = 0;      // 缓冲区索引
uint8_t coordinate_ready = 0;  // 坐标接收完成标志
int x = 0, y = 0;          // 解析后的坐标

uint16_t a,b,c1,d;
char text1[20];
uint8_t lcd_flag= 0;   
uint8_t distant_waring = 30;

// 接收状态机
typedef enum {
    WAIT_AT,
    WAIT_OPEN_BRACKET,
    RECEIVE_NUMBER,
    WAIT_CLOSE_BRACKET,
    WAIT_CR,
    WAIT_LF
} UART_ParseState_t;

// 接收缓冲区

uint8_t uart_rx_buf[RX_BUF_SIZE];
uint8_t uart_rx_index = 0;
UART_ParseState_t parse_state = WAIT_AT;
float received_yaw = 0.0f;
uint8_t data_ready = 0;


/**********************************************************************************************************/
// USART2接收数据结构
typedef enum {
    USART2_WAIT_AT,
    USART2_WAIT_OPEN_BRACKET,
    USART2_RECEIVE_X,
    USART2_WAIT_COMMA,
    USART2_RECEIVE_Y,
    USART2_WAIT_CLOSE_BRACKET,
    USART2_WAIT_CR,
    USART2_WAIT_LF
} USART2_ParseState_t;

// 共享缓冲区

uint8_t usart2_rx_buf[USART2_RX_BUF_SIZE];
uint8_t usart2_rx_index = 0;
USART2_ParseState_t usart2_parse_state = USART2_WAIT_AT;
int received_x, received_y;
uint8_t usart2_data_ready;



void OLEDINIT(void)
{
    OLED_Clear();
    // 在 OLED 上显示“程序”
    //OLED_ShowStr(0,0,"->",2);
    
    OLED_ShowStr(0,0,"front:",1);

    //OLED_ShowStr(0,0,"  ",2);

    OLED_ShowStr(0,1,"left:",1);


    OLED_ShowStr(0,2,"angle:",1);
// 
//    OLED_ShowStr(0,3,"beind:",1);
    OLED_ShowStr(0,5,"point:",1);

    
}
void show(void)
{
    switch(lcd_flag)
    {
        case 0:
            OLED_show();
            break;
        case 1:
            break;
        
    }
}

void OLED_show(void)
{
    sprintf(text1,"%.1f   ",distant);
    OLED_ShowStr(64,0,(uint8_t *)text1,1);    
        sprintf(text1,"%.1f   ",distant1);
    OLED_ShowStr(64,1,(uint8_t *)text1,1);    
//        sprintf(text1,"%.1f   ",distant2);
//    OLED_ShowStr(64,2,(uint8_t *)text1,1);    
        sprintf(text1,"%.2f   ",received_yaw);
    OLED_ShowStr(64,2,(uint8_t *)text1,1);    
        sprintf(text1,"(%d,%d)   ",received_x ,received_y);
    OLED_ShowStr(0,6,(uint8_t *)text1,1);    
}




MotorState_t motor_state = MOTOR_MOVE;
uint16_t pwm_values = 50;
// 全局变量
uint32_t motor_timestamp = 0;
uint8_t pwm_flag = 0;

#define BUFFER_SIZE 36  //

typedef struct {
    uint16_t angle;     // PWm（50-250）
    uint32_t distance;  // 对应距离（cm）
} ScanData;

ScanData scanBuffer[BUFFER_SIZE];
uint8_t bufferIndex = 0;
void motor_scan_non_blocking(void) {
    switch (motor_state) {
        case MOTOR_MOVE:

            motor_state = MOTOR_WAIT;
            break;

        case MOTOR_WAIT:
                motor_state = MOTOR_MEASURE;

            break;

        case MOTOR_MEASURE:
            SR04_GetData();
            if(sr04_flag)
            {
                a = 10 * distant;
            b = 10 * distant1;
            c1 = 10 * distant2;
            d = 10 * distant3;
                // 测量距离并存入缓冲区
//                printf("@(%d,%d,%d,%d)\r\n",a,b,c1,d);
                motor_state = MOTOR_MOVE;
            }
            break;
    }
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{

  if (htim->Instance == TIM4)
  {
    motor_scan_non_blocking();
  }
}





void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
    if(huart->Instance == USART1) {
        uint8_t byte = uart_rx_buf[uart_rx_index];
        //调试
        //HAL_UART_Transmit(&huart1, &byte, 1, 10); // 10ms超时
        // 状态机解析
        switch(parse_state) {
            case WAIT_AT:
                if(byte == '@') {
                    parse_state = WAIT_OPEN_BRACKET;
                    uart_rx_index = 0;
                    uart_rx_buf[uart_rx_index++] = byte;
                }
                else {
                    data_ready = 2;
                }
                break;
                
            case WAIT_OPEN_BRACKET:
                if(byte == '(') {
                    parse_state = RECEIVE_NUMBER;
                    uart_rx_buf[uart_rx_index++] = byte;
                } else {
                    parse_state = WAIT_AT;
                }
                break;
                
            case RECEIVE_NUMBER:
                if(byte == ')') {
                    parse_state = WAIT_CR;
                    uart_rx_buf[uart_rx_index++] = byte;
                } else if((byte >= '0' && byte <= '9') || byte == '.' || byte == '-') {
                    if(uart_rx_index < RX_BUF_SIZE-2) {
                        uart_rx_buf[uart_rx_index++] = byte;
                    } else {
                        parse_state = WAIT_AT; // 缓冲区溢出
                    }
                } else {
                    parse_state = WAIT_AT; // 非法字符
                }
                break;
                
            case WAIT_CR:
                if(byte == '\r') {
                    parse_state = WAIT_LF;
                    uart_rx_buf[uart_rx_index++] = byte;
                } else {
                    parse_state = WAIT_AT;
                }
                break;
                
            case WAIT_LF:
                if(byte == '\n') {
                    uart_rx_buf[uart_rx_index++] = byte;
                    uart_rx_buf[uart_rx_index] = '\0'; // 添加字符串结束符
                    
                    // 提取数字部分
                    char num_str[16];
                    if(sscanf((char*)uart_rx_buf, "@(%f)\r\n", &received_yaw) == 1) {
                        data_ready = 1;
                    }
                }
                parse_state = WAIT_AT;
                break;
        }
        // 重启接收
        if(uart_rx_index < RX_BUF_SIZE) {
            HAL_UART_Receive_IT(&huart1, &uart_rx_buf[uart_rx_index], 1);
        } else {
            parse_state = WAIT_AT; // 缓冲区满时重置
            uart_rx_index = 0;
            HAL_UART_Receive_IT(&huart1, &uart_rx_buf[0], 1);
        }
    }
 if(huart->Instance == USART3) {  // 注意：此处应与实际USART端口一致
        uint8_t byte = usart2_rx_buf[usart2_rx_index]; // 获取接收到的字节

        /*----- 调试输出（可选）-----*/
        // printf("[RAW] 0x%02X '%c'\n", byte, isprint(byte) ? byte : '.');

        /*----- 边界检查 -----*/
        if(usart2_rx_index >= USART2_RX_BUF_SIZE - 1) {
            // 缓冲区即将溢出，重置状态机
            usart2_parse_state = USART2_WAIT_AT;
            usart2_rx_index = 0;
            HAL_UART_Transmit(&huart3, (uint8_t*)"ERR:OVF\r\n", 9, 100);
            goto RESTART_RECEIVE;
        }

        /*----- 状态机处理 -----*/
        switch(usart2_parse_state) {
            case USART2_WAIT_AT:
                if(byte == '@') {
                    usart2_rx_index = 0;  // 重置索引
                    usart2_rx_buf[usart2_rx_index++] = byte;
                    usart2_parse_state = USART2_WAIT_OPEN_BRACKET;
                }
                break;

            case USART2_WAIT_OPEN_BRACKET:
                usart2_rx_buf[usart2_rx_index++] = byte;
                if(byte == '(') {
                    usart2_parse_state = USART2_RECEIVE_X;
                } else {
                    usart2_parse_state = USART2_WAIT_AT; // 非法字符
                }
                break;

            case USART2_RECEIVE_X:
                usart2_rx_buf[usart2_rx_index++] = byte;
                if(byte == ',') {
                    // 安全提取X值（支持负数）
                    char *endptr;
                    char *x_start = strchr((char*)usart2_rx_buf, '(') + 1;
                    received_x = strtol(x_start, &endptr, 10);
                    if(*endptr != ',') {
                        HAL_UART_Transmit(&huart3, (uint8_t*)"ERR:X_FMT\r\n", 11, 100);
                        usart2_parse_state = USART2_WAIT_AT;
                        break;
                    }
                    usart2_parse_state = USART2_RECEIVE_Y;
                } else if(!isdigit(byte) && byte != '-') {
                    usart2_parse_state = USART2_WAIT_AT; // 非法字符
                }
                break;

            case USART2_RECEIVE_Y:
                usart2_rx_buf[usart2_rx_index++] = byte;
                if(byte == ')') {
                    // 安全提取Y值（支持负数）
                    char *endptr;
                    char *y_start = strchr((char*)usart2_rx_buf, ',') + 1;
                    received_y = strtol(y_start, &endptr, 10);
                    if(*endptr != ')') {
                        HAL_UART_Transmit(&huart3, (uint8_t*)"ERR:Y_FMT\r\n", 11, 100);
                    }
                    usart2_parse_state = USART2_WAIT_CR;
                } else if(!isdigit(byte) && byte != '-') {
                    usart2_parse_state = USART2_WAIT_AT; // 非法字符
                }
                break;

            case USART2_WAIT_CR:
                usart2_rx_buf[usart2_rx_index++] = byte;
                if(byte == '\r') {
                    usart2_parse_state = USART2_WAIT_LF;
                } else {
                    usart2_parse_state = USART2_WAIT_AT;
                }
                break;

            case USART2_WAIT_LF:
                usart2_rx_buf[usart2_rx_index++] = byte;
                if(byte == '\n') {
                    usart2_rx_buf[usart2_rx_index] = '\0'; // 确保字符串终止
                    usart2_data_ready = 1;
//                    // 调试输出完整报文
//                    HAL_UART_Transmit(&huart3, (uint8_t*)"[OK] ", 5, 100);
//                    HAL_UART_Transmit(&huart3, usart2_rx_buf, usart2_rx_index, 100);
                }
                usart2_parse_state = USART2_WAIT_AT;
                usart2_rx_index = 0;
                break;
        }

RESTART_RECEIVE:
        /*----- 重启接收 -----*/
        if(usart2_parse_state == USART2_WAIT_AT) {
            // 发生错误时从头开始接收
            usart2_rx_index = 0;
            HAL_UART_Receive_IT(&huart3, &usart2_rx_buf[0], 1);
        } else {
            // 正常状态下继续接收下一个字节
            HAL_UART_Receive_IT(&huart3, &usart2_rx_buf[usart2_rx_index], 1);
        }
    }
}
void User_UART_Process(void) {
    if(data_ready == 1) {
        // 1. 回传原始数据
        
        // 2. 打印解析结果（可选）
        printf("@(%d,%d,%.0f)\r\n",a,b, received_yaw);
        
        
        // 重置标志
        data_ready = 0;
        uart_rx_index = 0;
    }
    else if(data_ready == 2) {
        printf("worrg\r\n");
        received_yaw = 666.0f;
                // 重置标志
        data_ready = 0;
        uart_rx_index = 0;
    }
        // USART2数据处理
    if(usart2_data_ready) {
        // 模式1：回传原始数据
        
        // 模式2：格式化回传
//        char response[32];
//        int len = snprintf(response, sizeof(response), 
//                          "ACK@(%d,%d)\r\n", received_x, received_y);
//        HAL_UART_Transmit(&huart3, (uint8_t*)response, len, 100);
        
        usart2_data_ready = 0;
        usart2_rx_index = 0;
    }
}