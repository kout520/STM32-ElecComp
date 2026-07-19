/**
 ****************************************************************************************************
 * @file        atk_mc5640.c
 * @author      正点原子团队(ALIENTEK)
 * @version     V1.0
 * @date        2022-06-21
 * @brief       ATK-MC5640模块驱动代码
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

#include "./BSP/ATK_MC5640/atk_mc5640.h"
#include "./BSP/ATK_MC5640/atk_mc5640_cfg.h"
#include "./BSP/ATK_MC5640/atk_mc5640_sccb.h"
#include "./BSP/ATK_MC5640/atk_mc5640_dcmi.h"
#include "./SYSTEM/delay/delay.h"

/* ATK-MC5640模块芯片ID */
#define ATK_MC5640_CHIP_ID  0x5640

/* ATK-MC5640模块固件下载超时时间，单位：毫秒（ms） */
#define ATK_MC5640_TIMEOUT  5000

/* ATK-MC5640寄存器块枚举 */
typedef enum
{
    ATK_MC5640_REG_BANK_DSP = 0x00, /* DSP寄存器块 */
    ATK_MC5640_REG_BANK_SENSOR,     /* Sensor寄存器块 */
} atk_mc5640_reg_bank_t;

/* ATK-MC5640模块数据结构体 */
static struct
{
    struct {
        uint16_t width;
        uint16_t height;
    } isp_input;
    struct {
        uint16_t width;
        uint16_t height;
    } pre_scaling;
    struct {
        uint16_t width;
        uint16_t height;
    } output;
} g_atk_mc5640_sta = {0};

/**
 * @brief       ATK-MC5640模块写寄存器
 * @param       reg: 寄存器地址
 *              dat: 待写入的值
 * @retval      无
 */
static void atk_mc5640_write_reg(uint16_t reg, uint8_t dat)
{
    atk_mc5640_sccb_3_phase_write(ATK_MC5640_SCCB_ADDR, reg, dat);
}

/**
 * @brief       ATK-MC5640模块读寄存器
 * @param       reg: 寄存器的地址
 * @retval      读取到的寄存器值
 */
static uint8_t atk_mc5640_read_reg(uint16_t reg)
{
    uint8_t dat = 0;
    
    atk_mc5640_sccb_2_phase_write(ATK_MC5640_SCCB_ADDR, reg);
    atk_mc5640_sccb_2_phase_read(ATK_MC5640_SCCB_ADDR, &dat);
    
    return dat;
}

/**
 * @brief       获取ATK-MC5640模块ISP输入窗口尺寸
 * @param       无
 * @retval      无
 */
static void atk_mc5640_get_isp_input_size(void)
{
    uint8_t reg3800;
    uint8_t reg3801;
    uint8_t reg3802;
    uint8_t reg3803;
    uint8_t reg3804;
    uint8_t reg3805;
    uint8_t reg3806;
    uint8_t reg3807;
    uint16_t x_addr_st;
    uint16_t y_addr_st;
    uint16_t x_addr_end;
    uint16_t y_addr_end;
    
    delay_ms(100);
    
    reg3800 = atk_mc5640_read_reg(0x3800);
    reg3801 = atk_mc5640_read_reg(0x3801);
    reg3802 = atk_mc5640_read_reg(0x3802);
    reg3803 = atk_mc5640_read_reg(0x3803);
    reg3804 = atk_mc5640_read_reg(0x3804);
    reg3805 = atk_mc5640_read_reg(0x3805);
    reg3806 = atk_mc5640_read_reg(0x3806);
    reg3807 = atk_mc5640_read_reg(0x3807);
    
    x_addr_st = (uint16_t)((reg3800 & 0x0F) << 8) | reg3801;
    y_addr_st = (uint16_t)((reg3802 & 0x07) << 8) | reg3803;
    x_addr_end = (uint16_t)((reg3804 & 0x0F) << 8) | reg3805;
    y_addr_end = (uint16_t)((reg3806 & 0x07) << 8) | reg3807;
    
    g_atk_mc5640_sta.isp_input.width = x_addr_end - x_addr_st;
    g_atk_mc5640_sta.isp_input.height = y_addr_end - y_addr_st;
}

/**
 * @brief       获取ATK-MC5640模块预缩放窗口尺寸
 * @param       无
 * @retval      无
 */
static void atk_mc5640_get_pre_scaling_size(void)
{
    uint8_t reg3810;
    uint8_t reg3811;
    uint8_t reg3812;
    uint8_t reg3813;
    uint16_t x_offset;
    uint16_t y_offset;
    
    delay_ms(100);
    
    reg3810 = atk_mc5640_read_reg(0x3810);
    reg3811 = atk_mc5640_read_reg(0x3811);
    reg3812 = atk_mc5640_read_reg(0x3812);
    reg3813 = atk_mc5640_read_reg(0x3813);
    
    x_offset = (uint16_t)((reg3810 & 0x0F) << 8) | reg3811;
    y_offset = (uint16_t)((reg3812 & 0x07) << 8) | reg3813;
    
    atk_mc5640_get_isp_input_size();
    
    g_atk_mc5640_sta.pre_scaling.width = g_atk_mc5640_sta.isp_input.width - (x_offset << 1);
    g_atk_mc5640_sta.pre_scaling.height = g_atk_mc5640_sta.isp_input.height - (y_offset << 1);
}

/**
 * @brief       获取ATK-MC5640模块输出图像尺寸
 * @param       无
 * @retval      无
 */
static void atk_mc5640_get_output_size(void)
{
    uint8_t reg3808;
    uint8_t reg3809;
    uint8_t reg380A;
    uint8_t reg380B;
    uint16_t x_output_size;
    uint16_t y_output_size;
    
    delay_ms(100);
    
    reg3808 = atk_mc5640_read_reg(0x3808);
    reg3809 = atk_mc5640_read_reg(0x3809);
    reg380A = atk_mc5640_read_reg(0x380A);
    reg380B = atk_mc5640_read_reg(0x380B);
    
    x_output_size = (uint16_t)((reg3808 & 0x0F) << 8) | reg3809;
    y_output_size = (uint16_t)((reg380A & 0x07) << 8) | reg380B;
    
    g_atk_mc5640_sta.output.width = x_output_size;
    g_atk_mc5640_sta.output.height = y_output_size;
}

/**
 * @brief       ATK-MC5640模块硬件初始化
 * @param       无
 * @retval      无
 */
static void atk_mc5640_hw_init(void)
{
    GPIO_InitTypeDef gpio_init_struct = {0};
    
    /* 使能GPIO时钟 */
    ATK_MC5640_RST_GPIO_CLK_ENABLE();
    ATK_MC5640_PWDN_GPIO_CLK_ENABLE();
    
    /* 初始化RST引脚 */
    gpio_init_struct.Pin    = ATK_MC5640_RST_GPIO_PIN;
    gpio_init_struct.Mode   = GPIO_MODE_OUTPUT_PP;
    gpio_init_struct.Pull   = GPIO_PULLUP;
    gpio_init_struct.Speed  = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(ATK_MC5640_RST_GPIO_PORT, &gpio_init_struct);
    
    /* 初始化PWDN引脚 */
    gpio_init_struct.Pin    = ATK_MC5640_PWDN_GPIO_PIN;
    gpio_init_struct.Mode   = GPIO_MODE_OUTPUT_PP;
    gpio_init_struct.Pull   = GPIO_PULLUP;
    gpio_init_struct.Speed  = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(ATK_MC5640_PWDN_GPIO_PORT, &gpio_init_struct);
    
#if (ATK_MC5640_LED_CTL_BY_OV5640 == 0)
    /* 初始化FLASH引脚 */
    gpio_init_struct.Pin    = ATK_MC5640_FLASH_GPIO_PIN;
    gpio_init_struct.Mode   = GPIO_MODE_OUTPUT_PP;
    gpio_init_struct.Pull   = GPIO_PULLUP;
    gpio_init_struct.Speed  = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(ATK_MC5640_FLASH_GPIO_PORT, &gpio_init_struct);
#endif
    
    ATK_MC5640_RST(1);
    ATK_MC5640_PWDN(1);
#if (ATK_MC5640_LED_CTL_BY_OV5640 == 0)
    ATK_MC5640_FLASH(0);
#endif
}

/**
 * @brief       ATK-MC5640模块退出掉电模式
 * @param       无
 * @retval      无
 */
static void atk_mc5640_exit_power_down(void)
{
    ATK_MC5640_RST(0);
    delay_ms(20);
    ATK_MC5640_PWDN(0);
    delay_ms(5);
    ATK_MC5640_RST(1);
    delay_ms(20);
}

/**
 * @brief       ATK-MC5640模块硬件复位
 * @param       无
 * @retval      无
 */
static void atk_mc5640_hw_reset(void)
{
    ATK_MC5640_RST(0);
    delay_ms(20);
    ATK_MC5640_RST(1);
    delay_ms(20);
}

/**
 * @brief       ATK-MC5640模块软件复位
 * @param       无
 * @retval      无
 */
