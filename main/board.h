#pragma once

#define LCD_VSYNC_GPIO    (38)
#define LCD_HSYNC_GPIO    (5)
#define LCD_DE_GPIO       (39)
#define LCD_PCLK_GPIO     (9)
#define LCD_D0_GPIO         (17)    // B0
#define LCD_D1_GPIO         (16)    // B1
#define LCD_D2_GPIO         (15)    // B2
#define LCD_D3_GPIO         (7)     // B3
#define LCD_D4_GPIO         (6)     // B4
#define LCD_D5_GPIO         (21)    // G0
#define LCD_D6_GPIO         (0)     // G1
#define LCD_D7_GPIO         (46)    // G2
#define LCD_D8_GPIO         (3)     // G3
#define LCD_D9_GPIO         (8)     // G4
#define LCD_D10_GPIO        (18)    // G5
#define LCD_D11_GPIO        (10)    // R0
#define LCD_D12_GPIO        (11)    // R1
#define LCD_D13_GPIO        (12)    // R2
#define LCD_D14_GPIO        (13)    // R3
#define LCD_D15_GPIO        (14)    // R4
#define LCD_DISP_EN_GPIO  (-1)

#define LCD_PIN_RST            -1
#define LCD_PIN_BK_LIGHT       45

#define TOUCH_IIC_SCL   47
#define TOUCH_IIC_SDA   48

#define LCD_BK_LIGHT_ON_LEVEL  1
#define LCD_BK_LIGHT_OFF_LEVEL !LCD_BK_LIGHT_ON_LEVEL

