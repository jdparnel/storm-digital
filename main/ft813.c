/*
 * FT813 GD2-Style Driver - WORKING IMPLEMENTATION
 * Based on jamesbowman/gd2-lib patterns for FT81x
 * 
 * Successfully displays text, buttons, and graphics using the FT813's coprocessor.
 * This uses the REG_CMDB_WRITE approach for FT81x chips (auto-incrementing buffer).
 */

#include "ft813.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_log.h"
#include <string.h>

static const char *TAG = "FT813_GD2";
static spi_device_handle_t spi = NULL;

// SPI transaction
static void spi_xfer(const uint8_t *tx, uint8_t *rx, size_t len)
{
    spi_transaction_t t = {
        .length = len * 8,
        .tx_buffer = tx,
        .rx_buffer = rx
    };
    spi_device_transmit(spi, &t);
}

// Host command (single byte)
static void hostcmd(uint8_t cmd)
{
    uint8_t tx[3] = {cmd, 0, 0};
    spi_xfer(tx, NULL, 3);
}

// Read register 16-bit
static uint16_t rd16(uint32_t addr)
{
    uint8_t tx[6] = {
        (addr >> 16) & 0x3F,
        (addr >> 8) & 0xFF,
        addr & 0xFF,
        0, 0, 0  // dummy + 2 data bytes
    };
    uint8_t rx[6];
    spi_xfer(tx, rx, 6);
    return (rx[5] << 8) | rx[4];
}

// Read register 32-bit
static uint32_t rd32(uint32_t addr)
{
    uint8_t tx[8] = {
        (addr >> 16) & 0x3F,
        (addr >> 8) & 0xFF,
        addr & 0xFF,
        0, 0, 0, 0, 0  // dummy + 4 data bytes
    };
    uint8_t rx[8];
    spi_xfer(tx, rx, 8);
    return (rx[7] << 24) | (rx[6] << 16) | (rx[5] << 8) | rx[4];
}

// Write register 8-bit
static void wr8(uint32_t addr, uint8_t val)
{
    uint8_t tx[4] = {
        0x80 | ((addr >> 16) & 0x3F),
        (addr >> 8) & 0xFF,
        addr & 0xFF,
        val
    };
    spi_xfer(tx, NULL, 4);
}

// Write register 16-bit
static void wr16(uint32_t addr, uint16_t val)
{
    uint8_t tx[5] = {
        0x80 | ((addr >> 16) & 0x3F),
        (addr >> 8) & 0xFF,
        addr & 0xFF,
        val & 0xFF,
        (val >> 8) & 0xFF
    };
    spi_xfer(tx, NULL, 5);
}

// Write register 32-bit
static void wr32(uint32_t addr, uint32_t val)
{
    uint8_t tx[7] = {
        0x80 | ((addr >> 16) & 0x3F),
        (addr >> 8) & 0xFF,
        addr & 0xFF,
        val & 0xFF,
        (val >> 8) & 0xFF,
        (val >> 16) & 0xFF,
        (val >> 24) & 0xFF
    };
    spi_xfer(tx, NULL, 7);
}

// GD2 style command buffer - For FT81x, stream to REG_CMDB_WRITE
// Hardware auto-increments, we just keep writing bytes

static uint8_t stream_buf[4096];  // Command buffer
static size_t stream_pos = 0;  // Position in buffer
static bool stream_active = false;

// Start streaming to REG_CMDB_WRITE
static void stream_begin(void)
{
    if (stream_active) return;
    
    stream_pos = 0;
    stream_active = true;
}

// Write a byte to command buffer
static void cmdbyte(uint8_t b)
{
    if (!stream_active) stream_begin();
    stream_buf[stream_pos++] = b;
    
    if (stream_pos >= sizeof(stream_buf)) {
        // Buffer full, flush it
        wr32(FT813_REG_CMDB_WRITE, *(uint32_t*)&stream_buf[0]);
        for (size_t i = 4; i < stream_pos; i += 4) {
            wr32(FT813_REG_CMDB_WRITE, *(uint32_t*)&stream_buf[i]);
        }
        stream_pos = 0;
    }
}

// Write 32-bit command
void ft813_cmd32(uint32_t val)
{
    cmdbyte(val & 0xFF);
    cmdbyte((val >> 8) & 0xFF);
    cmdbyte((val >> 16) & 0xFF);
    cmdbyte((val >> 24) & 0xFF);
}