static void atk_mc5640_sw_reset(void)
{
    uint8_t reg3103;
    
    reg3103 = atk_mc5640_read_reg(0x3103);
    reg3103 &= ~(0x01 << 1);
    atk_mc5640_write_reg(0x3103, reg3103);
    atk_mc5640_write_reg(0x3008, 0x82);
    delay_ms(10);
}

/**
 * @brief       获取ATK-MC5640模块芯片ID
 * @param       无
 * @retval      芯片ID
 */
static uint16_t atk_mc5640_get_chip_id(void)
{
    uint16_t chip_id;
    
    chip_id = atk_mc5640_read_reg(0x300A) << 8;
    chip_id |= atk_mc5640_read_reg(0x300B);
    
    return chip_id;
}

/**
 * @brief       初始化ATK-MC5640寄存器配置
 * @param       无
 * @retval      无
 */
static void atk_mc5640_init_reg(void)
{
    uint32_t cfg_index;
    
    for (cfg_index=0; cfg_index<sizeof(atk_mc5640_init_cfg)/sizeof(atk_mc5640_init_cfg[0]); cfg_index++)
    {
        atk_mc5640_write_reg(atk_mc5640_init_cfg[cfg_index].reg, atk_mc5640_init_cfg[cfg_index].dat);
    }
}

/**
 * @brief       初始化ATK-MC5640模块
 * @param       无
 * @retval      ATK_MC5640_EOK   : ATK-MC5640模块初始化成功
 *              ATK_MC5640_ERROR : 通讯出错，ATK-MC5640模块初始化失败
 *              ATK_MC5640_ENOMEM: 内存不足，ATK-MC5640模块初始化失败
 */
uint8_t atk_mc5640_init(void)
{
    uint16_t chip_id;
    
    atk_mc5640_hw_init();               /* ATK-MC5640模块硬件初始化 */
    atk_mc5640_exit_power_down();       /* ATK-MC5640模块退出掉电模式 */
    atk_mc5640_hw_reset();              /* ATK-MC5640模块硬件复位 */
    atk_mc5640_sccb_init();             /* ATK-MC5640 SCCB接口初始化 */
    atk_mc5640_sw_reset();              /* ATK-MC5640模块软件复位 */
    
    chip_id = atk_mc5640_get_chip_id(); /* 获取芯片ID */
    if (chip_id != ATK_MC5640_CHIP_ID)
    {
        return ATK_MC5640_ERROR;
    }
    
    atk_mc5640_init_reg();              /* 初始化ATK-MC5640寄存器配置 */
    
    atk_mc5640_dcmi_init();             /* 初始化ATK-MC5640模块DCMI接口 */
    
    return ATK_MC5640_EOK;
}

/**
 * @brief       初始化ATK-MC5640模块自动对焦
 * @param       无
 * @retval      ATK_MC5640_EOK     : ATK-MC5640模块自动对焦初始化成功
 *              ATK_MC5640_ETIMEOUT: ATK-MC5640模块下载固件超时
 */
uint8_t atk_mc5640_auto_focus_init(void)
{
    uint32_t fw_index;
    uint16_t addr_index;
    uint8_t reg3029 = 0;
    uint16_t timeout = 0;
    
    atk_mc5640_write_reg(0x3000, 0x20);
    
    for (addr_index=ATK_MC5640_FW_DOWNLOAD_ADDR, fw_index=0; fw_index<sizeof(atk_mc5640_auto_focus_firmware);addr_index++, fw_index++)
    {
        atk_mc5640_write_reg(addr_index, atk_mc5640_auto_focus_firmware[fw_index]);
    }
    
    atk_mc5640_write_reg(0x3022, 0x00);
    atk_mc5640_write_reg(0x3023, 0x00);
    atk_mc5640_write_reg(0x3024, 0x00);
    atk_mc5640_write_reg(0x3025, 0x00);
    atk_mc5640_write_reg(0x3026, 0x00);
    atk_mc5640_write_reg(0x3027, 0x00);
    atk_mc5640_write_reg(0x3028, 0x00);
    atk_mc5640_write_reg(0x3029, 0x7F);
    atk_mc5640_write_reg(0x3000, 0x00);
    
    while ((reg3029 != 0x70) && (timeout < ATK_MC5640_TIMEOUT))
    {
        delay_ms(1);
        reg3029 = atk_mc5640_read_reg(0x3029);
        timeout++;
    }
    
    if (reg3029 != 0x70)
    {
        return ATK_MC5640_ETIMEOUT;
    }
    
    return ATK_MC5640_EOK;
}

/**
 * @brief       ATK-MC5640模块自动对焦一次
 * @param       无
 * @retval      ATK_MC5640_EOK     : ATK-MC5640模块自动对焦一次成功
 *              ATK_MC5640_ETIMEOUT: ATK-MC5640模块自动对焦一次超时
 */
uint8_t atk_mc5640_auto_focus_once(void)
{
    uint8_t reg3029 = 0;
    uint16_t timeout = 0;
    
    atk_mc5640_write_reg(0x3022, 0x03);
    
    while ((reg3029 != 0x10) && (timeout < ATK_MC5640_TIMEOUT))
    {
        delay_ms(1);
        reg3029 = atk_mc5640_read_reg(0x3029);
        timeout++;
    }
    
    if (reg3029 != 0x10)
    {
        return ATK_MC5640_ETIMEOUT;
    }
    
    return ATK_MC5640_EOK;
}

/**
 * @brief       ATK-MC5640模块持续自动对焦
 * @param       无
 * @retval      ATK_MC5640_EOK     : ATK-MC5640模块持续自动对焦成功
 *              ATK_MC5640_ETIMEOUT: ATK-MC5640模块持续自动对焦超时
 */
uint8_t atk_mc5640_auto_focus_continuance(void)
{
    uint8_t reg3023 = ~0;
    uint16_t timeout = 0;
    
    atk_mc5640_write_reg(0x3023, 0x01);
    atk_mc5640_write_reg(0x3022, 0x08);
    
    while ((reg3023 != 0x00) && (timeout < ATK_MC5640_TIMEOUT))
    {
        delay_ms(1);
        reg3023 = atk_mc5640_read_reg(0x3023);
        timeout++;
    }
    
    if (reg3023 != 0x00)
    {
        return ATK_MC5640_ETIMEOUT;
    }
    
    reg3023 = ~0;
    timeout = 0;
    
    atk_mc5640_write_reg(0x3023, 0x01);
    atk_mc5640_write_reg(0x3022, 0x04);
    
    while ((reg3023 != 0x00) && (timeout < ATK_MC5640_TIMEOUT))
    {
        delay_ms(1);
        reg3023 = atk_mc5640_read_reg(0x3023);
        timeout++;
    }
    
    if (reg3023 != 0x00)
    {
        return ATK_MC5640_ETIMEOUT;
    }
    
    return ATK_MC5640_EOK;
}

#if (ATK_MC5640_LED_CTL_BY_OV5640 == 0)
/**
 * @brief       开启ATK-MC5640模块闪光灯
 * @param       无
 * @retval      无
 */
void atk_mc5640_led_on(void)
{
    ATK_MC5640_FLASH(1);
}

/**
 * @brief       关闭ATK-MC5640模块闪光灯
 * @param       无
 * @retval      无
 */
void atk_mc5640_led_off(void)
{
    ATK_MC5640_FLASH(0);
}
#else
/**
 * @brief       开启ATK-MC5640模块闪光灯
 * @param       无
 * @retval      无
 */
void atk_mc5640_led_on(void)
{
    atk_mc5640_write_reg(0x3016, 0x02);
    atk_mc5640_write_reg(0x301C, 0x02);
    atk_mc5640_write_reg(0x3019, 0x02);
}

/**
 * @brief       关闭ATK-MC5640模块闪光灯
 * @param       无
 * @retval      无
 */
void atk_mc5640_led_off(void)
{
    atk_mc5640_write_reg(0x3016, 0x02);
    atk_mc5640_write_reg(0x301C, 0x02);
    atk_mc5640_write_reg(0x3019, 0x00);
}
#endif

/**
 * @brief       设置ATK-MC5640模块灯光模式
 * @param       mode: ATK_MC5640_LIGHT_MODE_ADVANCED_AWB : Advanced AWB
 *                    ATK_MC5640_LIGHT_MODE_SIMPLE_AWB   : Simple AWB
 *                    ATK_MC5640_LIGHT_MODE_MANUAL_DAY   : Manual day
 *                    ATK_MC5640_LIGHT_MODE_MANUAL_A     : Manual A
 *                    ATK_MC5640_LIGHT_MODE_MANUAL_CWF   : Manual cwf
 *                    ATK_MC5640_LIGHT_MODE_MANUAL_CLOUDY: Manual cloudy
 * @retval      ATK_MC5640_EOK   : 设置ATK-MC5640模块灯光模式成功
 *              ATK_MC5640_EINVAL: 传入参数错误
 */
