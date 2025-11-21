/*
 * FT813 Display Driver with UI Screens
 */

#include "ft813.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_log.h"
#include <string.h>

static const char *TAG = "FT813";

static spi_device_handle_t g_spi = NULL;
static screen_t g_current_screen = SCREEN_HOME;

// Host commands (must be sent before any register access)
#define FT_GPU_ACTIVE_M         0x00
#define FT_GPU_EXTERNAL_OSC     0x44

// ============================================================================
// LOW-LEVEL SPI OPERATIONS
// ============================================================================

static esp_err_t spi_write_read(const uint8_t *tx, uint8_t *rx, size_t len)
{
    spi_transaction_t trans = {
        .length = len * 8,
        .tx_buffer = tx,
        .rx_buffer = rx
    };
    return spi_device_transmit(g_spi, &trans);
}

static void ft813_host_cmd(uint8_t cmd)
{
    uint8_t buf[3] = {cmd, 0x00, 0x00};
    spi_write_read(buf, NULL, 3);
}

static uint8_t ft813_rd8(uint32_t addr)
{
    uint8_t tx[5] = {(addr >> 16) & 0x3F, (addr >> 8) & 0xFF, addr & 0xFF, 0, 0};
    uint8_t rx[5];
    spi_write_read(tx, rx, 5);
    return rx[4];
}

static void ft813_wr8(uint32_t addr, uint8_t val)
{
    uint8_t tx[4] = {0x80 | ((addr >> 16) & 0x3F), (addr >> 8) & 0xFF, addr & 0xFF, val};
    spi_write_read(tx, NULL, 4);
}

static void ft813_wr16(uint32_t addr, uint16_t val)
{
    uint8_t tx[5] = {0x80 | ((addr >> 16) & 0x3F), (addr >> 8) & 0xFF, addr & 0xFF, val & 0xFF, (val >> 8) & 0xFF};
    spi_write_read(tx, NULL, 5);
}

static void ft813_wr32(uint32_t addr, uint32_t val)
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
    spi_write_read(tx, NULL, 7);
}

// ============================================================================
// COMMAND COPROCESSOR OPERATIONS
// ============================================================================

static uint16_t g_cmd_offset = 0;

static void cmd_start(void)
{
    // Sync with coprocessor - read current write pointer
    g_cmd_offset = ft813_rd8(FT813_REG_CMD_WRITE) | (ft813_rd8(FT813_REG_CMD_WRITE + 1) << 8);
}

static void cmd_write(uint32_t data)
{
    ft813_wr32(FT813_RAM_CMD + g_cmd_offset, data);
    g_cmd_offset = (g_cmd_offset + 4) & 0xFFF;
}

static void cmd_execute(void)
{
    // Update write pointer to trigger execution
    ft813_wr16(FT813_REG_CMD_WRITE, g_cmd_offset);
    
    // Wait for coprocessor to catch up
    vTaskDelay(pdMS_TO_TICKS(20));
}

static void cmd_text(int16_t x, int16_t y, int16_t font, uint16_t options, const char *str)
{
    cmd_write(CMD_TEXT);
    cmd_write(((uint32_t)y << 16) | (uint32_t)x);
    cmd_write(((uint32_t)options << 16) | (uint32_t)font);
    
    // Write string padded to 4-byte boundary
    size_t len = strlen(str) + 1;
    for (size_t i = 0; i < len; i += 4) {
        uint32_t chunk = 0;
        for (int j = 0; j < 4 && (i + j) < len; j++) {
            chunk |= ((uint32_t)str[i + j]) << (j * 8);
        }
        cmd_write(chunk);
    }
}

static void cmd_button(int16_t x, int16_t y, int16_t w, int16_t h, int16_t font, uint16_t options, const char *str)
{
    cmd_write(CMD_BUTTON);
    cmd_write(((uint32_t)y << 16) | (uint32_t)x);
    cmd_write(((uint32_t)h << 16) | (uint32_t)w);
    cmd_write(((uint32_t)options << 16) | (uint32_t)font);
    
    size_t len = strlen(str) + 1;
    for (size_t i = 0; i < len; i += 4) {
        uint32_t chunk = 0;
        for (int j = 0; j < 4 && (i + j) < len; j++) {
            chunk |= ((uint32_t)str[i + j]) << (j * 8);
        }
        cmd_write(chunk);
    }
}

