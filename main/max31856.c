/*
 * MAX31856 Thermocouple-to-Digital Converter Driver
 * Precision thermocouple temperature measurement IC
 * Supports all standard thermocouple types (K, J, N, R, S, T, E, B)
 */

#include "max31856.h"
#include "driver/spi_master.h" // Ensure SPI3_HOST enum available
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_log.h"
#include "esp_rom_sys.h"   // esp_rom_delay_us
#include <string.h>

static const char *TAG = "MAX31856";
static spi_device_handle_t spi = NULL;
static spi_device_interface_config_t devcfg_current; // Track current SPI config

#if defined(SPI3_HOST)
#define MAX31856_SPI_HOST SPI3_HOST
#elif defined(VSPI_HOST)
#define MAX31856_SPI_HOST VSPI_HOST
#elif defined(SPI2_HOST)
#define MAX31856_SPI_HOST SPI2_HOST
#else
#define MAX31856_SPI_HOST 2
#endif

// SPI transaction helper (define early so other helpers can use it)
static void spi_xfer(const uint8_t *tx, uint8_t *rx, size_t len)
{
    spi_transaction_t t = {
        .length = len * 8,
        .tx_buffer = tx,
        .rx_buffer = rx
    };
    spi_device_transmit(spi, &t);
}

// Read single register (define early for verified write helper)
static uint8_t read_reg(uint8_t addr)
{
    uint8_t tx[2] = {addr & 0x7F, 0x00};  // Read = addr with MSB clear
    uint8_t rx[2] = {0};
    spi_xfer(tx, rx, 2);
    return rx[1];
}

static esp_err_t max31856_add_device(uint8_t spi_mode, int hz)
{
    if (spi) {
        spi_bus_remove_device(spi);
        spi = NULL;
    }
    memset(&devcfg_current, 0, sizeof(devcfg_current));
    devcfg_current.clock_speed_hz = hz;
    devcfg_current.mode = spi_mode;
    devcfg_current.spics_io_num = MAX31856_PIN_CS;
    devcfg_current.queue_size = 3;
    devcfg_current.flags = SPI_DEVICE_NO_DUMMY;
    ESP_LOGI(TAG, "Adding MAX31856 SPI device: mode=%u hz=%d", spi_mode, hz);
    return spi_bus_add_device(MAX31856_SPI_HOST, &devcfg_current, &spi);
}

// Verified register write with retry and microsecond delay between attempts
static esp_err_t write_reg_verified(uint8_t addr, uint8_t val)
{
    const int max_attempts = 5;
    for (int attempt = 1; attempt <= max_attempts; ++attempt) {
        uint8_t tx[2] = {addr | 0x80, val};
        spi_xfer(tx, NULL, 2);
        esp_rom_delay_us(10); // short settle
        uint8_t rb = read_reg(addr);
        if (rb == val) {
            if (attempt > 1) {
                ESP_LOGW(TAG, "Reg 0x%02X verified after %d attempts", addr, attempt);
            }
            return ESP_OK;
        }
    }
    ESP_LOGW(TAG, "Reg 0x%02X failed to latch value 0x%02X after %d attempts (last rb=0x%02X)", addr, val, max_attempts, read_reg(addr));
    return ESP_FAIL;
}

// Write single register (non‑verified legacy path, retained for internal use)
static void write_reg(uint8_t addr, uint8_t val)
{
    uint8_t tx[2] = {addr | 0x80, val};
    spi_xfer(tx, NULL, 2);
}

// Read multiple registers
static void read_regs(uint8_t addr, uint8_t *data, size_t len)
{
    uint8_t tx[16] = {0};
    uint8_t rx[16] = {0};
    
    if (len > 15) len = 15;  // Safety limit
    
    tx[0] = addr & 0x7F;  // Read address
    spi_xfer(tx, rx, len + 1);
    
    // Copy data (skip first byte which is dummy)
    memcpy(data, &rx[1], len);
}