uint8_t atk_mc5640_set_light_mode(atk_mc5640_light_mode_t mode)
{
    switch (mode)
    {
        case ATK_MC5640_LIGHT_MODE_ADVANCED_AWB:
        {
            atk_mc5640_write_reg(0x3406, 0x00);
            atk_mc5640_write_reg(0x5192, 0x04);
            atk_mc5640_write_reg(0x5191, 0xF8);
            atk_mc5640_write_reg(0x5193, 0x70);
            atk_mc5640_write_reg(0x5194, 0xF0);
            atk_mc5640_write_reg(0x5195, 0xF0);
            atk_mc5640_write_reg(0x518D, 0x3D);
            atk_mc5640_write_reg(0x518F, 0x54);
            atk_mc5640_write_reg(0x518E, 0x3D);
            atk_mc5640_write_reg(0x5190, 0x54);
            atk_mc5640_write_reg(0x518B, 0xA8);
            atk_mc5640_write_reg(0x518C, 0xA8);
            atk_mc5640_write_reg(0x5187, 0x18);
            atk_mc5640_write_reg(0x5188, 0x18);
            atk_mc5640_write_reg(0x5189, 0x6E);
            atk_mc5640_write_reg(0x518A, 0x68);
            atk_mc5640_write_reg(0x5186, 0x1C);
            atk_mc5640_write_reg(0x5181, 0x50);
            atk_mc5640_write_reg(0x5184, 0x25);
            atk_mc5640_write_reg(0x5182, 0x11);
            atk_mc5640_write_reg(0x5183, 0x14);
            atk_mc5640_write_reg(0x5184, 0x25);
            atk_mc5640_write_reg(0x5185, 0x24);
            break;
        }
        case ATK_MC5640_LIGHT_MODE_SIMPLE_AWB:
        {
            atk_mc5640_write_reg(0x3406, 0x00);
            atk_mc5640_write_reg(0x5183, 0x94);
            atk_mc5640_write_reg(0x5191, 0xFF);
            atk_mc5640_write_reg(0x5192, 0x00);
            break;
        }
        case ATK_MC5640_LIGHT_MODE_MANUAL_DAY:
        {
            atk_mc5640_write_reg(0x3406, 0x01);
            atk_mc5640_write_reg(0x3400, 0x06);
            atk_mc5640_write_reg(0x3401, 0x1C);
            atk_mc5640_write_reg(0x3402, 0x04);
            atk_mc5640_write_reg(0x3403, 0x00);
            atk_mc5640_write_reg(0x3404, 0x04);
            atk_mc5640_write_reg(0x3405, 0xF3);
            break;
        }
        case ATK_MC5640_LIGHT_MODE_MANUAL_A:
        {
            atk_mc5640_write_reg(0x3406, 0x01);
            atk_mc5640_write_reg(0x3400, 0x04);
            atk_mc5640_write_reg(0x3401, 0x10);
            atk_mc5640_write_reg(0x3402, 0x04);
            atk_mc5640_write_reg(0x3403, 0x00);
            atk_mc5640_write_reg(0x3404, 0x08);
            atk_mc5640_write_reg(0x3405, 0xB6);
            break;
        }
        case ATK_MC5640_LIGHT_MODE_MANUAL_CWF:
        {
            atk_mc5640_write_reg(0x3406, 0x01);
            atk_mc5640_write_reg(0x3400, 0x05);
            atk_mc5640_write_reg(0x3401, 0x48);
            atk_mc5640_write_reg(0x3402, 0x04);
            atk_mc5640_write_reg(0x3403, 0x00);
            atk_mc5640_write_reg(0x3404, 0x07);
            atk_mc5640_write_reg(0x3405, 0xCF);
            break;
        }
        case ATK_MC5640_LIGHT_MODE_MANUAL_CLOUDY:
        {
            atk_mc5640_write_reg(0x3406, 0x01);
            atk_mc5640_write_reg(0x3400, 0x06);
            atk_mc5640_write_reg(0x3401, 0x48);
            atk_mc5640_write_reg(0x3402, 0x04);
            atk_mc5640_write_reg(0x3403, 0x00);
            atk_mc5640_write_reg(0x3404, 0x04);
            atk_mc5640_write_reg(0x3405, 0xD3);
            break;
        }
        default:
        {
            return ATK_MC5640_EINVAL;
        }
    }
    
    return ATK_MC5640_EOK;
}

/**
 * @brief       设置ATK-MC5640模块色彩饱和度
 * @param       saturation: ATK_MC5640_COLOR_SATURATION_0: +4
 *                          ATK_MC5640_COLOR_SATURATION_1: +3
 *                          ATK_MC5640_COLOR_SATURATION_2: +2
 *                          ATK_MC5640_COLOR_SATURATION_3: +1
 *                          ATK_MC5640_COLOR_SATURATION_4: 0
 *                          ATK_MC5640_COLOR_SATURATION_5: -1
 *                          ATK_MC5640_COLOR_SATURATION_6: -2
 *                          ATK_MC5640_COLOR_SATURATION_7: -3
 *                          ATK_MC5640_COLOR_SATURATION_8: -4
 * @retval      ATK_MC5640_EOK   : 设置ATK-MC5640模块色彩饱和度成功
 *              ATK_MC5640_EINVAL: 传入参数错误
 */
uint8_t atk_mc5640_set_color_saturation(atk_mc5640_color_saturation_t saturation)
{
    switch (saturation)
    {
        case ATK_MC5640_COLOR_SATURATION_0:
        {
            atk_mc5640_write_reg(0x5001, 0xFF);
            atk_mc5640_write_reg(0x5583, 0x80);
            atk_mc5640_write_reg(0x5584, 0x80);
            atk_mc5640_write_reg(0x5580, 0x02);
            atk_mc5640_write_reg(0x5588, 0x41);
            break;
        }
        case ATK_MC5640_COLOR_SATURATION_1:
        {
            atk_mc5640_write_reg(0x5001, 0xFF);
            atk_mc5640_write_reg(0x5583, 0x70);
            atk_mc5640_write_reg(0x5584, 0x70);
            atk_mc5640_write_reg(0x5580, 0x02);
            atk_mc5640_write_reg(0x5588, 0x41);
            break;
        }
        case ATK_MC5640_COLOR_SATURATION_2:
        {
            atk_mc5640_write_reg(0x5001, 0xFF);
            atk_mc5640_write_reg(0x5583, 0x60);
            atk_mc5640_write_reg(0x5584, 0x60);
            atk_mc5640_write_reg(0x5580, 0x02);
            atk_mc5640_write_reg(0x5588, 0x41);
            break;
        }
        case ATK_MC5640_COLOR_SATURATION_3:
        {
            atk_mc5640_write_reg(0x5001, 0xFF);
            atk_mc5640_write_reg(0x5583, 0x50);
            atk_mc5640_write_reg(0x5584, 0x50);
            atk_mc5640_write_reg(0x5580, 0x02);
            atk_mc5640_write_reg(0x5588, 0x41);
            break;
        }
        case ATK_MC5640_COLOR_SATURATION_4:
        {
            atk_mc5640_write_reg(0x5001, 0xFF);
            atk_mc5640_write_reg(0x5583, 0x40);
            atk_mc5640_write_reg(0x5584, 0x40);
            atk_mc5640_write_reg(0x5580, 0x02);
            atk_mc5640_write_reg(0x5588, 0x41);
            break;
        }
        case ATK_MC5640_COLOR_SATURATION_5:
        {
            atk_mc5640_write_reg(0x5001, 0xFF);
            atk_mc5640_write_reg(0x5583, 0x30);
            atk_mc5640_write_reg(0x5584, 0x30);
            atk_mc5640_write_reg(0x5580, 0x02);
            atk_mc5640_write_reg(0x5588, 0x41);
            break;
        }
        case ATK_MC5640_COLOR_SATURATION_6:
        {
            atk_mc5640_write_reg(0x5001, 0xFF);
            atk_mc5640_write_reg(0x5583, 0x20);
            atk_mc5640_write_reg(0x5584, 0x20);
            atk_mc5640_write_reg(0x5580, 0x02);
            atk_mc5640_write_reg(0x5588, 0x41);
            break;
        }
        case ATK_MC5640_COLOR_SATURATION_7:
        {
            atk_mc5640_write_reg(0x5001, 0xFF);
            atk_mc5640_write_reg(0x5583, 0x10);
            atk_mc5640_write_reg(0x5584, 0x10);
            atk_mc5640_write_reg(0x5580, 0x02);
            atk_mc5640_write_reg(0x5588, 0x41);
            break;
        }
        case ATK_MC5640_COLOR_SATURATION_8:
        {
            atk_mc5640_write_reg(0x5001, 0xFF);
            atk_mc5640_write_reg(0x5583, 0x00);
            atk_mc5640_write_reg(0x5584, 0x00);
            atk_mc5640_write_reg(0x5580, 0x02);
            atk_mc5640_write_reg(0x5588, 0x41);
            break;
        }
        default:
        {
            return ATK_MC5640_EINVAL;
        }
    }
    
    return ATK_MC5640_EOK;
}

