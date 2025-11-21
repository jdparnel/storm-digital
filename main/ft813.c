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
static void cmd32(uint32_t val)
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
        ESP_LOGW(TAG, "flush() called but stream not active!");
        return;
    }
    
    ESP_LOGI(TAG, "Flushing %u bytes from command buffer", stream_pos);
    
    // Pad to 4-byte boundary
    while (stream_pos & 3) {
        stream_buf[stream_pos++] = 0;
    }
    
    // Write all buffered commands
    for (size_t i = 0; i < stream_pos; i += 4) {
        wr32(FT813_REG_CMDB_WRITE, *(uint32_t*)&stream_buf[i]);
    }
    
    ESP_LOGI(TAG, "Flushed %u bytes to REG_CMDB_WRITE", stream_pos);
    stream_pos = 0;
    stream_active = false;
}

// Wait for coprocessor to finish
static void finish(void)
{
    // Flush any pending commands first
    flush();
    
    // Wait for coprocessor to catch up
    uint16_t rp, wp;
    int timeout = 1000;
    do {
        rp = rd16(FT813_REG_CMD_READ) & 0xFFF;
        wp = rd16(FT813_REG_CMD_WRITE) & 0xFFF;
        if (rp == wp) {
            ESP_LOGI(TAG, "Commands complete: RP=WP=%u", rp);
            return;
        }
        vTaskDelay(pdMS_TO_TICKS(1));
    } while (--timeout > 0);
    ESP_LOGE(TAG, "Timeout! RP=%u WP=%u", rp, wp);
}

// GD2-style command functions
static void cmd_dlstart(void)
{
    cmd32(0xFFFFFF00);  // CMD_DLSTART
}

static void cmd_swap(void)
{
    cmd32(0xFFFFFF01);  // CMD_SWAP
}

static void cmd_text(int16_t x, int16_t y, int16_t font, uint16_t options, const char *s)
{
    cmd32(0xFFFFFF0C);  // CMD_TEXT
    cmd32((((uint32_t)y) << 16) | (x & 0xFFFF));
    cmd32((((uint32_t)options) << 16) | (font & 0xFFFF));
    cmd_str(s);
}

static void cmd_button(int16_t x, int16_t y, int16_t w, int16_t h, int16_t font, uint16_t options, const char *s)
{
    cmd32(0xFFFFFF0D);  // CMD_BUTTON
    cmd32((((uint32_t)y) << 16) | (x & 0xFFFF));
    cmd32((((uint32_t)h) << 16) | (w & 0xFFFF));
    cmd32((((uint32_t)options) << 16) | (font & 0xFFFF));
    cmd_str(s);
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
        .clock_speed_hz = 10*1000*1000,  // 10 MHz
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
    
    // Configure display timing for 800x480
    wr16(FT813_REG_HCYCLE, 928);
    wr16(FT813_REG_HOFFSET, 88);
    wr16(FT813_REG_HSYNC0, 0);
    wr16(FT813_REG_HSYNC1, 48);
    wr16(FT813_REG_VCYCLE, 525);
    wr16(FT813_REG_VOFFSET, 32);
    wr16(FT813_REG_VSYNC0, 0);
    wr16(FT813_REG_VSYNC1, 3);
    wr8(FT813_REG_SWIZZLE, 0);
    wr8(FT813_REG_PCLK_POL, 1);
    wr16(FT813_REG_HSIZE, 800);
    wr16(FT813_REG_VSIZE, 480);
    
    // Clear display list
    wr32(FT813_RAM_DL + 0, 0x02000000);  // CLEAR_COLOR_RGB(0,0,0)
    wr32(FT813_RAM_DL + 4, 0x26000007);  // CLEAR(1,1,1)
    wr32(FT813_RAM_DL + 8, 0x00000000);  // DISPLAY
    wr8(FT813_REG_DLSWAP, 2);  // DLSWAP_FRAME
    
    // Enable display
    wr8(FT813_REG_GPIO_DIR, 0x80);
    wr8(FT813_REG_GPIO, 0x80);
    wr8(FT813_REG_PCLK, 2);
    wr8(FT813_REG_PWM_DUTY, 128);  // 50% backlight
    
    ESP_LOGI(TAG, "FT813 initialized - black screen should be visible");
    return ESP_OK;
}

// GD2-style hello world test
esp_err_t ft813_draw_hello_world(void)
{
    ESP_LOGI(TAG, "=== GD2-Style Hello World ===");
    
    // Read initial command buffer state
    uint16_t rp = rd16(FT813_REG_CMD_READ) & 0xFFF;
    uint16_t wp = rd16(FT813_REG_CMD_WRITE) & 0xFFF;
    ESP_LOGI(TAG, "Initial: RP=%u WP=%u", rp, wp);
    
    // Build display list using coprocessor
    cmd_dlstart();
    cmd32(0x02FFFFFF);  // CLEAR_COLOR_RGB(255,255,255) - white
    cmd32(0x26000007);  // CLEAR(1,1,1)
    cmd32(0x04000000);  // COLOR_RGB(0,0,0) - black text
    cmd_text(400, 150, 31, FT813_OPT_CENTER, "Hello World!");
    cmd_text(400, 250, 27, FT813_OPT_CENTER, "FT813 + GD2 Style");
    cmd_button(300, 320, 200, 60, 28, 0, "Click Me");
    cmd32(0x00000000);  // DISPLAY
    cmd_swap();
    
    // Execute commands
    ESP_LOGI(TAG, "Commands queued, executing...");
    finish();
    
    ESP_LOGI(TAG, "Done! Check display for text and button");
    return ESP_OK;
}
