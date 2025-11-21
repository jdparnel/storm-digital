#include <stdio.h>
#include <stdarg.h>
#include "screens.h"
#include "ft813.h"
#include "max31856.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// Local logging wrappers to avoid dependency issues with esp_log macros
static void screens_log(const char *level, const char *fmt, ...) {
    va_list ap; va_start(ap, fmt);
    printf("%s (SCREENS) ", level);
    vprintf(fmt, ap);
    printf("\n");
    va_end(ap);
}
#define ESP_LOGI(TAG, fmt, ...) screens_log("I", fmt, ##__VA_ARGS__)
#define ESP_LOGW(TAG, fmt, ...) screens_log("W", fmt, ##__VA_ARGS__)
#include <stdio.h>
#include <stdbool.h>

// Button tags
#define TAG_HOME_NAV_BUTTON     100
#define TAG_SECONDARY_NAV_BUTTON 101

// Current screen state
static screen_id_t current_screen = SCREEN_HOME;
static uint8_t prev_tag = 0;

// Navigation functions
void screens_set_current(screen_id_t screen) {
    current_screen = screen;
}

screen_id_t screens_get_current(void) {
    return current_screen;
}

static const char *SCREENS_TAG __attribute__((unused)) = "SCREENS";

// Home screen with navigation button
void screen_home(const ft813_touch_t *inputs)
{
    static uint32_t press_count = 0;
    static bool prev_touching = false;
    bool button_pressed = false;
    
    // Edge detection for button release - navigate on release
    if (inputs && inputs->touching && inputs->tag == TAG_HOME_NAV_BUTTON) {
        button_pressed = true;
        prev_tag = TAG_HOME_NAV_BUTTON;
        if (!prev_touching) {
            ESP_LOGI(SCREENS_TAG, "Home: button press tag=%u x=%d y=%d", inputs->tag, inputs->x, inputs->y);
        }
    } else if (prev_tag == TAG_HOME_NAV_BUTTON && !inputs->touching) {
        // Button was released - navigate to secondary screen
        screens_set_current(SCREEN_SECONDARY);
        press_count++;
        ESP_LOGI(SCREENS_TAG, "Navigate to Secondary (count=%lu)", press_count);
        prev_tag = 0;
    } else {
        prev_tag = inputs ? inputs->tag : 0;
    }

    if (inputs && inputs->touching && !prev_touching && inputs->tag == 0) {
        ESP_LOGI(SCREENS_TAG, "Home: general touch x=%d y=%d", inputs->x, inputs->y);
    }
    if ((!inputs || !inputs->touching) && prev_touching) {
        ESP_LOGI(SCREENS_TAG, "Home: touch release");
    }
    prev_touching = inputs && inputs->touching;
    
    // Build display list
    ft813_cmd32(0x02FFFFFF);  // CLEAR_COLOR_RGB(255,255,255) - White background
    ft813_cmd32(0x26000007);  // CLEAR(1,1,1)
    
    // Title
    ft813_cmd32(0x04000000);  // COLOR_RGB(0,0,0) - Black text
    ft813_cmd_text(400, 60, 31, FT813_OPT_CENTER, "Home Screen");
    
    // Touch status
    char buf[64];
    if (inputs && inputs->touching) {
        ft813_cmd32(0x0400FF00);  // COLOR_RGB(0,255,0) - Green text
        snprintf(buf, sizeof(buf), "Touch: X=%d Y=%d", inputs->x, inputs->y);
        ft813_cmd_text(400, 130, 28, FT813_OPT_CENTER, buf);
        
        ft813_cmd32(0x04FF8000);  // COLOR_RGB(255,128,0) - Orange text
        snprintf(buf, sizeof(buf), "Tag: %u", inputs->tag);
        ft813_cmd_text(400, 170, 28, FT813_OPT_CENTER, buf);
    } else {
        ft813_cmd32(0x04808080);  // COLOR_RGB(128,128,128) - Gray text
        ft813_cmd_text(400, 130, 28, FT813_OPT_CENTER, "Not touching");
        ft813_cmd_text(400, 170, 28, FT813_OPT_CENTER, "Tag: 0");
    }
    
    // Navigation count
    ft813_cmd32(0x04000000);  // COLOR_RGB(0,0,0) - Black text
    snprintf(buf, sizeof(buf), "Navigation Count: %lu", press_count);
    ft813_cmd_text(400, 220, 28, FT813_OPT_CENTER, buf);
    
    // Navigation button with tag
    ft813_cmd_tag(TAG_HOME_NAV_BUTTON);
    ft813_cmd_button(250, 300, 300, 80, 29, button_pressed ? FT813_OPT_FLAT : 0, "Go to Secondary");
    ft813_cmd_tag(0);
    
    // Instructions
    ft813_cmd32(0x04606060);  // COLOR_RGB(96,96,96) - Dark gray
    ft813_cmd_text(400, 420, 26, FT813_OPT_CENTER, "Press and release button to navigate");
    
    ft813_swap();
}

