/*
 * Simple FT813 Test - Based on Newhaven Display Examples
 * Focuses on getting basic display output working
 */

#include "ft813_display.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_log.h"
#include <string.h>

static const char *TAG = "FT813_BASIC";

// Global SPI handle
static spi_device_handle_t g_spi_handle = NULL;

// GPIO Configuration
static esp_err_t ft813_configure_gpio_simple(void)
{
    gpio_config_t io_conf = {};
    
    // Configure PD pin as output
    io_conf.pin_bit_mask = (1ULL << FT813_PIN_PD);
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    
    esp_err_t ret = gpio_config(&io_conf);
    if (ret != ESP_OK) return ret;
    
    gpio_set_level(FT813_PIN_PD, 1); // Start with chip enabled
    return ESP_OK;
}

// SPI Helper Functions
static esp_err_t spi_write_read(const uint8_t *tx_data, uint8_t *rx_data, size_t len)
{
    spi_transaction_t trans = {};
    trans.length = len * 8;
    trans.tx_buffer = tx_data;
    trans.rx_buffer = rx_data;
    return spi_device_transmit(g_spi_handle, &trans);
}

// Basic register operations
static uint8_t ft813_read_8(uint32_t addr)
{
    uint8_t tx_buf[5] = {
        (addr >> 16) & 0x3F,  // Read command: bits 7-6 = 00
        (addr >> 8) & 0xFF,
        addr & 0xFF,
        0x00,  // dummy byte for SINGLE channel SPI
        0x00   // receive byte
    };
    uint8_t rx_buf[5];
    
    spi_write_read(tx_buf, rx_buf, 5);
    return rx_buf[4];  // Data in last byte after dummy
}

static uint16_t ft813_read_16(uint32_t addr)
{
    uint8_t tx_buf[6] = {
        (addr >> 16) & 0x3F,
        (addr >> 8) & 0xFF,
        addr & 0xFF,
        0x00,  // dummy byte
        0x00,  // LSB receive
        0x00   // MSB receive
    };
    uint8_t rx_buf[6];
    
    spi_write_read(tx_buf, rx_buf, 6);
    return (rx_buf[5] << 8) | rx_buf[4];  // MSB first, then LSB
}

static uint32_t ft813_read_32(uint32_t addr)
{
    uint8_t tx_buf[8] = {
        (addr >> 16) & 0x3F,
        (addr >> 8) & 0xFF,
        addr & 0xFF,
        0x00,  // dummy byte
        0x00, 0x00, 0x00, 0x00  // 4 bytes to receive
    };
    uint8_t rx_buf[8];
    
    spi_write_read(tx_buf, rx_buf, 8);
    return (rx_buf[7] << 24) | (rx_buf[6] << 16) | (rx_buf[5] << 8) | rx_buf[4];
}

static void ft813_write_8(uint32_t addr, uint8_t value)
{
    uint8_t tx_buf[4] = {
        0x80 | ((addr >> 16) & 0x3F),
        (addr >> 8) & 0xFF,
        addr & 0xFF,
        value
    };
    
    spi_write_read(tx_buf, NULL, 4);
}

static void ft813_write_16(uint32_t addr, uint16_t value)
{
    uint8_t tx_buf[5] = {
        0x80 | ((addr >> 16) & 0x3F),
        (addr >> 8) & 0xFF,
        addr & 0xFF,
        value & 0xFF,
        (value >> 8) & 0xFF
    };
    
    spi_write_read(tx_buf, NULL, 5);
}

static void ft813_write_32(uint32_t addr, uint32_t value)
{
    uint8_t tx_buf[7] = {
        0x80 | ((addr >> 16) & 0x3F),
        (addr >> 8) & 0xFF,
        addr & 0xFF,
        value & 0xFF,
        (value >> 8) & 0xFF,
        (value >> 16) & 0xFF,
        (value >> 24) & 0xFF
    };
    
    spi_write_read(tx_buf, NULL, 7);
}

// Host command definitions (from Newhaven examples)
#define FT_GPU_ACTIVE_M      0x00
#define FT_GPU_EXTERNAL_OSC  0x44
#define FT_GPU_PLL_48M       0x62