/**
 * @brief       设置ATK-MC5640模块亮度
 * @param       brightness: ATK_MC5640_BRIGHTNESS_0: +4
 *                          ATK_MC5640_BRIGHTNESS_1: +3
 *                          ATK_MC5640_BRIGHTNESS_2: +2
 *                          ATK_MC5640_BRIGHTNESS_3: +1
 *                          ATK_MC5640_BRIGHTNESS_4: 0
 *                          ATK_MC5640_BRIGHTNESS_5: -1
 *                          ATK_MC5640_BRIGHTNESS_6: -2
 *                          ATK_MC5640_BRIGHTNESS_7: -3
 *                          ATK_MC5640_BRIGHTNESS_8: -4
 * @retval      ATK_MC5640_EOK   : 设置ATK-MC5640模块亮度成功
 *              ATK_MC5640_EINVAL: 传入参数错误
 */
uint8_t atk_mc5640_set_brightness(atk_mc5640_brightness_t brightness)
{
    switch (brightness)
    {
        case ATK_MC5640_BRIGHTNESS_0:
        {
            atk_mc5640_write_reg(0x5001, 0xFF);
            atk_mc5640_write_reg(0x5587, 0x40);
            atk_mc5640_write_reg(0x5580, 0x04);
            atk_mc5640_write_reg(0x5588, 0x01);
            break;
        }
        case ATK_MC5640_BRIGHTNESS_1:
        {
            atk_mc5640_write_reg(0x5001, 0xFF);
            atk_mc5640_write_reg(0x5587, 0x30);
            atk_mc5640_write_reg(0x5580, 0x04);
            atk_mc5640_write_reg(0x5588, 0x01);
            break;
        }
        case ATK_MC5640_BRIGHTNESS_2:
        {
            atk_mc5640_write_reg(0x5001, 0xFF);
            atk_mc5640_write_reg(0x5587, 0x20);
            atk_mc5640_write_reg(0x5580, 0x04);
            atk_mc5640_write_reg(0x5588, 0x01);
            break;
        }
        case ATK_MC5640_BRIGHTNESS_3:
        {
            atk_mc5640_write_reg(0x5001, 0xFF);
            atk_mc5640_write_reg(0x5587, 0x10);
            atk_mc5640_write_reg(0x5580, 0x04);
            atk_mc5640_write_reg(0x5588, 0x01);
            break;
        }
        case ATK_MC5640_BRIGHTNESS_4:
        {
            atk_mc5640_write_reg(0x5001, 0xFF);
            atk_mc5640_write_reg(0x5587, 0x00);
            atk_mc5640_write_reg(0x5580, 0x04);
            atk_mc5640_write_reg(0x5588, 0x01);
            break;
        }
        case ATK_MC5640_BRIGHTNESS_5:
        {
            atk_mc5640_write_reg(0x5001, 0xFF);
            atk_mc5640_write_reg(0x5587, 0x10);
            atk_mc5640_write_reg(0x5580, 0x04);
            atk_mc5640_write_reg(0x5588, 0x09);
            break;
        }
        case ATK_MC5640_BRIGHTNESS_6:
        {
            atk_mc5640_write_reg(0x5001, 0xFF);
            atk_mc5640_write_reg(0x5587, 0x20);
            atk_mc5640_write_reg(0x5580, 0x04);
            atk_mc5640_write_reg(0x5588, 0x09);
            break;
        }
        case ATK_MC5640_BRIGHTNESS_7:
        {
            atk_mc5640_write_reg(0x5001, 0xFF);
            atk_mc5640_write_reg(0x5587, 0x30);
            atk_mc5640_write_reg(0x5580, 0x04);
            atk_mc5640_write_reg(0x5588, 0x09);
            break;
        }
        case ATK_MC5640_BRIGHTNESS_8:
        {
            atk_mc5640_write_reg(0x5001, 0xFF);
            atk_mc5640_write_reg(0x5587, 0x40);
            atk_mc5640_write_reg(0x5580, 0x04);
            atk_mc5640_write_reg(0x5588, 0x09);
            break;
        }
        default:
        {
            return ATK_MC5640_EINVAL;
        }
    }
    
    return ATK_MC5640_EOK;
}

/**
 * @brief       设置ATK-MC5640模块对比度
 * @param       contrast: ATK_MC5640_CONTRAST_0: +4
 *                        ATK_MC5640_CONTRAST_1: +3
 *                        ATK_MC5640_CONTRAST_2: +2
 *                        ATK_MC5640_CONTRAST_3: +1
 *                        ATK_MC5640_CONTRAST_4: 0
 *                        ATK_MC5640_CONTRAST_5: -1
 *                        ATK_MC5640_CONTRAST_6: -2
 *                        ATK_MC5640_CONTRAST_7: -3
 *                        ATK_MC5640_CONTRAST_8: -4
 * @retval      ATK_MC5640_EOK   : 设置ATK-MC5640模块对比度成功
 *              ATK_MC5640_EINVAL: 传入参数错误
 */
uint8_t atk_mc5640_set_contrast(atk_mc5640_contrast_t contrast)
{
    switch (contrast)
    {
        case ATK_MC5640_CONTRAST_0:
        {
            atk_mc5640_write_reg(0x5001, 0xFF);
            atk_mc5640_write_reg(0x5580, 0x04);
            atk_mc5640_write_reg(0x5586, 0x30);
            atk_mc5640_write_reg(0x5585, 0x30);
            atk_mc5640_write_reg(0x5588, 0x41);
            break;
        }
        case ATK_MC5640_CONTRAST_1:
        {
            atk_mc5640_write_reg(0x5001, 0xFF);
            atk_mc5640_write_reg(0x5580, 0x04);
            atk_mc5640_write_reg(0x5586, 0x2C);
            atk_mc5640_write_reg(0x5585, 0x2C);
            atk_mc5640_write_reg(0x5588, 0x41);
            break;
        }
        case ATK_MC5640_CONTRAST_2:
        {
            atk_mc5640_write_reg(0x5001, 0xFF);
            atk_mc5640_write_reg(0x5580, 0x04);
            atk_mc5640_write_reg(0x5586, 0x28);
            atk_mc5640_write_reg(0x5585, 0x28);
            atk_mc5640_write_reg(0x5588, 0x41);
            break;
        }
        case ATK_MC5640_CONTRAST_3:
        {
            atk_mc5640_write_reg(0x5001, 0xFF);
            atk_mc5640_write_reg(0x5580, 0x04);
            atk_mc5640_write_reg(0x5586, 0x24);
            atk_mc5640_write_reg(0x5585, 0x24);
            atk_mc5640_write_reg(0x5588, 0x41);
            break;
        }
        case ATK_MC5640_CONTRAST_4:
        {
            atk_mc5640_write_reg(0x5001, 0xFF);
            atk_mc5640_write_reg(0x5580, 0x04);
            atk_mc5640_write_reg(0x5586, 0x20);
            atk_mc5640_write_reg(0x5585, 0x20);
            atk_mc5640_write_reg(0x5588, 0x41);
            break;
        }
        case ATK_MC5640_CONTRAST_5:
        {
            atk_mc5640_write_reg(0x5001, 0xFF);
            atk_mc5640_write_reg(0x5580, 0x04);
            atk_mc5640_write_reg(0x5586, 0x1C);
            atk_mc5640_write_reg(0x5585, 0x1C);
            atk_mc5640_write_reg(0x5588, 0x41);
            break;
        }
        case ATK_MC5640_CONTRAST_6:
        {
            atk_mc5640_write_reg(0x5001, 0xFF);
            atk_mc5640_write_reg(0x5580, 0x04);
            atk_mc5640_write_reg(0x5586, 0x18);
            atk_mc5640_write_reg(0x5585, 0x18);
            atk_mc5640_write_reg(0x5588, 0x41);
            break;
        }
        case ATK_MC5640_CONTRAST_7:
        {
            atk_mc5640_write_reg(0x5001, 0xFF);
            atk_mc5640_write_reg(0x5580, 0x04);
            atk_mc5640_write_reg(0x5586, 0x14);
            atk_mc5640_write_reg(0x5585, 0x14);
            atk_mc5640_write_reg(0x5588, 0x41);
            break;
        }
        case ATK_MC5640_CONTRAST_8:
        {
            atk_mc5640_write_reg(0x5001, 0xFF);
            atk_mc5640_write_reg(0x5580, 0x04);
            atk_mc5640_write_reg(0x5586, 0x10);
            atk_mc5640_write_reg(0x5585, 0x10);
            atk_mc5640_write_reg(0x5588, 0x41);
            break;
        }
        default:
        {
            return ATK_MC5640_EINVAL;
        }
    }
    
    return ATK_MC5640_EOK;
}

