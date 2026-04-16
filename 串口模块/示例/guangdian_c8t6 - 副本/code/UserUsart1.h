#ifndef __USER_USART1_H__
#define __USER_USART1_H__

#define  USART1_BUF_SIZE    256                            // DMA缓冲区长度
//用户自己中断回调函数（在it.c中void USART1_IRQHandler(void)调用）
extern void HAL_UART_IdleCallback(UART_HandleTypeDef *UartHandle);
// 发送缓冲区操作
extern uint8_t  usart1_SendBuf[USART1_BUF_SIZE+1];                                         // 发送缓存区
extern uint16_t usart1_SendLen;                                                    // 发送数据长度
// 接收缓冲区操作
extern uint8_t  usart1_RecvBuf[USART1_BUF_SIZE+1];        // 接收数据环形缓冲区 
extern uint16_t usart1_RecvLen;                // 环形缓冲区的当前放入位置 
extern uint8_t  usart1_recvFrame;                        // 1 接收一个完整数据包. 0:NO

extern void Usart1_init(void);

// 通过DMA方式，直接发送数据，注意发送数据不能够超过缓冲区长度
extern uint8_t Usart1_SendData(uint8_t *buf, uint16_t Size);   
// 发送完成中断，打开接收
extern void USART1_EndTxd_IRQHandler(void);

// 启动新的串口读取
extern void Usart1_ReadData(void);
// 从串口收到数据，存入缓存
extern void USART1_RXD_IRQHandler(void);
extern void USART1_IDLE_IRQHandler(void);


//命令执行
extern void user_uart_process(void);


#endif 