// Send 3-byte host command (critical for FT813 initialization)
static void ft813_host_command(uint8_t cmd)
{
    uint8_t tx_buf[3] = {
        cmd,
        (cmd >> 8) & 0xFF,
        (cmd >> 16) & 0xFF
    };
    
    // Send command via SPI transaction
    spi_transaction_t trans = {};
    trans.length = 3 * 8;  // 3 bytes = 24 bits
    trans.tx_buffer = tx_buf;
    spi_device_transmit(g_spi_handle, &trans);
}

// Power cycle and bootup sequence (from Newhaven examples)
static void ft813_reset_simple(void)
{
    ESP_LOGI(TAG, "Performing power cycle");
    
    // Power down
    gpio_set_level(FT813_PIN_PD, 0);
    vTaskDelay(pdMS_TO_TICKS(50));
    
    // Power up
    gpio_set_level(FT813_PIN_PD, 1);
    vTaskDelay(pdMS_TO_TICKS(50));
    
    ESP_LOGI(TAG, "Sending host commands");
    
    // CRITICAL: Send ACTIVE_M command to wake chip
    ft813_host_command(FT_GPU_ACTIVE_M);
    vTaskDelay(pdMS_TO_TICKS(20));  // Wait 20ms as per Newhaven examples
    
    // CRITICAL: Send EXTERNAL_OSC command
    ft813_host_command(FT_GPU_EXTERNAL_OSC);
    vTaskDelay(pdMS_TO_TICKS(10));  // Wait 10ms as per Newhaven examples
    
    ESP_LOGI(TAG, "Host commands sent, waiting for chip ready");
}

// Simple display list based on Newhaven examples  
esp_err_t ft813_basic_display_test(void)
{
    ESP_LOGI(TAG, "Waiting for FT813 chip ID 0x7C");
    
    // Poll chip ID until ready (Newhaven pattern)
    int retry = 0;
    uint8_t chip_id = 0;
    while (chip_id != 0x7C && retry < 100) {
        chip_id = ft813_read_8(FT813_REG_ID);
        if (chip_id == 0x7C) {
            ESP_LOGI(TAG, "✓ FT813 ready! Chip ID: 0x%02X (correct)", chip_id);
            break;
        }
        if (retry % 10 == 0) {  // Log every 10 attempts
            ESP_LOGW(TAG, "Chip ID: 0x%02X (expected 0x7C), retry %d/100", chip_id, retry);
        }
        vTaskDelay(pdMS_TO_TICKS(10));
        retry++;
    }
    
    if (chip_id != 0x7C) {
        ESP_LOGE(TAG, "✗ FT813 NOT responding after %d attempts, chip ID: 0x%02X", retry, chip_id);
        ESP_LOGE(TAG, "Possible issues:");
        ESP_LOGE(TAG, "  - SPI wiring (MOSI/MISO/CLK/CS)");
        ESP_LOGE(TAG, "  - Power supply to display");
        ESP_LOGE(TAG, "  - PD pin control");
        return ESP_FAIL;
    }
    
    // Configure display timing first
    ESP_LOGI(TAG, "Configuring display timing");
    
    ft813_write_16(FT813_REG_HSIZE, 800);
    ft813_write_16(FT813_REG_HCYCLE, 928);
    ft813_write_16(FT813_REG_HOFFSET, 88);
    ft813_write_16(FT813_REG_HSYNC0, 0);
    ft813_write_16(FT813_REG_HSYNC1, 48);
    
    ft813_write_16(FT813_REG_VSIZE, 480);
    ft813_write_16(FT813_REG_VCYCLE, 525);
    ft813_write_16(FT813_REG_VOFFSET, 32);
    ft813_write_16(FT813_REG_VSYNC0, 0);
    ft813_write_16(FT813_REG_VSYNC1, 3);
    
    ft813_write_8(FT813_REG_SWIZZLE, 0);
    ft813_write_8(FT813_REG_PCLK_POL, 1);
    ft813_write_8(FT813_REG_CSPREAD, 0);
    ft813_write_8(FT813_REG_DITHER, 1);
    
    // Read and log the system frequency to verify chip is running
    uint32_t freq = ft813_read_32(FT813_REG_FREQUENCY);
    ESP_LOGI(TAG, "System frequency: %lu Hz (should be ~60MHz)", freq);
    
    // Configure GPIO for backlight - try enabling ALL GPIO pins
    // Set all GPIO pins (0-7) as outputs and set them all high
    ft813_write_8(FT813_REG_GPIO_DIR, 0xFF);  // All GPIOs as output
    ft813_write_8(FT813_REG_GPIO, 0xFF);      // All GPIOs high
    
    // Set PWM for backlight control to maximum brightness
    ft813_write_16(FT813_REG_PWM_HZ, 1000);   // 1000Hz PWM frequency
    ft813_write_8(FT813_REG_PWM_DUTY, 128);   // 100% duty cycle (full brightness)
    
    ESP_LOGI(TAG, "All GPIO pins enabled (0xFF) and PWM at maximum");
    
    // Enable display with PCLK (MUST be last!)
    ft813_write_8(FT813_REG_PCLK, 2);
    vTaskDelay(pdMS_TO_TICKS(100));
    
    ESP_LOGI(TAG, "Display timing configured and PCLK enabled");
    
    // Verify PCLK was set
    uint8_t pclk_readback = ft813_read_8(FT813_REG_PCLK);
    ESP_LOGI(TAG, "PCLK readback: %d (should be 2)", pclk_readback);
    
    // Create a simple display list directly in display RAM
    ESP_LOGI(TAG, "Writing display list");
    
    uint32_t dl_addr = FT813_RAM_DL;
    
    // Clear color to blue
    ft813_write_32(dl_addr, 0x02000000 | (0x20 << 16) | (0x40 << 8) | 0x80); 
    dl_addr += 4;
    
    // Clear screen
    ft813_write_32(dl_addr, 0x26000007); // CLEAR all buffers
    dl_addr += 4;
    
    // Set color to white
    ft813_write_32(dl_addr, 0x04FFFFFF); // COLOR_RGB white
    dl_addr += 4;
    
    // Display command
    ft813_write_32(dl_addr, 0x00000000); // DISPLAY
    dl_addr += 4;
    
    // Swap display list
    ft813_write_8(FT813_REG_DLSWAP, 2);  // DLSWAP_FRAME
    
    ESP_LOGI(TAG, "Basic display list created and swapped");
    
    // Check if display list was processed
    vTaskDelay(pdMS_TO_TICKS(100));
    uint8_t dlswap = ft813_read_8(FT813_REG_DLSWAP);
    ESP_LOGI(TAG, "DLSWAP status: %d (0=done)", dlswap);
    
    // Read back display configuration to verify
    uint16_t hsize = ft813_read_16(FT813_REG_HSIZE);
    uint16_t vsize = ft813_read_16(FT813_REG_VSIZE);
    uint8_t gpio = ft813_read_8(FT813_REG_GPIO);
    ESP_LOGI(TAG, "Display config: %dx%d, GPIO=0x%02X", hsize, vsize, gpio);
    
    ESP_LOGI(TAG, "*** If backlight is on, you should see a blue screen now! ***");
    
    return ESP_OK;
}