/**
 * @brief       设置ATK-MC5640模块色相
 * @param       contrast: ATK_MC5640_HUE_0 : -180 degree
 *                        ATK_MC5640_HUE_1 : -150 degree
 *                        ATK_MC5640_HUE_2 : -120 degree
 *                        ATK_MC5640_HUE_3 : -90 degree
 *                        ATK_MC5640_HUE_4 : -60 degree
 *                        ATK_MC5640_HUE_5 : -30 degree
 *                        ATK_MC5640_HUE_6 : 0 degree
 *                        ATK_MC5640_HUE_7 : +30 degree
 *                        ATK_MC5640_HUE_8 : +60 degree
 *                        ATK_MC5640_HUE_9 : +90 degree
 *                        ATK_MC5640_HUE_10: +120 degree
 *                        ATK_MC5640_HUE_11: +150 degree
 * @retval      ATK_MC5640_EOK   : 设置ATK-MC5640模块色相成功
 *              ATK_MC5640_EINVAL: 传入参数错误
 */
uint8_t atk_mc5640_set_hue(atk_mc5640_hue_t hue)
{
    switch (hue)
    {
        case ATK_MC5640_HUE_0:
        {
            atk_mc5640_write_reg(0x5001, 0xFF);
            atk_mc5640_write_reg(0x5580, 0x01);
            atk_mc5640_write_reg(0x5581, 0x80);
            atk_mc5640_write_reg(0x5582, 0x00);
            atk_mc5640_write_reg(0x5588, 0x32);
            break;
        }
        case ATK_MC5640_HUE_1:
        {
            atk_mc5640_write_reg(0x5001, 0xFF);
            atk_mc5640_write_reg(0x5580, 0x01);
            atk_mc5640_write_reg(0x5581, 0x6F);
            atk_mc5640_write_reg(0x5582, 0x40);
            atk_mc5640_write_reg(0x5588, 0x32);
            break;
        }
        case ATK_MC5640_HUE_2:
        {
            atk_mc5640_write_reg(0x5001, 0xFF);
            atk_mc5640_write_reg(0x5580, 0x01);
            atk_mc5640_write_reg(0x5581, 0x40);
            atk_mc5640_write_reg(0x5582, 0x6F);
            atk_mc5640_write_reg(0x5588, 0x32);
            break;
        }
        case ATK_MC5640_HUE_3:
        {
            atk_mc5640_write_reg(0x5001, 0xFF);
            atk_mc5640_write_reg(0x5580, 0x01);
            atk_mc5640_write_reg(0x5581, 0x00);
            atk_mc5640_write_reg(0x5582, 0x80);
            atk_mc5640_write_reg(0x5588, 0x02);
            break;
        }
        case ATK_MC5640_HUE_4:
        {
            atk_mc5640_write_reg(0x5001, 0xFF);
            atk_mc5640_write_reg(0x5580, 0x01);
            atk_mc5640_write_reg(0x5581, 0x40);
            atk_mc5640_write_reg(0x5582, 0x6F);
            atk_mc5640_write_reg(0x5588, 0x02);
            break;
        }
        case ATK_MC5640_HUE_5:
        {
            atk_mc5640_write_reg(0x5001, 0xFF);
            atk_mc5640_write_reg(0x5580, 0x01);
            atk_mc5640_write_reg(0x5581, 0x6F);
            atk_mc5640_write_reg(0x5582, 0x40);
            atk_mc5640_write_reg(0x5588, 0x02);
            break;
        }
        case ATK_MC5640_HUE_6:
        {
            atk_mc5640_write_reg(0x5001, 0xFF);
            atk_mc5640_write_reg(0x5580, 0x01);
            atk_mc5640_write_reg(0x5581, 0x80);
            atk_mc5640_write_reg(0x5582, 0x00);
            atk_mc5640_write_reg(0x5588, 0x01);
            break;
        }
        case ATK_MC5640_HUE_7:
        {
            atk_mc5640_write_reg(0x5001, 0xFF);
            atk_mc5640_write_reg(0x5580, 0x01);
            atk_mc5640_write_reg(0x5581, 0x6F);
            atk_mc5640_write_reg(0x5582, 0x40);
            atk_mc5640_write_reg(0x5588, 0x01);
            break;
        }
        case ATK_MC5640_HUE_8:
        {
            atk_mc5640_write_reg(0x5001, 0xFF);
            atk_mc5640_write_reg(0x5580, 0x01);
            atk_mc5640_write_reg(0x5581, 0x40);
            atk_mc5640_write_reg(0x5582, 0x6F);
            atk_mc5640_write_reg(0x5588, 0x01);
            break;
        }
        case ATK_MC5640_HUE_9:
        {
            atk_mc5640_write_reg(0x5001, 0xFF);
            atk_mc5640_write_reg(0x5580, 0x01);
            atk_mc5640_write_reg(0x5581, 0x00);
            atk_mc5640_write_reg(0x5582, 0x80);
            atk_mc5640_write_reg(0x5588, 0x31);
            break;
        }
        case ATK_MC5640_HUE_10:
        {
            atk_mc5640_write_reg(0x5001, 0xFF);
            atk_mc5640_write_reg(0x5580, 0x01);
            atk_mc5640_write_reg(0x5581, 0x40);
            atk_mc5640_write_reg(0x5582, 0x6F);
            atk_mc5640_write_reg(0x5588, 0x31);
            break;
        }
        case ATK_MC5640_HUE_11:
        {
            atk_mc5640_write_reg(0x5001, 0xFF);
            atk_mc5640_write_reg(0x5580, 0x01);
            atk_mc5640_write_reg(0x5581, 0x6F);
            atk_mc5640_write_reg(0x5582, 0x40);
            atk_mc5640_write_reg(0x5588, 0x31);
            break;
        }
        default:
        {
            return ATK_MC5640_EINVAL;
        }
    }
    
    return ATK_MC5640_EOK;
}

/**
 * @brief       设置ATK-MC5640模块特殊效果
 * @param       contrast: ATK_MC5640_SPECIAL_EFFECT_NORMAL  : Normal
 *                        ATK_MC5640_SPECIAL_EFFECT_BW      : B&W
 *                        ATK_MC5640_SPECIAL_EFFECT_BLUISH  : Bluish
 *                        ATK_MC5640_SPECIAL_EFFECT_SEPIA   : Sepia
 *                        ATK_MC5640_SPECIAL_EFFECT_REDDISH : Reddish 
 *                        ATK_MC5640_SPECIAL_EFFECT_GREENISH: Greenish
 *                        ATK_MC5640_SPECIAL_EFFECT_NEGATIVE: Negative
 * @retval      ATK_MC5640_EOK   : 设置ATK-MC5640模块特殊效果成功
 *              ATK_MC5640_EINVAL: 传入参数错误
 */
uint8_t atk_mc5640_set_special_effect(atk_mc5640_special_effect_t effect)
{
    switch (effect)
    {
        case ATK_MC5640_SPECIAL_EFFECT_NORMAL:
        {
            atk_mc5640_write_reg(0x5001, 0x7F);
            atk_mc5640_write_reg(0x5580, 0x00);
            break;
        }
        case ATK_MC5640_SPECIAL_EFFECT_BW:
        {
            atk_mc5640_write_reg(0x5001, 0xFF);
            atk_mc5640_write_reg(0x5580, 0x18);
            atk_mc5640_write_reg(0x5583, 0x80);
            atk_mc5640_write_reg(0x5584, 0x80);
            break;
        }
        case ATK_MC5640_SPECIAL_EFFECT_BLUISH:
        {
            atk_mc5640_write_reg(0x5001, 0xFF);
            atk_mc5640_write_reg(0x5580, 0x18);
            atk_mc5640_write_reg(0x5583, 0xA0);
            atk_mc5640_write_reg(0x5584, 0x40);
            break;
        }
        case ATK_MC5640_SPECIAL_EFFECT_SEPIA:
        {
            atk_mc5640_write_reg(0x5001, 0xFF);
            atk_mc5640_write_reg(0x5580, 0x18);
            atk_mc5640_write_reg(0x5583, 0x40);
            atk_mc5640_write_reg(0x5584, 0xA0);
            break;
        }
        case ATK_MC5640_SPECIAL_EFFECT_REDDISH:
        {
            atk_mc5640_write_reg(0x5001, 0xFF);
            atk_mc5640_write_reg(0x5580, 0x18);
            atk_mc5640_write_reg(0x5583, 0x80);
            atk_mc5640_write_reg(0x5584, 0xC0);
            break;
        }
        case ATK_MC5640_SPECIAL_EFFECT_GREENISH:
        {
            atk_mc5640_write_reg(0x5001, 0xFF);
            atk_mc5640_write_reg(0x5580, 0x18);
            atk_mc5640_write_reg(0x5583, 0x60);
            atk_mc5640_write_reg(0x5584, 0x60);
            break;
        }
        case ATK_MC5640_SPECIAL_EFFECT_NEGATIVE:
        {
            atk_mc5640_write_reg(0x5001, 0xFF);
            atk_mc5640_write_reg(0x5580, 0x40);
            break;
        }
        default:
        {
            return ATK_MC5640_EINVAL;
        }
    }
    
    return ATK_MC5640_EOK;
}