// Write string to command buffer (null terminated, padded to 4-byte boundary)
static void cmd_str(const char *s)
{
    size_t len = strlen(s) + 1;  // include null
    for (size_t i = 0; i < len; i++) {
        cmdbyte(s[i]);
    }
    // Pad to 4-byte boundary
    while ((len & 3) != 0) {
        cmdbyte(0);
        len++;
    }
}

// Flush any pending commands
static void flush(void)
{
    if (!stream_active) {
        return;
    }
    
    // Pad to 4-byte boundary
    while (stream_pos & 3) {
        stream_buf[stream_pos++] = 0;
    }
    
    // Write all buffered commands
    for (size_t i = 0; i < stream_pos; i += 4) {
        wr32(FT813_REG_CMDB_WRITE, *(uint32_t*)&stream_buf[i]);
    }
    
    // Reset buffer
    stream_pos = 0;
    stream_active = false;
}

// Wait for coprocessor to finish
static void finish(void)
{
    // Flush any pending commands first
    flush();
    
    // Wait for coprocessor to catch up (without logging to avoid flicker)
    uint16_t rp, wp;
    int timeout = 100;  // Reduced timeout since we're checking every 1ms
    do {
        rp = rd16(FT813_REG_CMD_READ) & 0xFFF;
        wp = rd16(FT813_REG_CMD_WRITE) & 0xFFF;
        if (rp == wp) {
            return;
        }
        vTaskDelay(pdMS_TO_TICKS(1));
    } while (--timeout > 0);
    // Timeout occurred but don't log to avoid screen flicker
}

// GD2-style command functions matching library implementation
static void cmd_dlstart(void)
{
    ft813_cmd32(0xFFFFFF00);  // CMD_DLSTART
}

static void cmd_swap(void)
{
    ft813_cmd32(0xFFFFFF01);  // CMD_SWAP
}

static void cmd_loadidentity(void)
{
    ft813_cmd32(0xFFFFFF26);  // CMD_LOADIDENTITY
}

// GD2 swap pattern: Display() → cmd_swap() → cmd_loadidentity() → cmd_dlstart() → flush()
void ft813_swap(void)
{
    ft813_cmd32(0x00000000);  // DISPLAY
    cmd_swap();         // CMD_SWAP
    cmd_loadidentity(); // CMD_LOADIDENTITY
    cmd_dlstart();      // CMD_DLSTART for next frame
    flush();            // Write to hardware
    finish();           // Ensure coprocessor processed previous list
}

void ft813_cmd_text(int16_t x, int16_t y, int16_t font, uint16_t options, const char *s)
{
    ft813_cmd32(0xFFFFFF0C);  // CMD_TEXT
    ft813_cmd32((((uint32_t)y) << 16) | (x & 0xFFFF));
    ft813_cmd32((((uint32_t)options) << 16) | (font & 0xFFFF));
    cmd_str(s);
}

void ft813_cmd_button(int16_t x, int16_t y, int16_t w, int16_t h, int16_t font, uint16_t options, const char *s)
{
    ft813_cmd32(0xFFFFFF0D);  // CMD_BUTTON
    ft813_cmd32((((uint32_t)y) << 16) | (x & 0xFFFF));
    ft813_cmd32((((uint32_t)h) << 16) | (w & 0xFFFF));
    ft813_cmd32((((uint32_t)options) << 16) | (font & 0xFFFF));
    cmd_str(s);
}

// GD2-style tag assignment - call before drawing tagged object
void ft813_cmd_tag(uint8_t tag_value)
{
    // Use display list command for tag assignment
    ft813_cmd32(0x03000000 | tag_value);  // TAG(n) display list command
}