static void cmd_bgcolor(uint32_t color)
{
    cmd_write(CMD_BGCOLOR);
    cmd_write(color);
}

static void cmd_fgcolor(uint32_t color)
{
    cmd_write(CMD_FGCOLOR);
    cmd_write(color);
}

// ============================================================================
// INITIALIZATION
// ============================================================================

esp_err_t ft813_init(void)
{
    esp_err_t ret;
    
    // Configure PD pin
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << FT813_PIN_PD),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&io_conf);
    
    // Power cycle
    gpio_set_level(FT813_PIN_PD, 0);
    vTaskDelay(pdMS_TO_TICKS(50));
    gpio_set_level(FT813_PIN_PD, 1);
    vTaskDelay(pdMS_TO_TICKS(50));
    
    // Configure SPI
    spi_bus_config_t bus_cfg = {
        .mosi_io_num = FT813_PIN_MOSI,
        .miso_io_num = FT813_PIN_MISO,
        .sclk_io_num = FT813_PIN_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 4096
    };
    
    ret = spi_bus_initialize(FT813_SPI_HOST, &bus_cfg, SPI_DMA_CH_AUTO);
    if (ret != ESP_OK) return ret;
    
    spi_device_interface_config_t dev_cfg = {
        .clock_speed_hz = 4000000,
        .mode = 0,
        .spics_io_num = FT813_PIN_CS,
        .queue_size = 7
    };
    
    ret = spi_bus_add_device(FT813_SPI_HOST, &dev_cfg, &g_spi);
    if (ret != ESP_OK) return ret;
    
    // Send host commands
    ft813_host_cmd(FT_GPU_ACTIVE_M);
    vTaskDelay(pdMS_TO_TICKS(20));
    ft813_host_cmd(FT_GPU_EXTERNAL_OSC);
    vTaskDelay(pdMS_TO_TICKS(10));
    
    // Wait for chip ID
    for (int i = 0; i < 100; i++) {
        uint8_t chip_id = ft813_rd8(FT813_REG_ID);
        if (chip_id == 0x7C) {
            ESP_LOGI(TAG, "FT813 ready (ID: 0x%02X)", chip_id);
            break;
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
    
    // Configure display timing (800x480)
    ft813_wr16(FT813_REG_HSIZE, 800);
    ft813_wr16(FT813_REG_HCYCLE, 928);
    ft813_wr16(FT813_REG_HOFFSET, 88);
    ft813_wr16(FT813_REG_HSYNC0, 0);
    ft813_wr16(FT813_REG_HSYNC1, 48);
    ft813_wr16(FT813_REG_VSIZE, 480);
    ft813_wr16(FT813_REG_VCYCLE, 525);
    ft813_wr16(FT813_REG_VOFFSET, 32);
    ft813_wr16(FT813_REG_VSYNC0, 0);
    ft813_wr16(FT813_REG_VSYNC1, 3);
    ft813_wr8(FT813_REG_SWIZZLE, 0);
    ft813_wr8(FT813_REG_PCLK_POL, 1);
    ft813_wr8(FT813_REG_CSPREAD, 0);
    ft813_wr8(FT813_REG_DITHER, 1);
    
    // Enable display clock first
    ft813_wr8(FT813_REG_PCLK, 2);
    vTaskDelay(pdMS_TO_TICKS(50));
    
    // Try PWM duty cycle at 0 (which might mean full brightness on some displays)
    ft813_wr16(FT813_REG_PWM_HZ, 250);
    ft813_wr8(FT813_REG_PWM_DUTY, 0);  // Try 0 first
    vTaskDelay(pdMS_TO_TICKS(100));
    
    // If that doesn't work, try 128 (50%)
    ft813_wr8(FT813_REG_PWM_DUTY, 128);
    vTaskDelay(pdMS_TO_TICKS(100));
    
    // Now set all GPIO pins high as well
    ft813_wr8(FT813_REG_GPIO_DIR, 0xFF);
    ft813_wr8(FT813_REG_GPIO, 0xFF);
    
    // Read back to verify
    uint8_t gpio_dir = ft813_rd8(FT813_REG_GPIO_DIR);
    uint8_t gpio = ft813_rd8(FT813_REG_GPIO);
    uint8_t pwm_duty = ft813_rd8(FT813_REG_PWM_DUTY);
    ESP_LOGI(TAG, "GPIO_DIR: 0x%02X, GPIO: 0x%02X, PWM_DUTY: %d", gpio_dir, gpio, pwm_duty);
    
    vTaskDelay(pdMS_TO_TICKS(100));
    
    ESP_LOGI(TAG, "Display initialized");
    
    return ESP_OK;
}

// ============================================================================
// UI SCREENS
// ============================================================================

void ft813_draw_screen(screen_t screen)
{
    g_current_screen = screen;
    
    cmd_start();
    cmd_write(CMD_DLSTART);
    cmd_write(CLEAR_COLOR_RGB(30, 30, 40));
    cmd_write(CLEAR(1, 1, 1));
    
    switch (screen) {
        case SCREEN_HOME:
            cmd_bgcolor(0x003870);
            cmd_fgcolor(0x0088CC);
            
            cmd_write(COLOR_RGB(255, 255, 255));
            cmd_text(400, 30, 31, 0x0600, "Storm Digital");
            
            cmd_write(TAG(1));
            cmd_button(250, 100, 300, 80, 31, 0, "Settings");
            
            cmd_write(TAG(2));
            cmd_button(250, 220, 300, 80, 31, 0, "Information");
            
            cmd_write(TAG(3));
            cmd_button(250, 340, 300, 80, 31, 0, "Options");
            break;
            
        case SCREEN_SETTINGS:
            cmd_bgcolor(0x387000);
            cmd_fgcolor(0x88CC00);
            
            cmd_write(COLOR_RGB(255, 255, 255));
            cmd_text(400, 100, 31, 0x0600, "Settings");
            cmd_text(400, 200, 28, 0x0600, "Configuration");
            
            cmd_write(TAG(10));
            cmd_button(50, 400, 200, 60, 29, 0, "< Back");
            break;
            
        case SCREEN_INFO:
            cmd_bgcolor(0x703800);
            cmd_fgcolor(0xCC8800);
            
            cmd_write(COLOR_RGB(255, 255, 255));
            cmd_text(400, 80, 31, 0x0600, "Information");
            cmd_text(400, 160, 27, 0x0600, "ESP32-S3-N16R8");
            cmd_text(400, 200, 27, 0x0600, "16MB Flash");
            cmd_text(400, 240, 27, 0x0600, "8MB PSRAM");
            
            cmd_write(TAG(10));
            cmd_button(50, 400, 200, 60, 29, 0, "< Back");
            break;
    }
    
    cmd_write(DISPLAY());
    cmd_write(CMD_SWAP);
    cmd_execute();
}

screen_t ft813_check_touch(void)
{
    uint8_t tag = ft813_rd8(FT813_REG_TOUCH_TAG);
    
    if (tag == 0) return g_current_screen;
    
    // Debounce
    vTaskDelay(pdMS_TO_TICKS(200));
    
    switch (g_current_screen) {
        case SCREEN_HOME:
            if (tag == 1) return SCREEN_SETTINGS;
            if (tag == 2) return SCREEN_INFO;
            break;
            
        case SCREEN_SETTINGS:
        case SCREEN_INFO:
            if (tag == 10) return SCREEN_HOME;
            break;
    }
    
    return g_current_screen;
}

// ============================================================================
// UI TASK
// ============================================================================

void ft813_ui_task(void *pvParameters)
{
    ft813_draw_screen(g_current_screen);
    
    while (1) {
        screen_t new_screen = ft813_check_touch();
        if (new_screen != g_current_screen) {
            ft813_draw_screen(new_screen);
        }
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}