/**
 * @brief       设置ATK-MC5640模块曝光度
 * @param       contrast: ATK_MC5640_EXPOSURE_LEVEL_0 : -1.7EV
 *                        ATK_MC5640_EXPOSURE_LEVEL_1 : -1.3EV
 *                        ATK_MC5640_EXPOSURE_LEVEL_2 : -1.0EV
 *                        ATK_MC5640_EXPOSURE_LEVEL_3 : -0.7EV
 *                        ATK_MC5640_EXPOSURE_LEVEL_4 : -0.3EV
 *                        ATK_MC5640_EXPOSURE_LEVEL_5 : default
 *                        ATK_MC5640_EXPOSURE_LEVEL_6 : 0.3EV
 *                        ATK_MC5640_EXPOSURE_LEVEL_7 : 0.7EV
 *                        ATK_MC5640_EXPOSURE_LEVEL_8 : 1.0EV
 *                        ATK_MC5640_EXPOSURE_LEVEL_9 : 1.3EV
 *                        ATK_MC5640_EXPOSURE_LEVEL_10: 1.7EV
 * @retval      ATK_MC5640_EOK   : 设置ATK-MC5640模块曝光度成功
 *              ATK_MC5640_EINVAL: 传入参数错误
 */
uint8_t atk_mc5640_set_exposure_level(atk_mc5640_exposure_level_t level)
{
    switch (level)
    {
        case ATK_MC5640_EXPOSURE_LEVEL_0:
        {
            atk_mc5640_write_reg(0x3A0F, 0x10);
            atk_mc5640_write_reg(0x3A10, 0x08);
            atk_mc5640_write_reg(0x3A1B, 0x10);
            atk_mc5640_write_reg(0x3A1E, 0x08);
            atk_mc5640_write_reg(0x3A11, 0x20);
            atk_mc5640_write_reg(0x3A1F, 0x10);
            break;
        }
        case ATK_MC5640_EXPOSURE_LEVEL_1:
        {
            atk_mc5640_write_reg(0x3A0F, 0x18);
            atk_mc5640_write_reg(0x3A10, 0x10);
            atk_mc5640_write_reg(0x3A1B, 0x18);
            atk_mc5640_write_reg(0x3A1E, 0x10);
            atk_mc5640_write_reg(0x3A11, 0x30);
            atk_mc5640_write_reg(0x3A1F, 0x10);
            break;
        }
        case ATK_MC5640_EXPOSURE_LEVEL_2:
        {
            atk_mc5640_write_reg(0x3A0F, 0x20);
            atk_mc5640_write_reg(0x3A10, 0x18);
            atk_mc5640_write_reg(0x3A11, 0x41);
            atk_mc5640_write_reg(0x3A1B, 0x20);
            atk_mc5640_write_reg(0x3A1E, 0x18);
            atk_mc5640_write_reg(0x3A1F, 0x10);
            break;
        }
        case ATK_MC5640_EXPOSURE_LEVEL_3:
        {
            atk_mc5640_write_reg(0x3A0F, 0x28);
            atk_mc5640_write_reg(0x3A10, 0x20);
            atk_mc5640_write_reg(0x3A11, 0x51);
            atk_mc5640_write_reg(0x3A1B, 0x28);
            atk_mc5640_write_reg(0x3A1E, 0x20);
            atk_mc5640_write_reg(0x3A1F, 0x10);
            break;
        }
        case ATK_MC5640_EXPOSURE_LEVEL_4:
        {
            atk_mc5640_write_reg(0x3A0F, 0x30);
            atk_mc5640_write_reg(0x3A10, 0x28);
            atk_mc5640_write_reg(0x3A11, 0x61);
            atk_mc5640_write_reg(0x3A1B, 0x30);
            atk_mc5640_write_reg(0x3A1E, 0x28);
            atk_mc5640_write_reg(0x3A1F, 0x10);
            break;
        }
        case ATK_MC5640_EXPOSURE_LEVEL_5:
        {
            atk_mc5640_write_reg(0x3A0F, 0x38);
            atk_mc5640_write_reg(0x3A10, 0x30);
            atk_mc5640_write_reg(0x3A11, 0x61);
            atk_mc5640_write_reg(0x3A1B, 0x38);
            atk_mc5640_write_reg(0x3A1E, 0x30);
            atk_mc5640_write_reg(0x3A1F, 0x10);
            break;
        }
        case ATK_MC5640_EXPOSURE_LEVEL_6:
        {
            atk_mc5640_write_reg(0x3A0F, 0x40);
            atk_mc5640_write_reg(0x3A10, 0x38);
            atk_mc5640_write_reg(0x3A11, 0x71);
            atk_mc5640_write_reg(0x3A1B, 0x40);
            atk_mc5640_write_reg(0x3A1E, 0x38);
            atk_mc5640_write_reg(0x3A1F, 0x10);
            break;
        }
        case ATK_MC5640_EXPOSURE_LEVEL_7:
        {
            atk_mc5640_write_reg(0x3A0F, 0x48);
            atk_mc5640_write_reg(0x3A10, 0x40);
            atk_mc5640_write_reg(0x3A11, 0x80);
            atk_mc5640_write_reg(0x3A1B, 0x48);
            atk_mc5640_write_reg(0x3A1E, 0x40);
            atk_mc5640_write_reg(0x3A1F, 0x20);
            break;
        }
        case ATK_MC5640_EXPOSURE_LEVEL_8:
        {
            atk_mc5640_write_reg(0x3A0F, 0x50);
            atk_mc5640_write_reg(0x3A10, 0x48);
            atk_mc5640_write_reg(0x3A11, 0x90);
            atk_mc5640_write_reg(0x3A1B, 0x50);
            atk_mc5640_write_reg(0x3A1E, 0x48);
            atk_mc5640_write_reg(0x3A1F, 0x20);
            break;
        }
        case ATK_MC5640_EXPOSURE_LEVEL_9:
        {
            atk_mc5640_write_reg(0x3A0F, 0x58);
            atk_mc5640_write_reg(0x3A10, 0x50);
            atk_mc5640_write_reg(0x3A11, 0x91);
            atk_mc5640_write_reg(0x3A1B, 0x58);
            atk_mc5640_write_reg(0x3A1E, 0x50);
            atk_mc5640_write_reg(0x3A1F, 0x20);
            break;
        }
        case ATK_MC5640_EXPOSURE_LEVEL_10:
        {
            atk_mc5640_write_reg(0x3A0F, 0x60);
            atk_mc5640_write_reg(0x3A10, 0x58);
            atk_mc5640_write_reg(0x3A11, 0xA0);
            atk_mc5640_write_reg(0x3A1B, 0x60);
            atk_mc5640_write_reg(0x3A1E, 0x58);
            atk_mc5640_write_reg(0x3A1F, 0x20);
            break;
        }
        default:
        {
            return ATK_MC5640_EINVAL;
        }
    }
    
    return ATK_MC5640_EOK;
}

/**
 * @brief       设置ATK-MC5640模块锐度
 * @param       contrast: ATK_MC5640_SHARPNESS_OFF  : Sharpness OFF
 *                        ATK_MC5640_SHARPNESS_1    : Sharpness 1
 *                        ATK_MC5640_SHARPNESS_2    : Sharpness 2
 *                        ATK_MC5640_SHARPNESS_3    : Sharpness 3
 *                        ATK_MC5640_SHARPNESS_4    : Sharpness 4
 *                        ATK_MC5640_SHARPNESS_5    : Sharpness 5
 *                        ATK_MC5640_SHARPNESS_6    : Sharpness 6
 *                        ATK_MC5640_SHARPNESS_7    : Sharpness 7
 *                        ATK_MC5640_SHARPNESS_8    : Sharpness 8
 *                        ATK_MC5640_SHARPNESS_AUTO : Sharpness Auto
 * @retval      ATK_MC5640_EOK   : 设置ATK-MC5640模块锐度成功
 *              ATK_MC5640_EINVAL: 传入参数错误
 */
