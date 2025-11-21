#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "esp_heap_caps.h"
#include "esp_flash.h"
#include "esp_chip_info.h"
#include "nvs_flash.h"
#include "ft813_display.h"

static const char *TAG = "ESP32S3-N16R8";

void print_gpio_info(void)
{
    ESP_LOGI(TAG, "FT813 Display GPIO Configuration:");
    ESP_LOGI(TAG, "  SPI MOSI: GPIO %d", (int)FT813_PIN_MOSI);
    ESP_LOGI(TAG, "  SPI MISO: GPIO %d", (int)FT813_PIN_MISO);
    ESP_LOGI(TAG, "  SPI CLK:  GPIO %d", (int)FT813_PIN_CLK);
    ESP_LOGI(TAG, "  SPI CS:   GPIO %d", (int)FT813_PIN_CS);
    ESP_LOGI(TAG, "  Power Down: GPIO %d", (int)FT813_PIN_PD);
}

void print_system_info(void)
{
    // Get chip information
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);
    
    // Get flash information
    uint32_t flash_size = 0;
    esp_flash_get_size(NULL, &flash_size);
    
    ESP_LOGI(TAG, "ESP32-S3 Chip Information:");
    ESP_LOGI(TAG, "Model: %s", CONFIG_IDF_TARGET);
    ESP_LOGI(TAG, "Cores: %d", chip_info.cores);
    ESP_LOGI(TAG, "Features: %s%s%s%s%d%s",
             chip_info.features & CHIP_FEATURE_WIFI_BGN ? "/802.11bgn" : "",
             chip_info.features & CHIP_FEATURE_BLE ? "/BLE" : "",
             chip_info.features & CHIP_FEATURE_BT ? "/BT" : "",
             chip_info.features & CHIP_FEATURE_EMB_FLASH ? "/Embedded-Flash:" : "/External-Flash:",
             flash_size / (1024 * 1024), " MB");
    ESP_LOGI(TAG, "Silicon revision: %d", chip_info.revision);
    
    // Print flash information
    ESP_LOGI(TAG, "Flash size: %d MB", flash_size / (1024 * 1024));
    
    // Print memory information
    ESP_LOGI(TAG, "Memory Information:");
    ESP_LOGI(TAG, "Free heap: %d bytes", esp_get_free_heap_size());
    ESP_LOGI(TAG, "Free internal heap: %d bytes", heap_caps_get_free_size(MALLOC_CAP_INTERNAL));
    ESP_LOGI(TAG, "Free PSRAM: %d bytes", heap_caps_get_free_size(MALLOC_CAP_SPIRAM));
    ESP_LOGI(TAG, "Largest free PSRAM block: %d bytes", heap_caps_get_largest_free_block(MALLOC_CAP_SPIRAM));
    ESP_LOGI(TAG, "Total PSRAM: %d bytes", heap_caps_get_total_size(MALLOC_CAP_SPIRAM));
}

void test_ft813_display(void)
{
    ESP_LOGI(TAG, "=== Starting Simple FT813 Display Test ===");
    
    // Run the simple test (it handles its own SPI initialization)
    esp_err_t ret = ft813_simple_test();
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "Simple FT813 test completed successfully!");
    } else {
        ESP_LOGE(TAG, "Simple FT813 test failed: %s", esp_err_to_name(ret));
    }
}

void app_main(void)
{
    ESP_LOGI(TAG, "ESP32-S3-N16R8 Project Starting...");
    
    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    
    // Print system information
    print_system_info();
    
    // Print GPIO pin configuration
    print_gpio_info();
    
    // Initialize and test FT813 display
    test_ft813_display();
    
    ESP_LOGI(TAG, "Setup completed successfully!");
    
    // Main application loop
    int counter = 0;
    while (1) {
        ESP_LOGI(TAG, "Hello from ESP32-S3-N16R8! Counter: %d", counter++);
        ESP_LOGI(TAG, "Free heap: %d bytes | Free PSRAM: %d bytes", 
                 esp_get_free_heap_size(), 
                 heap_caps_get_free_size(MALLOC_CAP_SPIRAM));
        
        vTaskDelay(pdMS_TO_TICKS(5000)); // Delay for 5 seconds
    }
}