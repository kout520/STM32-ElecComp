include "headfile.h"
/**********************************************************************************************************/
//接收"@(122.5)\r\n"形式
// USART1接收数据结构
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
//接收"@(255,122)\r\n"形式
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
 if(huart->Instance == USART2) {  // 注意：此处应与实际USART端口一致
        uint8_t byte = usart2_rx_buf[usart2_rx_index]; // 获取接收到的字节

        /*----- 调试输出（可选）-----*/
        // printf("[RAW] 0x%02X '%c'\n", byte, isprint(byte) ? byte : '.');

        /*----- 边界检查 -----*/
        if(usart2_rx_index >= USART2_RX_BUF_SIZE - 1) {
            // 缓冲区即将溢出，重置状态机
            usart2_parse_state = USART2_WAIT_AT;
            usart2_rx_index = 0;
            HAL_UART_Transmit(&huart2, (uint8_t*)"ERR:OVF\r\n", 9, 100);
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
                        HAL_UART_Transmit(&huart2, (uint8_t*)"ERR:X_FMT\r\n", 11, 100);
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
                        HAL_UART_Transmit(&huart2, (uint8_t*)"ERR:Y_FMT\r\n", 11, 100);
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
//                    HAL_UART_Transmit(&huart2, (uint8_t*)"[OK] ", 5, 100);
//                    HAL_UART_Transmit(&huart2, usart2_rx_buf, usart2_rx_index, 100);
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
            HAL_UART_Receive_IT(&huart2, &usart2_rx_buf[0], 1);
        } else {
            // 正常状态下继续接收下一个字节
            HAL_UART_Receive_IT(&huart2, &usart2_rx_buf[usart2_rx_index], 1);
        }
    }
}

//在主函数中调用
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