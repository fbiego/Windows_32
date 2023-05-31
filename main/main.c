#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "rom/gpio.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "lvgl.h"
#include "ui.h"
#include "board.h"
#include "esp_timer.h"
#include "aw9523.h"

#define TAG "MAIN"


static void increase_lvgl_tick(void* arg) {
    lv_tick_inc(portTICK_PERIOD_MS);
}

extern void screen_init(void);

void lvgl_task(void* arg) {
    screen_init();

    // Tick interface for LVGL
    const esp_timer_create_args_t periodic_timer_args = {
        .callback = increase_lvgl_tick,
        .name = "periodic_gui"
    };
    esp_timer_handle_t periodic_timer;
    esp_timer_create(&periodic_timer_args, &periodic_timer);
    esp_timer_start_periodic(periodic_timer, portTICK_PERIOD_MS * 1000);


    ui_init();

    for (;;) {
        lv_task_handler();
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void app_main(void) {
    /*
     * init aw9523
     */
    aw9523_softreset();
    aw9523_init(TOUCH_IIC_SDA, TOUCH_IIC_SDA);

    xTaskCreatePinnedToCore(lvgl_task, NULL, 8 * 1024, NULL, 5, NULL, 1);
}