// Secondary screen with back button
void screen_secondary(const ft813_touch_t *inputs)
{
    // Temperature state
    static max31856_reading_t temp_reading = {0};
    static uint32_t last_temp_read = 0;
    static bool prev_touching = false;
    uint32_t now = xTaskGetTickCount() * portTICK_PERIOD_MS;
    if (now - last_temp_read >= 500) {
        // Read temperature from continuous auto-conversion mode
        if (max31856_read_temperature(&temp_reading) == ESP_OK) {
            ESP_LOGI(SCREENS_TAG, "Temp: TC=%.2f CJ=%.2f", temp_reading.temp_c, temp_reading.cold_junction_c);
        } else {
            ESP_LOGW(SCREENS_TAG, "Temp read failed");
        }
        last_temp_read = now;
    }
    bool button_pressed = false;
    
    // Edge detection for button release - navigate on release
    if (inputs && inputs->touching && inputs->tag == TAG_SECONDARY_NAV_BUTTON) {
        button_pressed = true;
        prev_tag = TAG_SECONDARY_NAV_BUTTON;
        if (!prev_touching) {
            ESP_LOGI(SCREENS_TAG, "Secondary: back button press x=%d y=%d", inputs->x, inputs->y);
        }
    } else if (prev_tag == TAG_SECONDARY_NAV_BUTTON && !inputs->touching) {
        // Button was released - navigate back to home
        screens_set_current(SCREEN_HOME);
        ESP_LOGI(SCREENS_TAG, "Navigate to Home");
        prev_tag = 0;
    } else {
        prev_tag = inputs ? inputs->tag : 0;
    }

    if (inputs && inputs->touching && !prev_touching && inputs->tag == 0) {
        ESP_LOGI(SCREENS_TAG, "Secondary: general touch x=%d y=%d", inputs->x, inputs->y);
    }
    if ((!inputs || !inputs->touching) && prev_touching) {
        ESP_LOGI(SCREENS_TAG, "Secondary: touch release");
    }
    prev_touching = inputs && inputs->touching;
    
    // Build display list
    ft813_cmd32(0x02E0E0FF);  // CLEAR_COLOR_RGB(224,224,255) - Light blue background
    ft813_cmd32(0x26000007);  // CLEAR(1,1,1)
    
    // Title
    ft813_cmd32(0x04000080);  // COLOR_RGB(0,0,128) - Dark blue text
    ft813_cmd_text(400, 40, 31, FT813_OPT_CENTER, "Temperature Monitor");
    
    // Temperature display
    char buf[64];
    if (temp_reading.fault) {
        ft813_cmd32(0x04FF0000);  // COLOR_RGB(255,0,0) - Red for fault
        ft813_cmd_text(400, 140, 31, FT813_OPT_CENTER, "FAULT");
        
        ft813_cmd32(0x04800000);  // Dark red
        snprintf(buf, sizeof(buf), "Status: 0x%02X", temp_reading.fault_status);
        ft813_cmd_text(400, 190, 27, FT813_OPT_CENTER, buf);
    } else {
        // Main temperature - large display
        ft813_cmd32(0x04000000);  // COLOR_RGB(0,0,0) - Black text
        snprintf(buf, sizeof(buf), "%.2f C", temp_reading.temp_c);
        ft813_cmd_text(400, 140, 31, FT813_OPT_CENTER, buf);
        
        // Fahrenheit conversion
        float temp_f = (temp_reading.temp_c * 9.0f / 5.0f) + 32.0f;
        snprintf(buf, sizeof(buf), "%.2f F", temp_f);
        ft813_cmd_text(400, 190, 29, FT813_OPT_CENTER, buf);
    }
    
    // Cold junction temperature (both C and F)
    ft813_cmd32(0x04606060);  // COLOR_RGB(96,96,96) - Gray text
    float cj_f = (temp_reading.cold_junction_c * 9.0f / 5.0f) + 32.0f;
    snprintf(buf, sizeof(buf), "Cold Junction: %.2f C / %.2f F", temp_reading.cold_junction_c, cj_f);
    ft813_cmd_text(400, 240, 27, FT813_OPT_CENTER, buf);
    
    // Touch status (smaller, bottom area)
    if (inputs && inputs->touching) {
        ft813_cmd32(0x04008000);  // COLOR_RGB(0,128,0) - Green text
        snprintf(buf, sizeof(buf), "Touch: X=%d Y=%d", inputs->x, inputs->y);
        ft813_cmd_text(400, 290, 26, FT813_OPT_CENTER, buf);
    }
    
    // Back button with tag
    ft813_cmd_tag(TAG_SECONDARY_NAV_BUTTON);
    ft813_cmd_button(300, 360, 200, 70, 29, button_pressed ? FT813_OPT_FLAT : 0, "Back to Home");
    ft813_cmd_tag(0);
    
    // Instructions
    ft813_cmd32(0x04404040);  // COLOR_RGB(64,64,64) - Dark gray
    ft813_cmd_text(400, 450, 26, FT813_OPT_CENTER, "K-Type Thermocouple");
    
    ft813_swap();
}

// Main render function - dispatches to current screen
void screens_render(const ft813_touch_t *inputs)
{
    switch (current_screen) {
        case SCREEN_HOME:
            screen_home(inputs);
            break;
        case SCREEN_SECONDARY:
            screen_secondary(inputs);
            break;
        default:
            screen_home(inputs);
            break;
    }
}