// Initialize MAX31856
esp_err_t max31856_init(void)
{
    ESP_LOGI(TAG, "=== MAX31856 Thermocouple Initialization ===");
    
    // SPI bus configuration
    spi_bus_config_t buscfg = {
        .mosi_io_num = MAX31856_PIN_MOSI,
        .miso_io_num = MAX31856_PIN_MISO,
        .sclk_io_num = MAX31856_PIN_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 32
    };
    
    // Initialize SPI bus (use different host than FT813)
    ESP_ERROR_CHECK(spi_bus_initialize(MAX31856_SPI_HOST, &buscfg, SPI_DMA_CH_AUTO));
    
    // Add device initially with conservative settings (100 kHz) and SPI mode 1
    ESP_ERROR_CHECK(max31856_add_device(1, MAX31856_SPI_CLOCK_HZ_INIT));
    gpio_set_pull_mode(MAX31856_PIN_MISO, GPIO_PULLDOWN_ONLY); // reduce floating noise
    
    vTaskDelay(pdMS_TO_TICKS(100));  // Wait for device to stabilize
    
    // Configure continuous auto-conversion mode with K-type thermocouple
    write_reg_verified(MAX31856_REG_CR0, 0x00);  // Clear any previous state
    esp_rom_delay_us(100);
    
    write_reg_verified(MAX31856_REG_CR0, MAX31856_CR0_CMODE_AUTO | MAX31856_CR0_OCFAULT1 | MAX31856_CR0_OCFAULT0);  // 0xB0
    esp_rom_delay_us(100);
    
    write_reg_verified(MAX31856_REG_CR1, MAX31856_CR1_TC_TYPE_K | MAX31856_CR1_AVGSEL_16);
    esp_rom_delay_us(100);
    
    write_reg_verified(MAX31856_REG_MASK, 0x00);  // Unmask all faults
    esp_rom_delay_us(100);
    
    // Verify configuration
    uint8_t cr0_readback = read_reg(MAX31856_REG_CR0);
    uint8_t cr1_readback = read_reg(MAX31856_REG_CR1);
    
    if ((cr0_readback & 0xB0) == 0xB0 && (cr1_readback & 0x0F) == 0x03) {
        ESP_LOGI(TAG, "Continuous mode latched (CR0=0x%02X CR1=0x%02X)", cr0_readback, cr1_readback);
        
        // Speed up SPI now that config is stable
        ESP_ERROR_CHECK(max31856_add_device(devcfg_current.mode, MAX31856_SPI_CLOCK_HZ_RUN));
        vTaskDelay(pdMS_TO_TICKS(10));
    } else {
        ESP_LOGW(TAG, "Configuration verify: CR0=0x%02X CR1=0x%02X (expected CR0=0xB0 CR1=0x43)", 
                 cr0_readback, cr1_readback);
    }
    
    return ESP_OK;
}

// Trigger one-shot conversion (for normally-off mode)
esp_err_t max31856_trigger_oneshot(void)
{
    uint8_t cr0 = read_reg(MAX31856_REG_CR0);
    cr0 |= MAX31856_CR0_1SHOT;
    write_reg(MAX31856_REG_CR0, cr0);
    return ESP_OK;
}

// Perform a single-shot measurement (fallback path) and log raw decoded values
esp_err_t max31856_read_oneshot(max31856_reading_t *reading)
{
    if (!reading) return ESP_ERR_INVALID_ARG;
    // Trigger
    write_reg(MAX31856_REG_CR0, MAX31856_CR0_1SHOT);
    uint32_t start = xTaskGetTickCount();
    while ((xTaskGetTickCount() - start) < pdMS_TO_TICKS(500)) {
        uint8_t r = read_reg(MAX31856_REG_CR0);
        if ((r & MAX31856_CR0_1SHOT) == 0) break;
        vTaskDelay(pdMS_TO_TICKS(10));
    }
    // Read temps
    return max31856_read_temperature(reading);
}

