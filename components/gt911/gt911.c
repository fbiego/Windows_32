#include <stdio.h>
#include "stdatomic.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_err.h"
#include "esp_log.h"
#include "i2c_device.h"
#include "gt911.h"

#define TAG "GT911"

static I2CDevice_t gt911_device;

gt911_status_t gt911_status;

//TODO: handle multibyte read and refactor to just one read transaction
esp_err_t gt911_i2c_read(uint16_t register_addr, uint8_t *data_buf, uint8_t len) {
    return i2c_read_bytes(gt911_device, register_addr, data_buf, len);
}

esp_err_t gt911_i2c_write8(uint16_t register_addr, uint8_t data) {
    uint8_t buffer = data;
    return i2c_write_bytes(gt911_device, register_addr, &buffer, 1);
}

void gt911_init(int8_t sda, int8_t scl, int8_t intr_pin, uint8_t dev_addr)
{
    if (!gt911_status.inited) {
        gt911_status.i2c_dev_addr = dev_addr;
        uint8_t data_buf;
        esp_err_t ret;

        gt911_device = i2c_malloc_device(I2C_NUM_0, sda, scl, 400000, dev_addr);

        i2c_device_set_reg_bits(gt911_device, I2C_REG_16BIT_BIG);
        ESP_LOGI(TAG, "Checking for GT911 Touch Controller");
        if ((ret = gt911_i2c_read(GT911_PRODUCT_ID1, &data_buf, 1) != ESP_OK)) {
            ESP_LOGE(TAG, "Error reading from device: %s",
                        esp_err_to_name(ret));    // Only show error the first time
            return;
        }

        // Read 4 bytes for Product ID in ASCII
        for (int i = 0; i < GT911_PRODUCT_ID_LEN; i++) {
            gt911_i2c_read((GT911_PRODUCT_ID1 + i), (uint8_t *)&(gt911_status.product_id[i]), 1);
        }
        ESP_LOGI(TAG, "\tProduct ID: %s", gt911_status.product_id);

        gt911_i2c_read(GT911_VENDOR_ID, &data_buf, 1);
        ESP_LOGI(TAG, "\tVendor ID: 0x%02x", data_buf);

        gt911_i2c_read(GT911_X_COORD_RES_L, &data_buf, 1);
        gt911_status.max_x_coord = data_buf;
        gt911_i2c_read(GT911_X_COORD_RES_H, &data_buf, 1);
        gt911_status.max_x_coord |= ((uint16_t)data_buf << 8);
        ESP_LOGI(TAG, "\tX Resolution: %d", gt911_status.max_x_coord);

        gt911_i2c_read(GT911_Y_COORD_RES_L, &data_buf, 1);
        gt911_status.max_y_coord = data_buf;
        gt911_i2c_read(GT911_Y_COORD_RES_H, &data_buf, 1);
        gt911_status.max_y_coord |= ((uint16_t)data_buf << 8);
        ESP_LOGI(TAG, "\tY Resolution: %d", gt911_status.max_y_coord);
        gt911_status.inited = true;
    }
}

void gt911_read_status(uint16_t* x, uint16_t* y, uint8_t* touched) 
{
    uint8_t touch_pnt_cnt;        // Number of detected touch points
    static int16_t last_x = 0;  // 12bit pixel value
    static int16_t last_y = 0;  // 12bit pixel value
    uint8_t data_buf;
    uint8_t status_reg;

    gt911_i2c_read(GT911_STATUS_REG, &status_reg, 1);
    touch_pnt_cnt = status_reg & 0x0F;
    if ((status_reg & 0x80) || (touch_pnt_cnt < 6)) {
        //Reset Status Reg Value
        gt911_i2c_write8(GT911_STATUS_REG, 0x00);
    }

    if (touch_pnt_cnt != 1) {    // ignore no touch & multi touch
        *x = last_x;
        *y = last_y;
        *touched = 0;
        return;
    }

    gt911_i2c_read(GT911_PT1_X_COORD_L, &data_buf, 1);
    last_x = data_buf;
    gt911_i2c_read(GT911_PT1_X_COORD_H, &data_buf, 1);
    last_x |= ((uint16_t)data_buf << 8);

    gt911_i2c_read(GT911_PT1_Y_COORD_L, &data_buf, 1);
    last_y = data_buf;
    gt911_i2c_read(GT911_PT1_Y_COORD_H, &data_buf, 1);
    last_y |= ((uint16_t)data_buf << 8);

    *x = last_x;
    *y = last_y;
    *touched = 1;
}

