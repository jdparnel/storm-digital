#ifndef FT813_DISPLAY_H
#define FT813_DISPLAY_H

#include <stdint.h>
#include <esp_err.h>
#include "driver/gpio.h"

// ============================================================================
// FT813 GPIO PIN DEFINITIONS
// ============================================================================
#define FT813_SPI_HOST          SPI2_HOST
#define FT813_PIN_MOSI          GPIO_NUM_11
#define FT813_PIN_MISO          GPIO_NUM_13
#define FT813_PIN_CLK           GPIO_NUM_12
#define FT813_PIN_CS            GPIO_NUM_10
#define FT813_PIN_PD            GPIO_NUM_5

// ============================================================================
// FT813 REGISTER ADDRESSES
// ============================================================================
#define FT813_REG_ID            0x302000
#define FT813_REG_HSIZE         0x302034
#define FT813_REG_HCYCLE        0x30202C
#define FT813_REG_HOFFSET       0x302030
#define FT813_REG_HSYNC0        0x302038
#define FT813_REG_HSYNC1        0x30203C
#define FT813_REG_VSIZE         0x302048
#define FT813_REG_VCYCLE        0x302040
#define FT813_REG_VOFFSET       0x302044
#define FT813_REG_VSYNC0        0x30204C
#define FT813_REG_VSYNC1        0x302050
#define FT813_REG_SWIZZLE       0x302064
#define FT813_REG_PCLK_POL      0x30206C
#define FT813_REG_PCLK          0x302070
#define FT813_REG_CSPREAD       0x302068
#define FT813_REG_DITHER        0x302060
#define FT813_REG_DLSWAP        0x302054
#define FT813_REG_GPIO_DIR      0x302090
#define FT813_REG_GPIO          0x302094
#define FT813_REG_PWM_HZ        0x3020D0
#define FT813_REG_PWM_DUTY      0x3020D4
#define FT813_REG_FREQUENCY     0x30200C

// Memory Map
#define FT813_RAM_DL            0x300000    // Display List RAM

// Function declarations - simple test only
esp_err_t ft813_simple_test(void);

#endif // FT813_DISPLAY_H