// Read temperature from MAX31856
esp_err_t max31856_read_temperature(max31856_reading_t *reading)
{
    if (!reading) {
        return ESP_ERR_INVALID_ARG;
    }
    
    // Read fault status first
    reading->fault_status = read_reg(MAX31856_REG_SR);
    reading->fault = (reading->fault_status != 0);
    
    // Read cold junction temperature (CJTH, CJTL)
    // Datasheet: Cold junction is signed 14-bit value with resolution 0.015625°C (1/64)
    uint8_t cj_data[2];
    read_regs(MAX31856_REG_CJTH, cj_data, 2);
    int16_t cj_raw = (int16_t)((cj_data[0] << 8) | cj_data[1]);
    // Shift off 2 unused LSBs per datasheet
    cj_raw >>= 2; // Now 14-bit signed
    // Sign extend 14-bit (bit 13 sign)
    if (cj_raw & 0x2000) {
        cj_raw |= 0xC000;
    }
    reading->cold_junction_c = cj_raw * 0.015625f;
    
    // Read linearized thermocouple temperature (3 bytes, 19-bit signed)
    uint8_t tc_data[3];
    read_regs(MAX31856_REG_LTCBH, tc_data, 3);
    
    // Combine thermocouple temperature bytes
    int32_t tc_raw24 = ((int32_t)tc_data[0] << 16) | ((int32_t)tc_data[1] << 8) | tc_data[2];
    // Bits 18..0 after shifting right 5 are the signed temperature
    int32_t tc_19 = tc_raw24 >> 5;
    if (tc_19 & 0x00040000) { // sign bit (bit 18)
        tc_19 |= 0xFFF80000;  // sign extend
    }
    reading->temp_c = tc_19 * 0.0078125f; // 1/128°C per LSB
    
    return ESP_OK;
}

void max31856_dump_registers(void)
{
    if (!spi) {
        ESP_LOGW(TAG, "SPI device not initialized; cannot dump registers");
        return;
    }
    uint8_t regs[16];
    for (uint8_t addr = 0; addr <= MAX31856_REG_SR; ++addr) {
        regs[addr] = read_reg(addr);
    }
    ESP_LOGI(TAG, "MAX31856 REG DUMP:");
    ESP_LOGI(TAG, "CR0=0x%02X CR1=0x%02X MASK=0x%02X CJHF=0x%02X CJLF=0x%02X LTHFTH=0x%02X LTHFTL=0x%02X LTLFTH=0x%02X LTLFTL=0x%02X CJTO=0x%02X CJTH=0x%02X CJTL=0x%02X LTCBH=0x%02X LTCBM=0x%02X LTCBL=0x%02X SR=0x%02X",
             regs[0], regs[1], regs[2], regs[3], regs[4], regs[5], regs[6], regs[7], regs[8], regs[9], regs[10], regs[11], regs[12], regs[13], regs[14], regs[15]);
    // Decode common fault bits from SR
    uint8_t sr = regs[15];
    if (sr) {
        ESP_LOGW(TAG, "Fault Status: 0x%02X%s%s%s%s%s%s%s", sr,
            (sr & 0x80)?" OVUV":"",
            (sr & 0x40)?" CJ High":"",
            (sr & 0x20)?" CJ Low":"",
            (sr & 0x10)?" TC High":"",
            (sr & 0x08)?" TC Low":"",
            (sr & 0x04)?" OC Fault":"",
            (sr & 0x02)?" SCG":"",
            (sr & 0x01)?" SCV":"");
    } else {
        ESP_LOGI(TAG, "No faults reported");
    }
}

// Public helper to read CR0/CR1 for screen/debug logic
esp_err_t max31856_get_config(uint8_t *cr0, uint8_t *cr1)
{
    if (!spi) return ESP_ERR_INVALID_STATE;
    if (cr0) *cr0 = read_reg(MAX31856_REG_CR0);
    if (cr1) *cr1 = read_reg(MAX31856_REG_CR1);
    return ESP_OK;
}
