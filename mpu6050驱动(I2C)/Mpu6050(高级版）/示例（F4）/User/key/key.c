


#include "stm32f4xx.h"
#include "key.h"

/*
*--------------------------------------------------------------------------------------------------------
* Function:       key_init
* Description:    
* Input:          none
* Output:         none
* Return:         none
* Created by:     alvan 
* Created date:   2014-07-29
* Others:        	
*---------------------------------------------------------------------------------------------------------
*/
void key_init(void)
{ 
    GPIO_InitTypeDef GPIO_InitStructure;
    
    /* 使能端口A和E时钟 */
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOE, ENABLE);  
    
    /* 配置PA0引脚 */
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_DOWN;  // 下拉输入
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    /* 配置PE10引脚 */
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_DOWN;  // 也可以根据实际电路选择上拉或下拉
    GPIO_Init(GPIOE, &GPIO_InitStructure);
    
    /* 配置PE11引脚 */
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_11;
    GPIO_Init(GPIOE, &GPIO_InitStructure);
    
    /* 配置PE12引脚 */
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_12;
    GPIO_Init(GPIOE, &GPIO_InitStructure);
    
    /* 配置PE13引脚 */
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_13;
    GPIO_Init(GPIOE, &GPIO_InitStructure);
    
    /* 配置PE14引脚 */
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_14;
    GPIO_Init(GPIOE, &GPIO_InitStructure);
}

uint8_t key_get(void)
{
    // 检测PE10
    if(GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_10) == 0)
        return 1;
    
    // 检测PE11
    if(GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_11) == 0)
        return 2;
    
    // 检测PE12
    if(GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_12) == 0)
        return 3;
    
    // 检测PE13
    if(GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_13) == 0)
        return 4;
    
    // 检测PE14
    if(GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_14) == 0)
        return 5;
    
    // 检测PA0
    if(GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0) == 1)
        return 6;
    
    // 无按键按下
    return 0;
}
