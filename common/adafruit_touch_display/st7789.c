/**
 * Copyright (c) 2017 - 2018, Nordic Semiconductor ASA
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form, except as embedded into a Nordic
 *    Semiconductor ASA integrated circuit in a product or a software update for
 *    such product, must reproduce the above copyright notice, this list of
 *    conditions and the following disclaimer in the documentation and/or other
 *    materials provided with the distribution.
 *
 * 3. Neither the name of Nordic Semiconductor ASA nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * 4. This software, with or without modification, must only be used with a
 *    Nordic Semiconductor ASA integrated circuit.
 *
 * 5. Any software provided in binary form under this license must not be reverse
 *    engineered, decompiled, modified and/or disassembled.
 *
 * THIS SOFTWARE IS PROVIDED BY NORDIC SEMICONDUCTOR ASA "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL NORDIC SEMICONDUCTOR ASA OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include "sdk_common.h"

#if NRF_MODULE_ENABLED(ST7789)

#include "nrf_lcd_ext.h"
#include "nrf_drv_spi.h"
#include "nrf_delay.h"
#include "nrf_gpio.h"
#include "boards.h"

// Set of commands described in ST7789 data sheet.
#define ST7789_NOP     0x00
#define ST7789_SWRESET 0x01
#define ST7789_RDDID   0x04
#define ST7789_RDDST   0x09

#define ST7789_SLPIN   0x10
#define ST7789_SLPOUT  0x11
#define ST7789_PTLON   0x12
#define ST7789_NORON   0x13

#define ST7789_INVOFF  0x20
#define ST7789_INVON   0x21
#define ST7789_DISPOFF 0x28
#define ST7789_DISPON  0x29
#define ST7789_CASET   0x2A
#define ST7789_RASET   0x2B
#define ST7789_RAMWR   0x2C
#define ST7789_RAMRD   0x2E

#define ST7789_PTLAR   0x30
#define ST7789_COLMOD  0x3A
#define ST7789_MADCTL  0x36

#define ST7789_FRMCTR1 0xB1
#define ST7789_FRMCTR2 0xB2
#define ST7789_FRMCTR3 0xB3
#define ST7789_INVCTR  0xB4
#define ST7789_DISSET5 0xB6

#define ST7789_PWCTR1  0xC0
#define ST7789_PWCTR2  0xC1
#define ST7789_PWCTR3  0xC2
#define ST7789_PWCTR4  0xC3
#define ST7789_PWCTR5  0xC4
#define ST7789_VMCTR1  0xC5

#define ST7789_RDID1   0xDA
#define ST7789_RDID2   0xDB
#define ST7789_RDID3   0xDC
#define ST7789_RDID4   0xDD

#define ST7789_PWCTR6  0xFC

#define ST7789_GMCTRP1 0xE0
#define ST7789_GMCTRN1 0xE1

#define ST7789_MADCTL_MY  0x80
#define ST7789_MADCTL_MX  0x40
#define ST7789_MADCTL_MV  0x20
#define ST7789_MADCTL_ML  0x10
#define ST7789_MADCTL_RGB 0x00
#define ST7789_MADCTL_BGR 0x08
#define ST7789_MADCTL_MH  0x04
/* @} */

#define RGB2BGR(x)      (x << 11) | (x & 0x07E0) | (x >> 11)

static const nrf_drv_spi_t spi = NRF_DRV_SPI_INSTANCE(ST7789_SPI_INSTANCE);  /**< SPI instance. */

/**
 * @brief Structure holding ST7789 controller basic parameters.
 */
typedef struct
{
    uint8_t tab_color;      /**< Color of tab attached to the used screen. */
}st7789_t;

/**
 * @brief Enumerator with TFT tab colors.
 */
typedef enum{
    INITR_GREENTAB = 0,     /**< Green tab. */
    INITR_REDTAB,           /**< Red tab. */
    INITR_BLACKTAB,         /**< Black tab. */
    INITR_144GREENTAB       /**< Green tab, 1.44" display. */
}st7789_tab_t;

