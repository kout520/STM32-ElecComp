#ifndef __UART_IT_H__
#define __UART_IT_H__

void User_UART_Process(void);

#define RX_BUF_SIZE 32
extern  uint8_t uart_rx_buf[RX_BUF_SIZE];
#define USART2_RX_BUF_SIZE 48
extern uint8_t usart2_rx_buf[USART2_RX_BUF_SIZE];


#endif