uint8_t atk_mc5640_set_sharpness_level(atk_mc5640_sharpness_t sharpness)
{
    switch (sharpness)
    {
        case ATK_MC5640_SHARPNESS_OFF:
        {
            atk_mc5640_write_reg(0x5308, 0x65);
            atk_mc5640_write_reg(0x5302, 0x00);
            break;
        }
        case ATK_MC5640_SHARPNESS_1:
        {
            atk_mc5640_write_reg(0x5308, 0x65);
            atk_mc5640_write_reg(0x5302, 0x02);
            break;
        }
        case ATK_MC5640_SHARPNESS_2:
        {
            atk_mc5640_write_reg(0x5308, 0x65);
            atk_mc5640_write_reg(0x5302, 0x04);
            break;
        }
        case ATK_MC5640_SHARPNESS_3:
        {
            atk_mc5640_write_reg(0x5308, 0x65);
            atk_mc5640_write_reg(0x5302, 0x08);
            break;
        }
        case ATK_MC5640_SHARPNESS_4:
        {
            atk_mc5640_write_reg(0x5308, 0x65);
            atk_mc5640_write_reg(0x5302, 0x0C);
            break;
        }
        case ATK_MC5640_SHARPNESS_5:
        {
            atk_mc5640_write_reg(0x5308, 0x65);
            atk_mc5640_write_reg(0x5302, 0x10);
            break;
        }
        case ATK_MC5640_SHARPNESS_6:
        {
            atk_mc5640_write_reg(0x5308, 0x65);
            atk_mc5640_write_reg(0x5302, 0x14);
            break;
        }
        case ATK_MC5640_SHARPNESS_7:
        {
            atk_mc5640_write_reg(0x5308, 0x65);
            atk_mc5640_write_reg(0x5302, 0x18);
            break;
        }
        case ATK_MC5640_SHARPNESS_8:
        {
            atk_mc5640_write_reg(0x5308, 0x65);
            atk_mc5640_write_reg(0x5302, 0x20);
            break;
        }
        case ATK_MC5640_SHARPNESS_AUTO:
        {
            atk_mc5640_write_reg(0x5308, 0x25);
            atk_mc5640_write_reg(0x5300, 0x08);
            atk_mc5640_write_reg(0x5301, 0x30);
            atk_mc5640_write_reg(0x5302, 0x10);
            atk_mc5640_write_reg(0x5303, 0x00);
            atk_mc5640_write_reg(0x5309, 0x08);
            atk_mc5640_write_reg(0x530A, 0x30);
            atk_mc5640_write_reg(0x530B, 0x04);
            atk_mc5640_write_reg(0x530C, 0x06);
            break;
        }
        default:
        {
            return ATK_MC5640_EINVAL;
        }
    }
    
    return ATK_MC5640_EOK;
}

/**
 * @brief       设置ATK-MC5640模块镜像/翻转
 * @param       contrast: ATK_MC5640_MIRROR_FLIP_0: MIRROR
 *                        ATK_MC5640_MIRROR_FLIP_1: FLIP
 *                        ATK_MC5640_MIRROR_FLIP_2: MIRROR & FLIP
 *                        ATK_MC5640_MIRROR_FLIP_3: Normal
 * @retval      ATK_MC5640_EOK   : 设置ATK-MC5640模块镜像/翻转成功
 *              ATK_MC5640_EINVAL: 传入参数错误
 */
uint8_t atk_mc5640_set_mirror_flip(atk_mc5640_mirror_flip_t mirror_flip)
{
    uint8_t reg3820;
    uint8_t reg3821;
    
    switch (mirror_flip)
    {
        case ATK_MC5640_MIRROR_FLIP_0:
        {
            reg3820 = atk_mc5640_read_reg(0x3820);
            reg3820 = reg3820 & 0xF9;
            reg3820 = reg3820 | 0x00;
            atk_mc5640_write_reg(0x3820, reg3820);
            reg3821 = atk_mc5640_read_reg(0x3821);
            reg3821 = reg3821 & 0xF9;
            reg3821 = reg3821 | 0x06;
            atk_mc5640_write_reg(0x3821, reg3821);
            break;
        }
        case ATK_MC5640_MIRROR_FLIP_1:
        {
            reg3820 = atk_mc5640_read_reg(0x3820);
            reg3820 = reg3820 & 0xF9;
            reg3820 = reg3820 | 0x06;
            atk_mc5640_write_reg(0x3820, reg3820);
            reg3821 = atk_mc5640_read_reg(0x3821);
            reg3821 = reg3821 & 0xF9;
            reg3821 = reg3821 | 0x00;
            atk_mc5640_write_reg(0x3821, reg3821);
            break;
        }
        case ATK_MC5640_MIRROR_FLIP_2:
        {
            reg3820 = atk_mc5640_read_reg(0x3820);
            reg3820 = reg3820 & 0xF9;
            reg3820 = reg3820 | 0x06;
            atk_mc5640_write_reg(0x3820, reg3820);
            reg3821 = atk_mc5640_read_reg(0x3821);
            reg3821 = reg3821 & 0xF9;
            reg3821 = reg3821 | 0x06;
            atk_mc5640_write_reg(0x3821, reg3821);
            break;
        }
        case ATK_MC5640_MIRROR_FLIP_3:
        {
            reg3820 = atk_mc5640_read_reg(0x3820);
            reg3820 = reg3820 & 0xF9;
            reg3820 = reg3820 | 0x00;
            atk_mc5640_write_reg(0x3820, reg3820);
            reg3821 = atk_mc5640_read_reg(0x3821);
            reg3821 = reg3821 & 0xF9;
            reg3821 = reg3821 | 0x00;
            atk_mc5640_write_reg(0x3821, reg3821);
            break;
        }
        default:
        {
            return ATK_MC5640_EINVAL;
        }
    }
    
    return ATK_MC5640_EOK;
}

/**
 * @brief       设置ATK-MC5640模块测试图案
 * @param       contrast: ATK_MC5640_TEST_PATTERN_OFF         : OFF
 *                        ATK_MC5640_TEST_PATTERN_COLOR_BAR   : Color bar
 *                        ATK_MC5640_TEST_PATTERN_COLOR_SQUARE: Color square
 * @retval      ATK_MC5640_EOK   : 设置ATK-MC5640模块测试图案成功
 *              ATK_MC5640_EINVAL: 传入参数错误
 */
uint8_t atk_mc5640_set_test_pattern(atk_mc5640_test_pattern_t pattern)
{
    switch (pattern)
    {
        case ATK_MC5640_TEST_PATTERN_OFF:
        {
            atk_mc5640_write_reg(0x503D, 0x00);
            atk_mc5640_write_reg(0x4741, 0x00);
            break;
        }
        case ATK_MC5640_TEST_PATTERN_COLOR_BAR:
        {
            atk_mc5640_write_reg(0x503D, 0x80);
            atk_mc5640_write_reg(0x4741, 0x00);
            break;
        }
        case ATK_MC5640_TEST_PATTERN_COLOR_SQUARE:
        {
            atk_mc5640_write_reg(0x503D, 0x82);
            atk_mc5640_write_reg(0x4741, 0x00);
            break;
        }
        default:
        {
            return ATK_MC5640_EINVAL;
        }
    }
    
    return ATK_MC5640_EOK;
}

/**
 * @brief       设置ATK-MC5640模块输出图像格式
 * @param       mode: ATK_MC5640_OUTPUT_FORMAT_RGB565: RGB565
 *                    ATK_MC5640_OUTPUT_FORMAT_JPEG  : JPEG
 * @retval      ATK_MC5640_EOK   : 设置ATK-MC5640模块输出图像格式成功
 *              ATK_MC5640_EINVAL: 传入参数错误
 */
uint8_t atk_mc5640_set_output_format(atk_mc5640_output_format_t format)
{
    uint32_t cfg_index;
    
    switch (format)
    {
        case ATK_MC5640_OUTPUT_FORMAT_RGB565:
        {
            for (cfg_index=0; cfg_index<sizeof(atk_mc5640_rgb565_cfg)/sizeof(atk_mc5640_rgb565_cfg[0]); cfg_index++)
            {
                atk_mc5640_write_reg(atk_mc5640_rgb565_cfg[cfg_index].reg, atk_mc5640_rgb565_cfg[cfg_index].dat);
            }
            break;
        }
        case ATK_MC5640_OUTPUT_FORMAT_JPEG:
        {
            for (cfg_index=0; cfg_index<sizeof(atk_mc5640_jpeg_cfg)/sizeof(atk_mc5640_jpeg_cfg[0]); cfg_index++)
            {
                atk_mc5640_write_reg(atk_mc5640_jpeg_cfg[cfg_index].reg, atk_mc5640_jpeg_cfg[cfg_index].dat);
            }
            break;
        }
        default:
        {
            return ATK_MC5640_EINVAL;
        }
    }
    
    return ATK_MC5640_EOK;
}

/**
 * @brief       设置ATK-MC5640模块ISP输入窗口尺寸
 * @note        ATK-MC5640模块ISP输入窗口的最大尺寸为0x0A3F*0x079F
 * @param       x     : ISP输入窗口起始X坐标
 *              y     : ISP输入窗口起始Y坐标
 *              width : ISP输入窗口宽度
 *              height: ISP输入窗口高度
 * @retval      ATK_MC5640_EOK   : 设置ATK-MC5640模块ISP输入窗口尺寸成功
 *              ATK_MC5640_EINVAL: 传入参数错误
 */
