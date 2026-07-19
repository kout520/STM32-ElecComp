/**
 ****************************************************************************************************
 * @file        atk_mc5640_dcmi.c
 * @author      正点原子团队(ALIENTEK)
 * @version     V1.0
 * @date        2022-06-21
 * @brief       ATK-MC5640模块DCMI接口驱动代码
 * @license     Copyright (c) 2020-2032, 广州市星翼电子科技有限公司
 ****************************************************************************************************
 * @attention
 *
 * 实验平台:正点原子 探索者 F407开发板
 * 在线视频:www.yuanzige.com
 * 技术论坛:www.openedv.com
 * 公司网址:www.alientek.com
 * 购买地址:openedv.taobao.com
 *
 ****************************************************************************************************
 */

#include "./BSP/ATK_MC5640/atk_mc5640_dcmi.h"

/* ATK-MC5640模块DCMI接口数据结构体 */
static struct
{
    DCMI_HandleTypeDef dcmi;
    uint8_t frame_sem;
} g_atk_mc5640_dcmi_sta = {0};


/**
 * @brief       ATK-MC5640模块DCMI接口中断服务函数
 * @param       无
 * @retval      无
 */
void ATK_MC5640_DCMI_IRQHandler(void)
{
     HAL_DCMI_IRQHandler(&g_atk_mc5640_dcmi_sta.dcmi);
}


/**
 * @brief       ATK-MC5640模块DCMI接口DMA中断服务函数
 * @param       无
 * @retval      无
 */
void ATK_MC5640_DCMI_DMA_IRQHandler(void)
{
    HAL_DMA_IRQHandler(g_atk_mc5640_dcmi_sta.dcmi.DMA_Handle);
}

/**
 * @brief       ATK-MC5640模块DCMI接口帧中断回调函数
 * @param       无
 * @retval      无
 */
void HAL_DCMI_FrameEventCallback(DCMI_HandleTypeDef *hdcmi)
{
    if (hdcmi == &g_atk_mc5640_dcmi_sta.dcmi)
    {
        g_atk_mc5640_dcmi_sta.frame_sem = 1;
    }
}

/**
 * @brief       DCMI底层初始化
 * @param       无
 * @retval      无
 */
