//使用说明
//首先打开USART中断，和进行DMA开始
//主函数初始  Usart1_init();  Usart1_ReadData();
//对用户自己使用中断回调函数（在it.c中void USART1_IRQHandler(void)调用）以下函数
//HAL_UART_IdleCallback(UART_HandleTypeDef *UartHandle);
//对Usart自行改变配置（usart1，2，3...)
//在需要发送调用Usart1_SendData（）；
//在需要接收调用user_uart_process（）；
//
//
#include "headfile.h"
#define RX_TIMEOUT_MS 30
uint32_t last_rx_time = 0;

void UserUsart1_CheckRxTimeout(void) {
    if(HAL_GetTick() - last_rx_time > RX_TIMEOUT_MS) {
        HAL_UART_AbortReceive(&huart1);
        Usart1_ReadData(); // 重新初始化接收
    }
}


// 发送缓冲区操作
uint8_t  usart1_SendBuf[USART1_BUF_SIZE+1];
uint8_t  c[USART1_BUF_SIZE+1];                                         // 发送缓存区
uint16_t usart1_SendLen= 0;                                                    // 发送数据长度
// 接收缓冲区操作
uint8_t  usart1_RecvBuf[USART1_BUF_SIZE+1];        // 接收数据环形缓冲区 
uint16_t usart1_RecvLen=0;                // 环形缓冲区的当前放入位置 
uint8_t  usart1_recvFrame = 0;                        // 1 接收一个完整数据包. 0:NO


void Usart1_init(void)
{
  usart1_RecvLen=0;                                                                          // 清除标志
  usart1_recvFrame=0;
  memset(usart1_RecvBuf,0,USART1_BUF_SIZE);     
  
  usart1_RecvLen = 0;
  memset(usart1_SendBuf,0,USART1_BUF_SIZE);  
}




// 通过DMA方式，直接发送数据，注意发送数据不能够超过缓冲区长度
// 注意数据不要溢出
uint8_t Usart1_SendData(uint8_t *buf, uint16_t Size)   
{
  if(__HAL_DMA_GET_COUNTER(huart1.hdmatx) == 0 )                                // 检查上次数据是否发送完成   
  {
    if(Size>USART1_BUF_SIZE)
      Size = USART1_BUF_SIZE;

    for(int i=0; i<Size; i++)
      usart1_SendBuf[i] = buf[i];                                                // 发送缓存区
    usart1_SendLen= Size;                                                        // 发送数据长度
    HAL_UART_DMAStop(&huart1);                                                  // 关闭DMA
    HAL_UART_Transmit_DMA(&huart1,usart1_SendBuf,usart1_SendLen);                 // 启动DMA发送 
    return 1;
  }
  else
	{
			 // 停止串口
       HAL_UART_DeInit(&huart1);
				MX_USART1_UART_Init();
//		   HAL_UART_DMAStop(&huart1);                                                  // 关闭DMA
//       HAL_UART_Transmit_DMA(&huart1,usart1_SendBuf,usart1_SendLen);                 // 启动DMA发
		   return 0;
  }
	}
   

// 发送完成中断，打开接收
void USART1_EndTxd_IRQHandler(void)
{
  usart1_SendLen = 0;    // 发送完成
}


// 启动新的串口读取
void Usart1_ReadData(void)
{
  HAL_UART_AbortReceive_IT(&huart1);                                            // 初始化接收缓冲区
  usart1_RecvLen=0;                                                                          // 清除标志
  usart1_recvFrame=0;
  memset(usart1_RecvBuf,0,USART1_BUF_SIZE);                                       // BUFFER清除, 可以不用
  HAL_UART_Receive_IT(&huart1,usart1_RecvBuf,1);                                 // 打开中断，接收第一个数据
}


// 数据包第一个字节，串口中断接收
void USART1_RXD_IRQHandler(void)
{
  HAL_UART_Receive_DMA(&huart1, usart1_RecvBuf+1, USART1_BUF_SIZE-1);             //打开DMA接收,数据放在g_USART1_DMA_RX_Buffer
  __HAL_UART_CLEAR_IDLEFLAG(&huart1);                                           //清除标志位
  __HAL_UART_ENABLE_IT(&huart1, UART_IT_IDLE);                                  //使能IDLE中断
}

// 串口DMA空闲中断接收