uint8_t atk_mc5640_set_isp_input_window(uint16_t x, uint16_t y, uint16_t width, uint16_t height)
{
    uint8_t reg3800;
    uint8_t reg3801;
    uint8_t reg3802;
    uint8_t reg3803;
    uint8_t reg3804;
    uint8_t reg3805;
    uint8_t reg3806;
    uint8_t reg3807;
    uint16_t x_end;
    uint16_t y_end;
    
    x_end = x + width;
    y_end = y + height;
    
    if ((x_end > ATK_MC5640_ISP_INPUT_WIDTH_MAX) || (y_end > ATK_MC5640_ISP_INPUT_HEIGHT_MAX))
    {
        return ATK_MC5640_EINVAL;
    }
    
    reg3800 = atk_mc5640_read_reg(0x3800);
    reg3802 = atk_mc5640_read_reg(0x3802);
    reg3804 = atk_mc5640_read_reg(0x3804);
    reg3806 = atk_mc5640_read_reg(0x3806);
    
    reg3800 &= ~0x0F;
    reg3800 |= (uint8_t)(x >> 8) & 0x0F;
    reg3801 = (uint8_t)x & 0xFF;
    reg3802 &= ~0x0F;
    reg3802 |= (uint8_t)(y >> 8) & 0x0F;
    reg3803 = (uint8_t)y & 0xFF;
    reg3804 &= ~0x0F;
    reg3804 |= (uint8_t)(x_end >> 8) & 0x0F;
    reg3805 = (uint8_t)x_end & 0xFF;
    reg3806 &= ~0x07;
    reg3806 |= (uint8_t)(y_end >> 8) & 0x07;
    reg3807 = (uint8_t)y_end & 0xFF;
    
    atk_mc5640_write_reg(0x3212, 0x03);
    atk_mc5640_write_reg(0x3800, reg3800);
    atk_mc5640_write_reg(0x3801, reg3801);
    atk_mc5640_write_reg(0x3802, reg3802);
    atk_mc5640_write_reg(0x3803, reg3803);
    atk_mc5640_write_reg(0x3804, reg3804);
    atk_mc5640_write_reg(0x3805, reg3805);
    atk_mc5640_write_reg(0x3806, reg3806);
    atk_mc5640_write_reg(0x3807, reg3807);
    atk_mc5640_write_reg(0x3212, 0x13);
    atk_mc5640_write_reg(0x3212, 0xA3);
    
    atk_mc5640_get_isp_input_size();
    
    return ATK_MC5640_EOK;
}

/**
 * @brief       设置ATK-MC5640模块预缩放窗口偏移
 * @note        ATK-MC5640模块预缩放窗口尺寸必须小于ISP输入窗口尺寸
 * @param       x_offset: 预缩放窗口X偏移
 *              y_offset: 预缩放窗口Y偏移
 * @retval      ATK_MC5640_EOK   : 设置ATK-MC5640模块预缩放窗口偏移成功
 */
uint8_t atk_mc5640_set_pre_scaling_window(uint16_t x_offset, uint16_t y_offset)
{
    uint8_t reg3810;
    uint8_t reg3811;
    uint8_t reg3812;
    uint8_t reg3813;
    
    reg3810 = (uint8_t)(x_offset >> 8) & 0x0F;
    reg3811 = (uint8_t)x_offset & 0xFF;
    reg3812 = (uint8_t)(y_offset >> 8) & 0x07;
    reg3813 = (uint8_t)y_offset & 0xFF;
    
    atk_mc5640_write_reg(0x3212, 0x03);
    atk_mc5640_write_reg(0x3810, reg3810);
    atk_mc5640_write_reg(0x3811, reg3811);
    atk_mc5640_write_reg(0x3812, reg3812);
    atk_mc5640_write_reg(0x3813, reg3813);
    atk_mc5640_write_reg(0x3212, 0x13);
    atk_mc5640_write_reg(0x3212, 0xA3);
    
    atk_mc5640_get_pre_scaling_size();
    
    return ATK_MC5640_EOK;
}

/**
 * @brief       设置ATK-MC5640模块输出图像尺寸
 * @param       width : 实际输出图像的宽度（可能被缩放）
 *              height: 实际输出图像的高度（可能被缩放）
 * @retval      ATK_MC5640_EOK   : 设置ATK-MC5640模块输出图像窗口成功
 */
uint8_t atk_mc5640_set_output_size(uint16_t width, uint16_t height)
{
    uint8_t reg3808;
    uint8_t reg3809;
    uint8_t reg380A;
    uint8_t reg380B;
    
    reg3808 = atk_mc5640_read_reg(0x3808);
    reg380A = atk_mc5640_read_reg(0x380A);
    
    reg3808 &= ~0x0F;
    reg3808 |= (uint8_t)(width >> 8) & 0x0F;
    reg3809 = (uint8_t)width & 0xFF;
    reg380A &= ~0x07;
    reg380A |= (uint8_t)(height >> 8) & 0x07;
    reg380B = (uint8_t)height & 0xFF;
    
    atk_mc5640_write_reg(0x3212, 0x03);
    atk_mc5640_write_reg(0x3808, reg3808);
    atk_mc5640_write_reg(0x3809, reg3809);
    atk_mc5640_write_reg(0x380A, reg380A);
    atk_mc5640_write_reg(0x380B, reg380B);
    atk_mc5640_write_reg(0x3212, 0x13);
    atk_mc5640_write_reg(0x3212, 0xA3);
    
    atk_mc5640_get_output_size();
    
    return ATK_MC5640_EOK;
}

/**
 * @brief       获取ATK-MC5640模块输出的一帧图像数据
 * @param       dts_addr : 帧数据的接收缓冲的首地址
 *              type: ATK_MC5640_GET_TYPE_DTS_8B_NOINC : 图像数据以字节方式写入目的地址，目的地址固定不变
 *                    ATK_MC5640_GET_TYPE_DTS_8B_INC   : 图像数据以字节方式写入目的地址，目的地址自动增加
 *                    ATK_MC5640_GET_TYPE_DTS_16B_NOINC: 图像数据以半字方式写入目的地址，目的地址固定不变
 *                    ATK_MC5640_GET_TYPE_DTS_16B_INC  : 图像数据以半字方式写入目的地址，目的地址自动增加
 *                    ATK_MC5640_GET_TYPE_DTS_32B_NOINC: 图像数据以字方式写入目的地址，目的地址固定不变
 *                    ATK_MC5640_GET_TYPE_DTS_32B_INC  : 图像数据以字方式写入目的地址，目的地址自动增加
 *              before_transfer: 帧数据传输前，需要完成的事务，可为NULL
 * @retval      ATK_MC5640_EOK   : 获取ATK-MC5640模块输出的一帧图像数据成功
 *              ATK_MC5640_EINVAL: 传入参数错误
 *              ATK_MC5640_EEMPTY: 图像数据为空
 */
uint8_t atk_mc5640_get_frame(uint32_t dts_addr, atk_mc5640_get_type_t type, void (*before_transfer)(void))
{
    uint32_t meminc;
    uint32_t memdataalignment;
    uint32_t len;
    
    switch (type)
    {
        case ATK_MC5640_GET_TYPE_DTS_8B_NOINC:
        {
            meminc = DMA_MINC_DISABLE;
            memdataalignment = DMA_MDATAALIGN_BYTE;
            len = (g_atk_mc5640_sta.output.width * g_atk_mc5640_sta.output.height) / sizeof(uint8_t);
            break;
        }
        case ATK_MC5640_GET_TYPE_DTS_8B_INC:
        {
            meminc = DMA_MINC_ENABLE;
            memdataalignment = DMA_MDATAALIGN_BYTE;
            len = (g_atk_mc5640_sta.output.width * g_atk_mc5640_sta.output.height) / sizeof(uint8_t);
            break;
        }
        case ATK_MC5640_GET_TYPE_DTS_16B_NOINC:
        {
            meminc = DMA_MINC_DISABLE;
            memdataalignment = DMA_MDATAALIGN_HALFWORD;
            len = (g_atk_mc5640_sta.output.width * g_atk_mc5640_sta.output.height) / sizeof(uint16_t);
            break;
        }
        case ATK_MC5640_GET_TYPE_DTS_16B_INC:
        {
            meminc = DMA_MINC_ENABLE;
            memdataalignment = DMA_MDATAALIGN_HALFWORD;
            len = (g_atk_mc5640_sta.output.width * g_atk_mc5640_sta.output.height) / sizeof(uint16_t);
            break;
        }
        case ATK_MC5640_GET_TYPE_DTS_32B_NOINC:
        {
            meminc = DMA_MINC_DISABLE;
            memdataalignment = DMA_MDATAALIGN_WORD;
            len = (g_atk_mc5640_sta.output.width * g_atk_mc5640_sta.output.height) / sizeof(uint32_t);
            break;
        }
        case ATK_MC5640_GET_TYPE_DTS_32B_INC:
        {
            meminc = DMA_MINC_ENABLE;
            memdataalignment = DMA_MDATAALIGN_WORD;
            len = (g_atk_mc5640_sta.output.width * g_atk_mc5640_sta.output.height) / sizeof(uint32_t);
            break;
        }
        default:
        {
            return ATK_MC5640_EINVAL;
        }
    }
    
    if (before_transfer != NULL)
    {
        before_transfer();
    }
    
    atk_mc5640_dcmi_start(dts_addr, meminc, memdataalignment, len);
    
    return ATK_MC5640_EOK;
}
