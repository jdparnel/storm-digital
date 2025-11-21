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

// Touch Screen Registers (FT813 Datasheet pp. 32-35)
#define FT813_REG_TOUCH_MODE        0x302104    // Touch screen sampling mode
#define FT813_REG_TOUCH_ADC_MODE    0x302108    // Touch screen ADC mode
#define FT813_REG_TOUCH_CHARGE      0x30210C    // Touch charge time
#define FT813_REG_TOUCH_SETTLE      0x302110    // Touch settle time
#define FT813_REG_TOUCH_OVERSAMPLE  0x302114    // Touch oversample
#define FT813_REG_TOUCH_RZTHRESH    0x302118    // Touch resistance threshold
#define FT813_REG_TOUCH_RAW_XY      0x30211C    // Touch raw X/Y
#define FT813_REG_TOUCH_RZ          0x302120    // Touch resistance
#define FT813_REG_TOUCH_SCREEN_XY   0x302124    // Touch screen X/Y (calibrated)
#define FT813_REG_TOUCH_TAG_XY      0x302128    // Touch tag X/Y
#define FT813_REG_TOUCH_TAG         0x30212C    // Touch tag
#define FT813_REG_TOUCH_TAG1_XY     0x302130    // Multi-touch tag1 XY
#define FT813_REG_TOUCH_TAG1        0x302134    // Multi-touch tag1
#define FT813_REG_TOUCH_TAG2_XY     0x302138    // Multi-touch tag2 XY
#define FT813_REG_TOUCH_TAG2        0x30213C    // Multi-touch tag2
#define FT813_REG_TOUCH_TAG3_XY     0x302140    // Multi-touch tag3 XY
#define FT813_REG_TOUCH_TAG3        0x302144    // Multi-touch tag3
#define FT813_REG_TOUCH_TAG4_XY     0x302148    // Multi-touch tag4 XY
#define FT813_REG_TOUCH_TAG4        0x30214C    // Multi-touch tag4
#define FT813_REG_TOUCH_TRANSFORM_A 0x302150    // Touch transform coefficient A
#define FT813_REG_TOUCH_TRANSFORM_B 0x302154    // Touch transform coefficient B
#define FT813_REG_TOUCH_TRANSFORM_C 0x302158    // Touch transform coefficient C
#define FT813_REG_TOUCH_TRANSFORM_D 0x30215C    // Touch transform coefficient D
#define FT813_REG_TOUCH_TRANSFORM_E 0x302160    // Touch transform coefficient E
#define FT813_REG_TOUCH_TRANSFORM_F 0x302164    // Touch transform coefficient F
#define FT813_REG_CTOUCH_EXTENDED   0x302108    // Extended CTouch mode (same as TOUCH_ADC_MODE)
#define FT813_REG_TAG               0x30207C    // Current tag value

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

// Touch Modes
#define FT813_TOUCH_MODE_OFF    0           // Touch detection off
#define FT813_TOUCH_MODE_ONESHOT 1          // One shot mode
#define FT813_TOUCH_MODE_FRAME  2           // Frame mode (continuous)
#define FT813_TOUCH_MODE_CONTINUOUS 3       // Continuous mode

// Touch Input Structure (similar to GD2 library)
typedef struct {
    int16_t x;              // Touch X coordinate (-32768 = not touching)
    int16_t y;              // Touch Y coordinate
    uint8_t tag;            // Tag value of touched object
    uint8_t touching;       // 1 if screen is touched, 0 if not
} ft813_touch_t;

// Public API - Hardware Initialization
esp_err_t ft813_init(void);

// Touch Input API
void ft813_get_touch_inputs(ft813_touch_t *inputs);

// Low-level Command Building API (for screens module)
void ft813_cmd32(uint32_t val);
void ft813_cmd_text(int16_t x, int16_t y, int16_t font, uint16_t options, const char *s);
void ft813_cmd_button(int16_t x, int16_t y, int16_t w, int16_t h, int16_t font, uint16_t options, const char *s);
void ft813_cmd_tag(uint8_t tag_value);
void ft813_swap(void);

#endif // FT813_H