// Initialize FT813
esp_err_t ft813_init(void)
{
    ESP_LOGI(TAG, "=== GD2-Style FT813 Initialization ===");
    
    // GPIO setup
    gpio_set_direction(FT813_PIN_PD, GPIO_MODE_OUTPUT);
    gpio_set_level(FT813_PIN_PD, 0);
    vTaskDelay(pdMS_TO_TICKS(20));
    gpio_set_level(FT813_PIN_PD, 1);
    vTaskDelay(pdMS_TO_TICKS(100));
    
    // SPI setup
    spi_bus_config_t buscfg = {
        .mosi_io_num = FT813_PIN_MOSI,
        .miso_io_num = FT813_PIN_MISO,
        .sclk_io_num = FT813_PIN_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 4096
    };
    ESP_ERROR_CHECK(spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO));
    
    spi_device_interface_config_t devcfg = {
        .clock_speed_hz = 20*1000*1000,  // 20 MHz (FT813 supports up to 30 MHz)
        .mode = 0,
        .spics_io_num = FT813_PIN_CS,
        .queue_size = 7
    };
    ESP_ERROR_CHECK(spi_bus_add_device(SPI2_HOST, &devcfg, &spi));
    
    // FT813 initialization sequence
    hostcmd(0x68);  // CLKEXT
    hostcmd(0x44);  // CLKSEL
    hostcmd(0x00);  // ACTIVE
    vTaskDelay(pdMS_TO_TICKS(300));
    
    // Read chip ID
    uint8_t id = rd32(FT813_REG_ID) & 0xFF;
    ESP_LOGI(TAG, "Chip ID: 0x%02X (expect 0x7C)", id);
    
    // Configure display timing for 800x480 (Newhaven FT813 7" reference + Bowman original)
    // Datasheet typical values: HCYCLE=928, HSIZE=800, HOFFSET=88, HSYNC0=0, HSYNC1=48
    // VCYCLE=525, VSIZE=480, VOFFSET=32, VSYNC0=0, VSYNC1=3, PCLK_POL=1, SWIZZLE=0, CSPREAD=1, DITHER=1
    wr16(FT813_REG_HCYCLE, 928);
    wr16(FT813_REG_HOFFSET, 88);
    wr16(FT813_REG_HSYNC0, 0);
    wr16(FT813_REG_HSYNC1, 48);
    wr16(FT813_REG_VCYCLE, 525);
    wr16(FT813_REG_VOFFSET, 32);
    wr16(FT813_REG_VSYNC0, 0);
    wr16(FT813_REG_VSYNC1, 3);
    wr16(FT813_REG_HSIZE, 800);
    wr16(FT813_REG_VSIZE, 480);
    wr8(FT813_REG_SWIZZLE, 0);
    wr8(FT813_REG_CSPREAD, 1);  // Clock spread enable (reduces EMI, recommended)
    wr8(FT813_REG_PCLK_POL, 1);
    wr8(FT813_REG_DITHER, 1);   // Enable dithering for smoother gradients
    
    // Clear display list
    wr32(FT813_RAM_DL + 0, 0x02000000);  // CLEAR_COLOR_RGB(0,0,0)
    wr32(FT813_RAM_DL + 4, 0x26000007);  // CLEAR(1,1,1)
    wr32(FT813_RAM_DL + 8, 0x00000000);  // DISPLAY
    wr8(FT813_REG_DLSWAP, 2);  // DLSWAP_FRAME
    
    // Enable display
    wr8(FT813_REG_GPIO_DIR, 0x80);
    wr8(FT813_REG_GPIO, 0x80);
    
    // Original Bowman/Newhaven typical pixel clock divider: PCLK=2 (~60 Hz frame)
    // Try PCLK=2 first to remove periodic full redraw artifact. Can iterate later.
    wr8(FT813_REG_PCLK, 2);

    // Read back and log timing registers for verification
    uint16_t hc = rd32(FT813_REG_HCYCLE) & 0xFFFF;
    uint16_t ho = rd32(FT813_REG_HOFFSET) & 0xFFFF;
    uint16_t h0 = rd32(FT813_REG_HSYNC0) & 0xFFFF;
    uint16_t h1 = rd32(FT813_REG_HSYNC1) & 0xFFFF;
    uint16_t vc = rd32(FT813_REG_VCYCLE) & 0xFFFF;
    uint16_t vo = rd32(FT813_REG_VOFFSET) & 0xFFFF;
    uint16_t v0 = rd32(FT813_REG_VSYNC0) & 0xFFFF;
    uint16_t v1 = rd32(FT813_REG_VSYNC1) & 0xFFFF;
    uint8_t sw = rd32(FT813_REG_SWIZZLE) & 0xFF;
    uint8_t cp = rd32(FT813_REG_CSPREAD) & 0xFF;
    uint8_t pp = rd32(FT813_REG_PCLK_POL) & 0xFF;
    uint8_t di = rd32(FT813_REG_DITHER) & 0xFF;
    uint8_t pk = rd32(FT813_REG_PCLK) & 0xFF;
    ESP_LOGI(TAG, "Timing set: HCYCLE=%u HOFFSET=%u HSYNC0=%u HSYNC1=%u VCYCLE=%u VOFFSET=%u VSYNC0=%u VSYNC1=%u", hc, ho, h0, h1, vc, vo, v0, v1);
    ESP_LOGI(TAG, "Size: HSIZE=%u VSIZE=%u", rd32(FT813_REG_HSIZE) & 0xFFFF, rd32(FT813_REG_VSIZE) & 0xFFFF);
    ESP_LOGI(TAG, "Misc: SWIZZLE=%u CSPREAD=%u PCLK_POL=%u DITHER=%u PCLK=%u", sw, cp, pp, di, pk);
    
    // =============================================================================
    // CONFIGURE CAPACITIVE TOUCH (FT813 Datasheet pp. 32-35)
    // =============================================================================
    // Configure touch BEFORE backlight to avoid electrical noise interference
    // For capacitive touch displays (CTP), use extended mode
    // Mode 3 = CTOUCH_MODE_EXTENDED for 5-point capacitive touch
    wr8(FT813_REG_CTOUCH_EXTENDED, 0x00);  // Set to compatibility mode first
    vTaskDelay(pdMS_TO_TICKS(10));
    
    // Configure touch parameters for capacitive touch
    wr16(FT813_REG_TOUCH_RZTHRESH, 1200);  // Touch resistance threshold
    wr8(FT813_REG_TOUCH_MODE, FT813_TOUCH_MODE_CONTINUOUS);  // Continuous touch sampling
    wr8(FT813_REG_TOUCH_OVERSAMPLE, 15);   // Touch oversample (0-15, max for stability with PWM noise)
    wr8(FT813_REG_TOUCH_SETTLE, 5);        // Touch settle time (increased for PWM noise immunity)
    
    vTaskDelay(pdMS_TO_TICKS(50));  // Let touch controller stabilize
    
    // Configure backlight PWM before setting duty cycle
    // REG_PWM_HZ: Backlight PWM frequency (default 250 Hz, range 0-128)
    // Higher frequency = less flicker, but may cause EMI
    wr16(FT813_REG_PWM_HZ, 250);  // 250 Hz PWM frequency (standard for backlight)
    
    // Set backlight duty cycle after PWM frequency is configured
    // 50% duty cycle to manage power/thermal load (760mA @ 100% → ~380mA @ 50%)
    wr8(FT813_REG_PWM_DUTY, 128);  // 50% backlight (128/255 ≈ 0.50)
    ESP_LOGI(TAG, "Backlight PWM configured: 250 Hz, 50%% duty cycle (~380mA vs 760mA at 100%%)");
    
    vTaskDelay(pdMS_TO_TICKS(50));  // Allow power supply to stabilize after backlight change
    
    // Note: Calibration for capacitive touch is typically not needed
    // as CTP controllers have built-in calibration. If calibration is needed,
    // use CMD_CALIBRATE command.
    
    ESP_LOGI(TAG, "Capacitive touch configured in continuous mode");
    
    ESP_LOGI(TAG, "FT813 initialized - black screen should be visible");
    return ESP_OK;
}

// =============================================================================
// CAPACITIVE TOUCH INPUT FUNCTIONS (FT813 Datasheet pp. 32-35)
// =============================================================================

// Read touch inputs from FT813 (following GD2 library pattern)
void ft813_get_touch_inputs(ft813_touch_t *inputs)
{
    if (!inputs) return;
    
    // Read touch screen coordinates (32-bit: Y in upper 16 bits, X in lower 16 bits)
    // FT813 returns -32768 (0x8000) when not touching
    uint32_t xy = rd32(FT813_REG_TOUCH_SCREEN_XY);
    inputs->x = (int16_t)(xy & 0xFFFF);
    inputs->y = (int16_t)((xy >> 16) & 0xFFFF);
    
    // Read tag value (which tagged object is being touched)
    inputs->tag = rd32(FT813_REG_TOUCH_TAG) & 0xFF;
    
    // Determine if touching (x != -32768 means touch detected)
    inputs->touching = (inputs->x != -32768) ? 1 : 0;
}