static st7789_t m_st7789;

static inline void spi_write(const void * data, size_t size)
{
    APP_ERROR_CHECK(nrf_drv_spi_transfer(&spi, data, size, NULL, 0));
}

static inline void write_command(uint8_t c)
{
    nrf_gpio_pin_clear(ST7789_DC_PIN);
    spi_write(&c, sizeof(c));
}

static inline void write_data(uint8_t c)
{
    nrf_gpio_pin_set(ST7789_DC_PIN);
    spi_write(&c, sizeof(c));
}

static void set_addr_window(uint16_t x_0, uint16_t y_0, uint16_t x_1, uint16_t y_1)
{
    ASSERT(x_0 <= x_1);
    ASSERT(y_0 <= y_1);

    write_command(ST7789_CASET);
    write_data(x_0 >> 8);
    write_data(x_0);
    write_data(x_1 >> 8);
    write_data(x_1);
    write_command(ST7789_RASET);
    write_data(y_0 >> 8);
    write_data(y_0);
    write_data(y_1 >> 8);
    write_data(y_1);
    write_command(ST7789_RAMWR);
}

#define ST7789_240x240_XSTART 0
#define ST7789_240x240_YSTART 80

static void command_list(void)
{
    write_command(ST7789_SWRESET);
    nrf_delay_ms(150);    
    write_command(ST7789_SLPOUT);
    nrf_delay_ms(500);

    write_command(ST7789_COLMOD);
    write_data(0x55);
    nrf_delay_ms(10);

    write_command(ST7789_MADCTL);
    write_data(0x00);
    
    write_command(ST7789_CASET);
    write_data(0x00);
    write_data(ST7789_240x240_XSTART);
    write_data((240+ST7789_240x240_XSTART)>>8);
    write_data((240+ST7789_240x240_XSTART)&0xFF);  //     XEND = 240

    write_command(ST7789_RASET);
    write_data(0x00);
    write_data(ST7789_240x240_YSTART);             //     YSTART = 0
    write_data((240+ST7789_240x240_YSTART)>>8);
    write_data((240+ST7789_240x240_YSTART)&0xFF);  //     YEND = 240
    
    write_command(ST7789_INVON);//   ,  //  7: hack
    nrf_delay_ms(10);
    write_command(ST7789_NORON);    //  8: Normal display on, no args, w/delay
    nrf_delay_ms(10);                           //     10 ms delay
    write_command(ST7789_DISPON); //  9: Main screen turn on, no args, delay
    nrf_delay_ms(500);   

}


static ret_code_t hardware_init(void)
{
    ret_code_t err_code;

    nrf_gpio_cfg_output(ST7789_DC_PIN);

    nrf_drv_spi_config_t spi_config = NRF_DRV_SPI_DEFAULT_CONFIG;

    spi_config.sck_pin  = ST7789_SCK_PIN;
    spi_config.miso_pin = ST7789_MISO_PIN;
    spi_config.mosi_pin = ST7789_MOSI_PIN;
    spi_config.ss_pin   = ST7789_SS_PIN;

    err_code = nrf_drv_spi_init(&spi, &spi_config, NULL, NULL);
    return err_code;
}

static ret_code_t st7789_init(void)
{
    ret_code_t err_code;

    m_st7789.tab_color = ST7789_TAB_COLOR;

    err_code = hardware_init();
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    command_list();

    return err_code;
}

static void st7789_uninit(void)
{
    nrf_drv_spi_uninit(&spi);
}

static void st7789_pixel_draw(uint16_t x, uint16_t y, uint32_t color)
{
    set_addr_window(x, y, x, y);

    const uint8_t data[2] = {color >> 8, color};

    nrf_gpio_pin_set(ST7789_DC_PIN);

    spi_write(data, sizeof(data));

    nrf_gpio_pin_clear(ST7789_DC_PIN);
}