void HAL_DCMI_MspInit(DCMI_HandleTypeDef* hdcmi)
{
    GPIO_InitTypeDef gpio_init_struct = {0};
    static DMA_HandleTypeDef dma_handle = {0};
    
    if (hdcmi == &g_atk_mc5640_dcmi_sta.dcmi)
    {
        /* 使能时钟 */
        ATK_MC5640_DCMI_CLK_ENABLE();
        ATK_MC5640_DCMI_DMA_CLK_ENABLE();
        ATK_MC5640_DCMI_VSYNC_GPIO_CLK_ENABLE();
        ATK_MC5640_DCMI_HSYNC_GPIO_CLK_ENABLE();
        ATK_MC5640_DCMI_PIXCLK_GPIO_CLK_ENABLE();
        ATK_MC5640_DCMI_D0_GPIO_CLK_ENABLE();
        ATK_MC5640_DCMI_D1_GPIO_CLK_ENABLE();
        ATK_MC5640_DCMI_D2_GPIO_CLK_ENABLE();
        ATK_MC5640_DCMI_D3_GPIO_CLK_ENABLE();
        ATK_MC5640_DCMI_D4_GPIO_CLK_ENABLE();
        ATK_MC5640_DCMI_D5_GPIO_CLK_ENABLE();
        ATK_MC5640_DCMI_D6_GPIO_CLK_ENABLE();
        ATK_MC5640_DCMI_D7_GPIO_CLK_ENABLE();
        
        /* 初始化VSYNC引脚 */
        gpio_init_struct.Pin        = ATK_MC5640_DCMI_VSYNC_GPIO_PIN;
        gpio_init_struct.Mode       = GPIO_MODE_AF_PP;
        gpio_init_struct.Pull       = GPIO_PULLUP;
        gpio_init_struct.Speed      = GPIO_SPEED_FREQ_HIGH;
        gpio_init_struct.Alternate  = ATK_MC5640_DCMI_VSYNC_GPIO_AF;
        HAL_GPIO_Init(ATK_MC5640_DCMI_VSYNC_GPIO_PORT, &gpio_init_struct);
        
        /* 初始化HSYNC引脚 */
        gpio_init_struct.Pin        = ATK_MC5640_DCMI_HSYNC_GPIO_PIN;
        gpio_init_struct.Mode       = GPIO_MODE_AF_PP;
        gpio_init_struct.Pull       = GPIO_PULLUP;
        gpio_init_struct.Speed      = GPIO_SPEED_FREQ_HIGH;
        gpio_init_struct.Alternate  = ATK_MC5640_DCMI_HSYNC_GPIO_AF;
        HAL_GPIO_Init(ATK_MC5640_DCMI_HSYNC_GPIO_PORT, &gpio_init_struct);
        
        /* 初始化PIXCLK引脚 */
        gpio_init_struct.Pin        = ATK_MC5640_DCMI_PIXCLK_GPIO_PIN;
        gpio_init_struct.Mode       = GPIO_MODE_AF_PP;
        gpio_init_struct.Pull       = GPIO_PULLUP;
        gpio_init_struct.Speed      = GPIO_SPEED_FREQ_HIGH;
        gpio_init_struct.Alternate  = ATK_MC5640_DCMI_PIXCLK_GPIO_AF;
        HAL_GPIO_Init(ATK_MC5640_DCMI_PIXCLK_GPIO_PORT, &gpio_init_struct);
        
        /* 初始化D0引脚 */
        gpio_init_struct.Pin        = ATK_MC5640_DCMI_D0_GPIO_PIN;
        gpio_init_struct.Mode       = GPIO_MODE_AF_PP;
        gpio_init_struct.Pull       = GPIO_PULLUP;
        gpio_init_struct.Speed      = GPIO_SPEED_FREQ_HIGH;
        gpio_init_struct.Alternate  = ATK_MC5640_DCMI_D0_GPIO_AF;
        HAL_GPIO_Init(ATK_MC5640_DCMI_D0_GPIO_PORT, &gpio_init_struct);
        
        /* 初始化D1引脚 */
        gpio_init_struct.Pin        = ATK_MC5640_DCMI_D1_GPIO_PIN;
        gpio_init_struct.Mode       = GPIO_MODE_AF_PP;
        gpio_init_struct.Pull       = GPIO_PULLUP;
        gpio_init_struct.Speed      = GPIO_SPEED_FREQ_HIGH;
        gpio_init_struct.Alternate  = ATK_MC5640_DCMI_D1_GPIO_AF;
        HAL_GPIO_Init(ATK_MC5640_DCMI_D1_GPIO_PORT, &gpio_init_struct);
        
        /* 初始化D2引脚 */
        gpio_init_struct.Pin        = ATK_MC5640_DCMI_D2_GPIO_PIN;
        gpio_init_struct.Mode       = GPIO_MODE_AF_PP;
        gpio_init_struct.Pull       = GPIO_PULLUP;
        gpio_init_struct.Speed      = GPIO_SPEED_FREQ_HIGH;
        gpio_init_struct.Alternate  = ATK_MC5640_DCMI_D2_GPIO_AF;
        HAL_GPIO_Init(ATK_MC5640_DCMI_D2_GPIO_PORT, &gpio_init_struct);
        
        /* 初始化D3引脚 */
        gpio_init_struct.Pin        = ATK_MC5640_DCMI_D3_GPIO_PIN;
        gpio_init_struct.Mode       = GPIO_MODE_AF_PP;
        gpio_init_struct.Pull       = GPIO_PULLUP;
        gpio_init_struct.Speed      = GPIO_SPEED_FREQ_HIGH;
        gpio_init_struct.Alternate  = ATK_MC5640_DCMI_D3_GPIO_AF;
        HAL_GPIO_Init(ATK_MC5640_DCMI_D3_GPIO_PORT, &gpio_init_struct);
        
        /* 初始化D4引脚 */
        gpio_init_struct.Pin        = ATK_MC5640_DCMI_D4_GPIO_PIN;
        gpio_init_struct.Mode       = GPIO_MODE_AF_PP;
        gpio_init_struct.Pull       = GPIO_PULLUP;
        gpio_init_struct.Speed      = GPIO_SPEED_FREQ_HIGH;
        gpio_init_struct.Alternate  = ATK_MC5640_DCMI_D4_GPIO_AF;
        HAL_GPIO_Init(ATK_MC5640_DCMI_D4_GPIO_PORT, &gpio_init_struct);
        
        /* 初始化D5引脚 */
        gpio_init_struct.Pin        = ATK_MC5640_DCMI_D5_GPIO_PIN;
        gpio_init_struct.Mode       = GPIO_MODE_AF_PP;
        gpio_init_struct.Pull       = GPIO_PULLUP;
        gpio_init_struct.Speed      = GPIO_SPEED_FREQ_HIGH;
        gpio_init_struct.Alternate  = ATK_MC5640_DCMI_D5_GPIO_AF;
        HAL_GPIO_Init(ATK_MC5640_DCMI_D5_GPIO_PORT, &gpio_init_struct);
        
        /* 初始化D6引脚 */
        gpio_init_struct.Pin        = ATK_MC5640_DCMI_D6_GPIO_PIN;
        gpio_init_struct.Mode       = GPIO_MODE_AF_PP;
        gpio_init_struct.Pull       = GPIO_PULLUP;
        gpio_init_struct.Speed      = GPIO_SPEED_FREQ_HIGH;
        gpio_init_struct.Alternate  = ATK_MC5640_DCMI_D6_GPIO_AF;
        HAL_GPIO_Init(ATK_MC5640_DCMI_D6_GPIO_PORT, &gpio_init_struct);
        
        /* 初始化D7引脚 */
        gpio_init_struct.Pin        = ATK_MC5640_DCMI_D7_GPIO_PIN;
        gpio_init_struct.Mode       = GPIO_MODE_AF_PP;
        gpio_init_struct.Pull       = GPIO_PULLUP;
        gpio_init_struct.Speed      = GPIO_SPEED_FREQ_HIGH;
        gpio_init_struct.Alternate  = ATK_MC5640_DCMI_D7_GPIO_AF;
        HAL_GPIO_Init(ATK_MC5640_DCMI_D7_GPIO_PORT, &gpio_init_struct);
        
        /* 初始化DMA */
        dma_handle.Instance                 = ATK_MC5640_DCMI_DMA_INTERFACE;
        dma_handle.Init.Channel             = ATK_MC5640_DCMI_DMA_CHANNEL;
        dma_handle.Init.Direction           = DMA_PERIPH_TO_MEMORY;
        dma_handle.Init.PeriphInc           = DMA_PINC_DISABLE;
        dma_handle.Init.MemInc              = DMA_MINC_DISABLE;
        dma_handle.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
        dma_handle.Init.MemDataAlignment    = DMA_MDATAALIGN_HALFWORD;
        dma_handle.Init.Mode                = DMA_CIRCULAR;
        dma_handle.Init.Priority            = DMA_PRIORITY_HIGH;
        dma_handle.Init.FIFOMode            = DMA_FIFOMODE_ENABLE;
        dma_handle.Init.FIFOThreshold       = DMA_FIFO_THRESHOLD_HALFFULL;
        dma_handle.Init.MemBurst            = DMA_MBURST_SINGLE;
        dma_handle.Init.PeriphBurst         = DMA_PBURST_SINGLE;
        __HAL_LINKDMA(hdcmi, DMA_Handle, dma_handle);
        HAL_DMA_Init(&dma_handle);
        
        /* 配置DCMI中断 */
        HAL_NVIC_SetPriority(ATK_MC5640_DCMI_IRQn, 2, 2);
        HAL_NVIC_EnableIRQ(ATK_MC5640_DCMI_IRQn);
        
        /* 配置DMA中断 */
        HAL_NVIC_SetPriority(ATK_MC5640_DCMI_DMA_IRQn, 2, 3);
        HAL_NVIC_EnableIRQ(ATK_MC5640_DCMI_DMA_IRQn);
    }
}

