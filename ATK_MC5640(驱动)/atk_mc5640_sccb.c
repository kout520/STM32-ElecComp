/**
 ****************************************************************************************************
 * @file        atk_mc5640_sccb.c
 * @author      正点原子团队(ALIENTEK)
 * @version     V1.0
 * @date        2022-06-21
 * @brief       ATK-MC5640模块SCCB接口驱动代码
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

#include "./BSP/ATK_MC5640/atk_mc5640_sccb.h"
#include "./SYSTEM/delay/delay.h"

/* SCCB接口读写通讯地址bit0 */
#define ATK_MC5640_SCCB_WRITE   0x00
#define ATK_MC5640_SCCB_READ    0x01

#if (ATK_MC5640_SCCB_GPIO_PULLUP != 0)
/**
 * @brief       设置SCCB接口SDA引脚为输出模式
 * @param       无
 * @retval      无
 */
static void atk_mc5640_sccb_set_sda_output(void)
{
    GPIO_InitTypeDef gpio_init_struct = {0};
    
    gpio_init_struct.Pin    = ATK_MC5640_SCCB_SDA_GPIO_PIN;
    gpio_init_struct.Mode   = GPIO_MODE_OUTPUT_PP;
    gpio_init_struct.Pull   = GPIO_PULLUP;
    gpio_init_struct.Speed  = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(ATK_MC5640_SCCB_SDA_GPIO_PORT, &gpio_init_struct);
}

/**
 * @brief       设置SCCB接口SDA引脚为输入模式
 * @param       无
 * @retval      无
 */
static void atk_mc5640_sccb_set_sda_input(void)
{
    GPIO_InitTypeDef gpio_init_struct = {0};
    
    gpio_init_struct.Pin    = ATK_MC5640_SCCB_SDA_GPIO_PIN;
    gpio_init_struct.Mode   = GPIO_MODE_INPUT;
    gpio_init_struct.Pull   = GPIO_PULLUP;
    gpio_init_struct.Speed  = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(ATK_MC5640_SCCB_SDA_GPIO_PORT, &gpio_init_struct);
}
#endif

/**
 * @brief       SCCB接口延时函数
 * @param       无
 * @retval      无
 */
static inline void atk_mc5640_sccb_delay(void)
{
    delay_us(5);
}

/**
 * @brief       SCCB接口起始信号
 * @param       无
 * @retval      无
 */
static void atk_mc5640_sccb_start(void)
{
    ATK_MC5640_SCCB_SDA(1);
    ATK_MC5640_SCCB_SCL(1);
    atk_mc5640_sccb_delay();
    ATK_MC5640_SCCB_SDA(0);
    atk_mc5640_sccb_delay();
    ATK_MC5640_SCCB_SCL(0);
}

/**
 * @brief       SCCB接口停止信号
 * @param       无
 * @retval      无
 */
static void atk_mc5640_sccb_stop(void)
{
    ATK_MC5640_SCCB_SDA(0);
    atk_mc5640_sccb_delay();
    ATK_MC5640_SCCB_SCL(1);
    atk_mc5640_sccb_delay();
    ATK_MC5640_SCCB_SDA(1);
    atk_mc5640_sccb_delay();
}

/**
 * @brief       SCCB接口主机向从机写入一个字节数据
 * @param       dat: 待写入的一个字节数据
 * @retval      无
 */
static void atk_mc5640_sccb_write_byte(uint8_t dat)
{
    int8_t dat_index;
    uint8_t dat_bit;
    
    for (dat_index=7; dat_index>=0; dat_index--)
    {
        dat_bit = (dat >> dat_index) & 0x01;
        ATK_MC5640_SCCB_SDA(dat_bit);
        atk_mc5640_sccb_delay();
        ATK_MC5640_SCCB_SCL(1);
        atk_mc5640_sccb_delay();
        ATK_MC5640_SCCB_SCL(0);
    }
    
    ATK_MC5640_SCCB_SDA(1);
    atk_mc5640_sccb_delay();
    ATK_MC5640_SCCB_SCL(1);
    atk_mc5640_sccb_delay();
    ATK_MC5640_SCCB_SCL(0);
}

/**
 * @brief       SCCB接口主机向从机读取一个字节数据
 * @param       dat: 读取到的一个字节数据
 * @retval      无
 */