static void st7789_rect_draw(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint32_t color)
{
    static uint8_t data[32];
    
    set_addr_window(x, y, x + width - 1, y + height - 1);

    for(int i = 0; i < 16; i++)
    {
       data[i*2] = color >> 8;
       data[i*2+1] = color;
    }

    nrf_gpio_pin_set(ST7789_DC_PIN);

    uint32_t bytes_left = height * width * 2;

    do
    {
        if(bytes_left > 32)
        {
            spi_write(data, sizeof(data));
            bytes_left -= 16;
        }
        else
        {
           spi_write(data, bytes_left);
           bytes_left = 0;
        }
    
    }while(bytes_left > 0); 

    nrf_gpio_pin_clear(ST7789_DC_PIN);
}

static void st7789_buffer_draw(uint16_t x, uint16_t y, uint16_t width, uint16_t height, void * p_data, uint32_t length)
{
    uint32_t spi_max_length, spi_length;
    uint8_t *data_ptr = (uint8_t *)p_data;
    
    set_addr_window(x, y, x + width - 1, y + height - 1);

    nrf_gpio_pin_set(ST7789_DC_PIN);
    spi_max_length = 250; //(1 << SPIM0_EASYDMA_MAXCNT_SIZE - 4))
    do
    {
        spi_length = (length > spi_max_length) ? spi_max_length : length;
        spi_write(data_ptr, spi_length);
        length -= spi_length;
        data_ptr += spi_length;        
    }while(length > 0);

    nrf_gpio_pin_clear(ST7789_DC_PIN);    
}

static void st7789_dummy_display(void)
{
    /* No implementation needed. */
}

static void st7789_rotation_set(nrf_lcd_rotation_t rotation)
{
    write_command(ST7789_MADCTL);
    switch (rotation) {
        case NRF_LCD_ROTATE_0:
            if (m_st7789.tab_color == INITR_BLACKTAB)
            {
                write_data(ST7789_MADCTL_MX | ST7789_MADCTL_MY | ST7789_MADCTL_RGB);
            }
            else
            {
                write_data(ST7789_MADCTL_MX | ST7789_MADCTL_MY | ST7789_MADCTL_BGR);
            }
            break;
        case NRF_LCD_ROTATE_90:
            if (m_st7789.tab_color == INITR_BLACKTAB)
            {
                write_data(ST7789_MADCTL_MY | ST7789_MADCTL_MV | ST7789_MADCTL_RGB);
            }
            else
            {
                write_data(ST7789_MADCTL_MY | ST7789_MADCTL_MV | ST7789_MADCTL_BGR);
            }
            break;
        case NRF_LCD_ROTATE_180:
            if (m_st7789.tab_color == INITR_BLACKTAB)
            {
                write_data(ST7789_MADCTL_RGB);
            }
            else
            {
                write_data(ST7789_MADCTL_BGR);
            }
            break;
        case NRF_LCD_ROTATE_270:
            if (m_st7789.tab_color == INITR_BLACKTAB)
            {
                write_data(ST7789_MADCTL_MX | ST7789_MADCTL_MV | ST7789_MADCTL_RGB);
            }
            else
            {
                write_data(ST7789_MADCTL_MX | ST7789_MADCTL_MV | ST7789_MADCTL_BGR);
            }
            break;
        default:
            break;
    }
}


static void st7789_display_invert(bool invert)
{
    write_command(invert ? ST7789_INVON : ST7789_INVOFF);
}

static lcd_cb_t st7789_cb = {
    .height = ST7789_HEIGHT,
    .width = ST7789_WIDTH
};

const nrf_lcd_t nrf_lcd_st7789 = {
    .lcd_init = st7789_init,
    .lcd_uninit = st7789_uninit,
    .lcd_pixel_draw = st7789_pixel_draw,
    .lcd_rect_draw = st7789_rect_draw,
    .lcd_buffer_draw = st7789_buffer_draw,
    .lcd_display = st7789_dummy_display,
    .lcd_rotation_set = st7789_rotation_set,
    .lcd_display_invert = st7789_display_invert,
    .p_lcd_cb = &st7789_cb
};

#endif // NRF_MODULE_ENABLED(ST7789)
