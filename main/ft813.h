#ifndef FT813_H
#define FT813_H

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
#define FT813_RAM_CMD           0x308000    // Command FIFO RAM

// Command Coprocessor Registers
#define FT813_REG_CMD_READ      0x3025F8    // Command read pointer (read-only)
#define FT813_REG_CMD_WRITE     0x3025FC    // Command write pointer
#define FT813_REG_CMDB_SPACE    0x302574    // Command buffer space (FT81x)
#define FT813_REG_CMDB_WRITE    0x302578    // Command buffer write (FT81x direct write)
#define FT813_CMD_FIFO_SIZE     4096        // 4KB command FIFO

// Command Codes (from FTDI Programmer Guide)
#define FT813_CMD_DLSTART       0xFFFFFF00  // Start new display list
#define FT813_CMD_SWAP          0xFFFFFF01  // Swap display list
#define FT813_CMD_TEXT          0xFFFFFF0C  // Draw text
#define FT813_CMD_BUTTON        0xFFFFFF0D  // Draw button
#define FT813_CMD_KEYS          0xFFFFFF0E  // Draw key row
#define FT813_CMD_PROGRESS      0xFFFFFF0F  // Draw progress bar
#define FT813_CMD_SLIDER        0xFFFFFF10  // Draw slider
#define FT813_CMD_SCROLLBAR     0xFFFFFF11  // Draw scrollbar
#define FT813_CMD_TOGGLE        0xFFFFFF12  // Draw toggle switch
#define FT813_CMD_GAUGE         0xFFFFFF13  // Draw gauge
#define FT813_CMD_CLOCK         0xFFFFFF14  // Draw clock
#define FT813_CMD_CALIBRATE     0xFFFFFF15  // Calibrate touchscreen
#define FT813_CMD_SPINNER       0xFFFFFF16  // Draw spinner
#define FT813_CMD_STOP          0xFFFFFF17  // Stop spinner
#define FT813_CMD_SCREENSAVER   0xFFFFFF2F  // Start screensaver
#define FT813_CMD_SKETCH        0xFFFFFF30  // Start sketch
#define FT813_CMD_SNAPSHOT      0xFFFFFF1F  // Take snapshot
#define FT813_CMD_LOGO          0xFFFFFF31  // Play FTDI logo animation

// Text/Button Options
#define FT813_OPT_CENTER        0x0600      // Center text
#define FT813_OPT_CENTERX       0x0200      // Center horizontally
#define FT813_OPT_CENTERY       0x0400      // Center vertically
#define FT813_OPT_FLAT          0x0100      // No 3D effect
#define FT813_OPT_SIGNED        0x0100      // Signed number
#define FT813_OPT_RIGHTX        0x0800      // Right-aligned

// Public API
esp_err_t ft813_init(void);
esp_err_t ft813_draw_hello_world(void);
void ft813_test_pclk_settings(uint8_t pclk, uint8_t pclk_pol);

#endif // FT813_H