static void atk_mc5640_sccb_read_byte(uint8_t *dat)
{
    int8_t dat_index;
    uint8_t dat_bit;
    
#if (ATK_MC5640_SCCB_GPIO_PULLUP != 0)
    atk_mc5640_sccb_set_sda_input();
#endif
    
    for (dat_index=7; dat_index>=0; dat_index--)
    {
        atk_mc5640_sccb_delay();
        ATK_MC5640_SCCB_SCL(1);
        dat_bit = ATK_MC5640_SCCB_READ_SDA();
        *dat |= (dat_bit << dat_index);
        atk_mc5640_sccb_delay();
        ATK_MC5640_SCCB_SCL(0);
    }
    
    atk_mc5640_sccb_delay();
    ATK_MC5640_SCCB_SCL(1);
    atk_mc5640_sccb_delay();
    ATK_MC5640_SCCB_SCL(0);
    atk_mc5640_sccb_delay();
    ATK_MC5640_SCCB_SDA(0);
    atk_mc5640_sccb_delay();
    
#if (ATK_MC5640_SCCB_GPIO_PULLUP != 0)
    atk_mc5640_sccb_set_sda_output();
#endif
}

/**
 * @brief       ATK-MC5640 SCCB接口初始化
 * @param       无
 * @retval      无
 */
void atk_mc5640_sccb_init(void)
{
    GPIO_InitTypeDef gpio_init_struct = {0};
    
    /* 使能SCL、SDA引脚GPIO时钟 */
    ATK_MC5640_SCCB_SCL_GPIO_CLK_ENABLE();
    ATK_MC5640_SCCB_SDA_GPIO_CLK_ENABLE();
    
    /* 初始化SCL引脚 */
    gpio_init_struct.Pin    = ATK_MC5640_SCCB_SCL_GPIO_PIN;
    gpio_init_struct.Mode   = GPIO_MODE_OUTPUT_PP;
    gpio_init_struct.Pull   = GPIO_PULLUP;
    gpio_init_struct.Speed  = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(ATK_MC5640_SCCB_SCL_GPIO_PORT, &gpio_init_struct);
    
    /* 初始化SDA引脚 */
    gpio_init_struct.Pin    = ATK_MC5640_SCCB_SDA_GPIO_PIN;
#if (ATK_MC5640_SCCB_GPIO_PULLUP != 0)
    gpio_init_struct.Mode   = GPIO_MODE_OUTPUT_PP;
#else
    gpio_init_struct.Mode   = GPIO_MODE_OUTPUT_OD;
#endif
    gpio_init_struct.Pull   = GPIO_PULLUP;
    gpio_init_struct.Speed  = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(ATK_MC5640_SCCB_SDA_GPIO_PORT, &gpio_init_struct);
    
    atk_mc5640_sccb_stop();
}

/**
 * @brief       SCCB接口3阶段写传输
 * @param       id_addr : ID Address
 *              sub_addr: Sub-address
 *              dat     : Write Data
 * @retval      无
 */
void atk_mc5640_sccb_3_phase_write(uint8_t id_addr, uint16_t sub_addr, uint8_t dat)
{
    atk_mc5640_sccb_start();
    atk_mc5640_sccb_write_byte((id_addr << 1) | ATK_MC5640_SCCB_WRITE);
    atk_mc5640_sccb_write_byte((uint8_t)(sub_addr >> 8) & 0xFF);
    atk_mc5640_sccb_write_byte((uint8_t)sub_addr & 0xFF);
    atk_mc5640_sccb_write_byte(dat);
    atk_mc5640_sccb_stop();
}

/**
 * @brief       SCCB接口2阶段写传输
 * @param       id_addr : ID Address
 *              sub_addr: Sub-address
 * @retval      无
 */
void atk_mc5640_sccb_2_phase_write(uint8_t id_addr, uint16_t sub_addr)
{
    atk_mc5640_sccb_start();
    atk_mc5640_sccb_write_byte((id_addr << 1) | ATK_MC5640_SCCB_WRITE);
    atk_mc5640_sccb_write_byte((uint8_t)(sub_addr >> 8) & 0xFF);
    atk_mc5640_sccb_write_byte((uint8_t)sub_addr & 0xFF);
    atk_mc5640_sccb_stop();
}

/**
 * @brief       SCCB接口2阶段读传输
 * @param       id_addr: ID Address
 *              dat: 读取到的数据
 * @retval      无
 */
void atk_mc5640_sccb_2_phase_read(uint8_t id_addr, uint8_t *dat)
{
    atk_mc5640_sccb_start();
    atk_mc5640_sccb_write_byte((id_addr << 1) | ATK_MC5640_SCCB_READ);
    atk_mc5640_sccb_read_byte(dat);
    atk_mc5640_sccb_stop();
}
