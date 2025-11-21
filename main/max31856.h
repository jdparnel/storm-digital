#ifndef MAX31856_H
#define MAX31856_H

#include <stdint.h>
#include <esp_err.h>
#include "driver/gpio.h"

// ============================================================================
// MAX31856 GPIO PIN DEFINITIONS
// ============================================================================
// Ensure SPI host enum is defined before use
// SPI host defined in implementation file to avoid enum ordering issues
// Use GPIO enums for clarity
#define MAX31856_PIN_MOSI       35
#define MAX31856_PIN_MISO       37
#define MAX31856_PIN_CLK        36
#define MAX31856_PIN_CS         15   // CS moved to GPIO15 (non-JTAG, dedicated for MAX31856)

// Initial slow SPI clock for configuration (increase later if stable)
#define MAX31856_SPI_CLOCK_HZ_INIT 10000   // 10 kHz VERY slow for marginal signaling
#define MAX31856_SPI_CLOCK_HZ_RUN  50000   // 50 kHz run speed (was 500kHz, too fast for this hardware)

// ============================================================================
// MAX31856 REGISTER ADDRESSES
// ============================================================================
#define MAX31856_REG_CR0        0x00    // Configuration 0 Register
#define MAX31856_REG_CR1        0x01    // Configuration 1 Register
#define MAX31856_REG_MASK       0x02    // Fault Mask Register
#define MAX31856_REG_CJHF       0x03    // Cold-Junction High Fault Threshold
#define MAX31856_REG_CJLF       0x04    // Cold-Junction Low Fault Threshold
#define MAX31856_REG_LTHFTH     0x05    // Linearized TC High Fault Threshold MSB
#define MAX31856_REG_LTHFTL     0x06    // Linearized TC High Fault Threshold LSB
#define MAX31856_REG_LTLFTH     0x07    // Linearized TC Low Fault Threshold MSB
#define MAX31856_REG_LTLFTL     0x08    // Linearized TC Low Fault Threshold LSB
#define MAX31856_REG_CJTO       0x09    // Cold-Junction Temperature Offset
#define MAX31856_REG_CJTH       0x0A    // Cold-Junction Temperature MSB
#define MAX31856_REG_CJTL       0x0B    // Cold-Junction Temperature LSB
#define MAX31856_REG_LTCBH      0x0C    // Linearized TC Temperature MSB
#define MAX31856_REG_LTCBM      0x0D    // Linearized TC Temperature Middle Byte
#define MAX31856_REG_LTCBL      0x0E    // Linearized TC Temperature LSB
#define MAX31856_REG_SR         0x0F    // Fault Status Register

// ============================================================================
// MAX31856 CONFIGURATION BITS
// ============================================================================
// CR0 Register bits
#define MAX31856_CR0_CMODE_AUTO     0x80    // Automatic conversion mode
#define MAX31856_CR0_CMODE_MANUAL   0x00    // Manual conversion mode
#define MAX31856_CR0_1SHOT          0x40    // One-shot mode
#define MAX31856_CR0_OCFAULT1       0x20    // Open circuit fault detection (fault mode 1)
#define MAX31856_CR0_OCFAULT0       0x10    // Open circuit fault detection (fault mode 0)
#define MAX31856_CR0_CJ             0x08    // Cold junction disable
#define MAX31856_CR0_FAULT          0x04    // Fault mode (0=comparator, 1=interrupt)
#define MAX31856_CR0_FAULTCLR       0x02    // Fault status clear

// CR1 Register - Thermocouple Type
#define MAX31856_CR1_TC_TYPE_B      0x00
#define MAX31856_CR1_TC_TYPE_E      0x01
#define MAX31856_CR1_TC_TYPE_J      0x02
#define MAX31856_CR1_TC_TYPE_K      0x03
#define MAX31856_CR1_TC_TYPE_N      0x04
#define MAX31856_CR1_TC_TYPE_R      0x05
#define MAX31856_CR1_TC_TYPE_S      0x06
#define MAX31856_CR1_TC_TYPE_T      0x07
#define MAX31856_CR1_AVGSEL_1       0x00    // 1 sample averaging
#define MAX31856_CR1_AVGSEL_2       0x10    // 2 samples averaging
#define MAX31856_CR1_AVGSEL_4       0x20    // 4 samples averaging
#define MAX31856_CR1_AVGSEL_8       0x30    // 8 samples averaging
#define MAX31856_CR1_AVGSEL_16      0x40    // 16 samples averaging

// Temperature reading structure
typedef struct {
    float temp_c;           // Temperature in Celsius
    float cold_junction_c;  // Cold junction temperature in Celsius
    bool fault;             // True if any fault detected
    uint8_t fault_status;   // Raw fault status register
} max31856_reading_t;

// Public API
esp_err_t max31856_init(void);
esp_err_t max31856_read_temperature(max31856_reading_t *reading);
esp_err_t max31856_trigger_oneshot(void);
void max31856_dump_registers(void);
esp_err_t max31856_read_oneshot(max31856_reading_t *reading);
// Retrieve configuration register values (CR0, CR1)
esp_err_t max31856_get_config(uint8_t *cr0, uint8_t *cr1);

#endif // MAX31856_H
