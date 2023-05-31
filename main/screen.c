#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "soc/soc_caps.h"
#include "esp_freertos_hooks.h"
#include "esp_log.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_interface.h"
#include "esp_lcd_panel_rgb.h"
#include "driver/gpio.h"
#include "board.h"
#include "lvgl.h"
#include "esp_timer.h"
#include "gt911.h"

#define TAG "QMSD BOARD INIT"

static esp_lcd_panel_handle_t g_panel_handle = NULL;

void touchpad_gt911_read(lv_indev_drv_t *indev_drv, lv_indev_data_t *data)
{
    uint16_t tp_x,tp_y;
    uint8_t press;
    gt911_read_status(&tp_x,&tp_y,&press);
    data->state = press;
	data->point.x = tp_x;
	data->point.y = tp_y;
}

static void __qmsd_touch_init(void)
{
    gt911_init(TOUCH_IIC_SDA, TOUCH_IIC_SCL, -1, 0x5D);
    static lv_indev_drv_t indev_drv;               /*Descriptor of an input device driver*/
    lv_indev_drv_init(&indev_drv);          /*Basic initialization*/

    indev_drv.type = LV_INDEV_TYPE_POINTER; /*The touchpad is pointer type device*/
    indev_drv.read_cb = touchpad_gt911_read;            /*Library ready your touchpad via this function*/

    lv_indev_drv_register(&indev_drv);      /*Finally register the driver*/
}

static void __qsmd_rgb_disp_flush(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p)
{
    int offsetx1 = area->x1;
    int offsetx2 = area->x2;
    int offsety1 = area->y1;
    int offsety2 = area->y2;

    esp_lcd_panel_draw_bitmap(g_panel_handle, offsetx1, offsety1, offsetx2 + 1, offsety2 + 1, color_p);
    lv_disp_flush_ready(disp_drv);
}

void qmsd_rgb_init(esp_lcd_rgb_panel_config_t *panel_config)
{
    static lv_disp_drv_t disp_drv;
    int buffer_size;
    void *buf1 = NULL;
    void *buf2 = NULL;
    static lv_disp_draw_buf_t draw_buf;

    lv_init();

    ESP_ERROR_CHECK(esp_lcd_new_rgb_panel(panel_config, &g_panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_reset(g_panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_init(g_panel_handle));

    buffer_size = panel_config->timings.h_res * panel_config->timings.v_res;
    esp_lcd_rgb_panel_get_frame_buffer(g_panel_handle, 2, &buf1, &buf2);
    lv_disp_draw_buf_init(&draw_buf, buf1, buf2, buffer_size);

    lv_disp_drv_init(&disp_drv);
    disp_drv.flush_cb = __qsmd_rgb_disp_flush;
    disp_drv.draw_buf = &draw_buf;
    disp_drv.hor_res = panel_config->timings.h_res;
    disp_drv.ver_res = panel_config->timings.v_res;
    disp_drv.full_refresh = 1;

    lv_disp_drv_register(&disp_drv);
}

void screen_init(void)
{
    gpio_config_t bk_gpio_config = {
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = 1ULL << LCD_PIN_BK_LIGHT
    };
    // Initialize the GPIO of backlight
    ESP_ERROR_CHECK(gpio_config(&bk_gpio_config));

    ESP_ERROR_CHECK(gpio_set_level(LCD_PIN_BK_LIGHT, LCD_BK_LIGHT_OFF_LEVEL));

    if (LCD_PIN_RST > -1) {
        gpio_config_t rst_gpio_config = {
            .mode = GPIO_MODE_OUTPUT,
            .pin_bit_mask = 1ULL << LCD_PIN_RST
        };
        // Initialize the GPIO of rst
        ESP_ERROR_CHECK(gpio_config(&rst_gpio_config));

        ESP_ERROR_CHECK(gpio_set_level(LCD_PIN_RST, 1));
        vTaskDelay(pdMS_TO_TICKS(100));
        ESP_ERROR_CHECK(gpio_set_level(LCD_PIN_RST, 0));
        vTaskDelay(pdMS_TO_TICKS(100));
        ESP_ERROR_CHECK(gpio_set_level(LCD_PIN_RST, 1));
        vTaskDelay(pdMS_TO_TICKS(100));
    }

    esp_lcd_rgb_panel_config_t panel_config = {
        .data_width = 16,
        .psram_trans_align = 64,
        .pclk_gpio_num = LCD_PCLK_GPIO,
        .vsync_gpio_num = LCD_VSYNC_GPIO,
        .hsync_gpio_num = LCD_HSYNC_GPIO,
        .de_gpio_num = LCD_DE_GPIO,
        .disp_gpio_num = LCD_DISP_EN_GPIO,
        .data_gpio_nums = {
            LCD_D0_GPIO,
            LCD_D1_GPIO,
            LCD_D2_GPIO,
            LCD_D3_GPIO,
            LCD_D4_GPIO,
            LCD_D5_GPIO,
            LCD_D6_GPIO,
            LCD_D7_GPIO,
            LCD_D8_GPIO,
            LCD_D9_GPIO,
            LCD_D10_GPIO,
            LCD_D11_GPIO,
            LCD_D12_GPIO,
            LCD_D13_GPIO,
            LCD_D14_GPIO,
            LCD_D15_GPIO,
        },
        .timings = {
            .pclk_hz = 24000000,
            .h_res = 800,
            .v_res = 480,
            .hsync_pulse_width = 10,
            .hsync_back_porch = 40,
            .hsync_front_porch = 40,    // 890
            .vsync_pulse_width = 6,
            .vsync_back_porch = 31,
            .vsync_front_porch = 18,    // 535
        },
        .flags.fb_in_psram = 1,
        .flags.double_fb = 1,
        .flags.refresh_on_demand = 0,   // Mannually control refresh operation
        .bounce_buffer_size_px = 16 * 800,
        .clk_src = LCD_CLK_SRC_PLL160M,
    };

    qmsd_rgb_init(&panel_config);
    __qmsd_touch_init();

    ESP_ERROR_CHECK(gpio_set_level(LCD_PIN_BK_LIGHT, LCD_BK_LIGHT_ON_LEVEL));
}