/**
 * @brief       初始化ATK-MC5640模块DCMI接口
 * @param       无
 * @retval      无
 */
void atk_mc5640_dcmi_init(void)
{
    g_atk_mc5640_dcmi_sta.dcmi.Instance                 = ATK_MC5640_DCMI_INTERFACE;
    g_atk_mc5640_dcmi_sta.dcmi.Init.SynchroMode         = DCMI_SYNCHRO_HARDWARE;
    g_atk_mc5640_dcmi_sta.dcmi.Init.PCKPolarity         = DCMI_PCKPOLARITY_RISING;
    g_atk_mc5640_dcmi_sta.dcmi.Init.VSPolarity          = DCMI_VSPOLARITY_LOW;
    g_atk_mc5640_dcmi_sta.dcmi.Init.HSPolarity          = DCMI_HSPOLARITY_LOW;
    g_atk_mc5640_dcmi_sta.dcmi.Init.CaptureRate         = DCMI_CR_ALL_FRAME;
    g_atk_mc5640_dcmi_sta.dcmi.Init.ExtendedDataMode    = DCMI_EXTEND_DATA_8B;
    g_atk_mc5640_dcmi_sta.dcmi.Init.JPEGMode            = DCMI_JPEG_DISABLE;
    HAL_DCMI_Init(&g_atk_mc5640_dcmi_sta.dcmi);
}