void USART1_IDLE_IRQHandler(void)
{
  uint32_t temp1;
  if (__HAL_UART_GET_FLAG(&huart1, UART_FLAG_IDLE) != RESET)                    //获取IDLE标志位,检查idle标志是否被置位
  {
    __HAL_UART_CLEAR_IDLEFLAG(&huart1);                                         //清除标志位
    HAL_UART_DMAStop(&huart1); 
    temp1 = USART1_BUF_SIZE - __HAL_DMA_GET_COUNTER(huart1.hdmarx);
    if(usart1_recvFrame==0 && temp1>0 && temp1<USART1_BUF_SIZE) 
    {
      usart1_RecvLen =  temp1;                                             //总计数减去未传输的数据个数，得到已经接收的数据个数
      usart1_recvFrame = 1;	                                                // 接受完成标志位置1
    }
  }
}

uint8_t game_times = 0;
uint8_t uart_flag;
//执行命令函数
void user_uart_process(void)
{
  if(usart1_recvFrame==1 && !uart_flag)  // UART1 接收到数据，进行数据解析，解析完成后，将数据通过Can1发送出去
  {
      uart_flag = 1;
    if(usart1_RecvLen > 5)//接收数据大小限制
    {   //取棋
        BJMotor_distant(1,20);
        BJMotor_distant(2,-5.5);
        HAL_Delay(18000);
        BJMotor_distant(3,7);
        
        HAL_Delay(5000);
        TIM5->CCR2 = 250;
        HAL_Delay(1000);
        BJMotor_distant(3,-7);
        HAL_Delay(2000);
        //下棋
        BJMotor_distant(1,-28.8);
        BJMotor_distant(2,-3.5);
        HAL_Delay(25000);
        BJMotor_distant(3,7);
        HAL_Delay(5000);
        TIM5->CCR2 = 0;
        HAL_Delay(1000);
        BJMotor_distant(3,-7);
        HAL_Delay(4000);
        //看棋
        BJMotor_distant(1,8.8);
        BJMotor_distant(2,9);  
    }
    if(usart1_RecvLen==5)//接收数据大小限制
    {
        
        if(usart1_RecvBuf[0] == 'A' && usart1_RecvBuf[2] == ',' )
        {
            Usart1_SendData(usart1_RecvBuf,sizeof(usart1_RecvBuf)); 
            game_times %=5;
            switch(game_times)
            {

            case 0:
                QZ_points(1);
                game_times++;
                break;
            case 1:
                QZ_points(2);
                game_times++;
                break;
            case 2:
                QZ_points(3);
                game_times++;
                break;
            case 3:
                QZ_points(4);
                game_times++;
                break;
            case 4:
                QZ_points(5);
                game_times++;
                break;
            default:
                break;
            }
            //你赢了
            if(usart1_RecvBuf[1] == '3' && usart1_RecvBuf[3] == '0')
            {
                HAL_UART_Transmit(&huart2, (uint8_t *)"000", 3, 50);
            }//我赢了
            if(usart1_RecvBuf[1] == '3' && usart1_RecvBuf[3] == '1')
            {
                HAL_UART_Transmit(&huart2, (uint8_t *)"111", 3, 50);
            }//平局了
            if(usart1_RecvBuf[1] == '3' && usart1_RecvBuf[3] == '2')
            {
                HAL_UART_Transmit(&huart2, (uint8_t *)"222", 3, 50);
            }//该你下了
            if(usart1_RecvBuf[1] == '3' && usart1_RecvBuf[3] == '3')
            {
                HAL_UART_Transmit(&huart2, (uint8_t *)"333", 3, 50);
            }
        }

    }
    usart1_recvFrame =0;
    Usart1_ReadData();//再次开启中断，方便下次接收
    uart_flag = 0;
  }
}



/*******************************************************************************
此回调函数中，每次接收到的字节 xxx_Rxd_Buf[2]
因为在调用 HAL_UART_IRQHandler(&huart5);时，调用了 UART_Receive_IT（），其中
执行了 __HAL_UART_DISABLE_IT(huart, UART_IT_RXNE)，所以接收中断必须在回调函
数中重新开启，最简单的就是重复调用 HAL_UART_Receive_IT(&huart,dbg_Rxd_Buf,1)了
*******************************************************************************/
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *UartHandle)
{
  if(UartHandle->Instance == USART1)
    USART1_RXD_IRQHandler();   

}


// UART发送完成中断调用。 UART_DMATransmitCplt  和 UART_EndTransmit_IT 调用
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *UartHandle)
{
  if(UartHandle->Instance == USART1)
    USART1_EndTxd_IRQHandler();      
}


// UART空闲中断
void HAL_UART_IdleCallback(UART_HandleTypeDef *UartHandle)
{
  if(UartHandle->Instance == USART1)
	{
    USART1_IDLE_IRQHandler();  
		 last_rx_time= HAL_GetTick(); // 每次收到数据更新时间戳
	}		
}