esp_err_t ft813_simple_test(void)
{
    ESP_LOGI(TAG, "=== FT813 Simple Test Starting ===");
    
    // Configure GPIO
    esp_err_t ret = ft813_configure_gpio_simple();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure GPIO");
        return ret;
    }
    
    // Configure SPI
    spi_bus_config_t buscfg = {
        .mosi_io_num = FT813_PIN_MOSI,
        .miso_io_num = FT813_PIN_MISO,
        .sclk_io_num = FT813_PIN_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 4096,
    };
    
    ret = spi_bus_initialize(FT813_SPI_HOST, &buscfg, SPI_DMA_CH_AUTO);
    if (ret != ESP_OK && ret != ESP_ERR_INVALID_STATE) {
        ESP_LOGE(TAG, "Failed to initialize SPI bus");
        return ret;
    }
    
    spi_device_interface_config_t devcfg = {
        .clock_speed_hz = 4000000,  // 4MHz for initial communication (Arduino standard)
        .mode = 0,                  // SPI Mode 0 (CPOL=0, CPHA=0)
        .spics_io_num = FT813_PIN_CS,
        .queue_size = 7,
    };
    
    ret = spi_bus_add_device(FT813_SPI_HOST, &devcfg, &g_spi_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to add SPI device");
        return ret;
    }
    
    ESP_LOGI(TAG, "SPI configured at 4MHz for initialization");
    
    // Reset FT813
    ft813_reset_simple();
    
    // Run basic display test
    ft813_basic_display_test();
    
    ESP_LOGI(TAG, "=== FT813 Simple Test Completed ===");
    return ESP_OK;
}