/**
 * @brief       开始ATK-MC5640模块DCMI接口DMA传输
 * @param       dts_addr        : 帧数据的接收目的地址
 *              meminc          : DMA_MINC_DISABLE: 帧数据接收的目的地址自动增加
 *                                DMA_MINC_ENABLE : 帧数据接收的目的地址不自动增加
 *              memdataalignment: DMA_MDATAALIGN_BYTE    : 帧数据接收缓冲的位宽为8比特
 *                                DMA_MDATAALIGN_HALFWORD: 帧数据接收缓冲的位宽为16比特
 *                                DMA_MDATAALIGN_WORD    : 帧数据接收缓冲的位宽为32比特
 *              len             : 传输的帧数据大小
 * @retval      无
 */
void atk_mc5640_dcmi_start(uint32_t dts_addr, uint32_t meminc, uint32_t memdataalignment, uint32_t len)
{
    /* 根据需求重新配置DMA */
    g_atk_mc5640_dcmi_sta.dcmi.DMA_Handle->Init.MemInc = meminc;
    g_atk_mc5640_dcmi_sta.dcmi.DMA_Handle->Init.MemDataAlignment = memdataalignment;
    HAL_DMA_Init(g_atk_mc5640_dcmi_sta.dcmi.DMA_Handle);
    
    /* 清空帧接收完成标记
     * 使能DCMI帧接收中断
     */
    g_atk_mc5640_dcmi_sta.frame_sem = 0;
    __HAL_DCMI_ENABLE_IT(&g_atk_mc5640_dcmi_sta.dcmi, DCMI_IT_FRAME);
    HAL_DCMI_Start_DMA(&g_atk_mc5640_dcmi_sta.dcmi, DCMI_MODE_SNAPSHOT, dts_addr, len);
    
    /* 等待传输完成 */
    while (g_atk_mc5640_dcmi_sta.frame_sem == 0);
    HAL_DCMI_Stop(&g_atk_mc5640_dcmi_sta.dcmi);
}
