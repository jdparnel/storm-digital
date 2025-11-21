# FT813 7-inch Display Connection Guide

This document describes how to connect the NHD-7.0-800480FT-CSXV-CTP display (with FT813 controller) to the ESP32-S3-N16R8.

## Display Specifications

- **Size**: 7-inch TFT LCD
- **Resolution**: 800 x 480 pixels
- **Controller**: FT813 EVE (Embedded Video Engine)
- **Interface**: SPI (to ESP32-S3)
- **Touch**: Capacitive Touch Panel (CTP)
- **Backlight**: LED backlight with PWM control
- **Voltage**: 3.3V logic, 5V power

## Pin Connections

### SPI Interface (FT813 to ESP32-S3)

| FT813/Display Pin | ESP32-S3 GPIO | Description |
|------------------|---------------|-------------|
| MOSI/SDA        | GPIO 11       | SPI Master Out Slave In |
| MISO/SDO        | GPIO 13       | SPI Master In Slave Out |
| SCL/SCLK        | GPIO 12       | SPI Clock |
| CS              | GPIO 10       | SPI Chip Select |
| PD              | GPIO 9        | Power Down (Reset) |
| INT             | GPIO 14       | Interrupt (optional) |

### Power Connections

| Display Pin | Connection | Description |
|-------------|------------|-------------|
| VCC/VDD     | 3.3V       | Logic power supply |
| GND         | GND        | Ground |
| VIN/5V      | 5V         | Display/backlight power |

### Backlight Control (Optional)

| Display Pin | ESP32-S3 GPIO | Description |
|-------------|---------------|-------------|
| BL_PWM      | GPIO 15       | Backlight PWM control |
| BL_EN       | GPIO 16       | Backlight enable |

## Physical Connection

1. **Power**: Connect 3.3V and 5V rails with proper current capacity (display can draw 500mA+)
2. **Ground**: Ensure solid ground connections
3. **SPI Wiring**: Keep SPI traces short and consider impedance matching for high-speed signals
4. **Shielding**: Use proper shielding for long cable runs to prevent EMI

## Wiring Diagram

```
ESP32-S3-N16R8                    FT813 Display Module
┌─────────────────┐               ┌──────────────────────┐
│                 │               │                      │
│ GPIO 11 (MOSI)  ├─────────────┤ MOSI/SDA             │
│ GPIO 13 (MISO)  ├─────────────┤ MISO/SDO             │
│ GPIO 12 (SCK)   ├─────────────┤ SCL/SCLK             │
│ GPIO 10 (CS)    ├─────────────┤ CS                   │
│ GPIO 9  (PD)    ├─────────────┤ PD (Power Down)      │
│ GPIO 14 (INT)   ├─────────────┤ INT (optional)       │
│                 │               │                      │
│ 3.3V            ├─────────────┤ VCC/VDD              │
│ 5V              ├─────────────┤ VIN/5V               │
│ GND             ├─────────────┤ GND                  │
│                 │               │                      │
│ GPIO 15 (PWM)   ├─────────────┤ BL_PWM (optional)    │
│ GPIO 16 (EN)    ├─────────────┤ BL_EN (optional)     │
└─────────────────┘               └──────────────────────┘
```

## Configuration Notes

1. **SPI Speed**: Configured for 10MHz (can be increased up to 30MHz for FT813)
2. **Display Timing**: Pre-configured for 800x480 @ ~60Hz refresh rate
3. **Touch**: Capacitive touch with up to 5-point multi-touch support
4. **Memory**: Display uses internal FT813 memory for graphics and commands

## Software Features

- **Graphics Acceleration**: Hardware-accelerated 2D graphics via FT813
- **Touch Processing**: Hardware-processed capacitive touch with gesture support
- **Font Rendering**: Built-in font rendering with multiple font sizes
- **Widget Library**: Pre-built UI widgets (buttons, sliders, gauges, etc.)
- **Display Lists**: Efficient command-based rendering system
- **PSRAM Integration**: Large graphics buffers stored in ESP32-S3 PSRAM

## Usage Example

```c
#include "ft813_display.h"

ft813_handle_t display;

void setup_display() {
    // Initialize FT813
    ft813_init(&display);
    ft813_configure_display(&display);
    
    // Create a simple UI
    ft813_cmd_start(&display);
    ft813_cmd_dlstart(&display);
    
    // Clear screen
    ft813_dl_clear_color_rgb(&display, 0, 0, 0);
    ft813_dl_clear(&display, 1, 1, 1);
    
    // Add text
    ft813_dl_color_rgb(&display, 255, 255, 255);
    ft813_cmd_text(&display, 400, 240, 31, 0x1800, "Hello World!");
    
    // Update display
    ft813_dl_display(&display);
    ft813_cmd_swap(&display);
    ft813_cmd_execute(&display);
}
```

## Troubleshooting

### Common Issues

1. **Display not responding**:
   - Check power connections (both 3.3V and 5V)
   - Verify SPI wiring
   - Ensure proper reset sequence

2. **Touch not working**:
   - Verify capacitive touch configuration
   - Check touch calibration
   - Ensure proper grounding

3. **Poor display quality**:
   - Check pixel clock settings
   - Verify display timing parameters
   - Ensure adequate power supply

4. **SPI Communication errors**:
   - Reduce SPI clock speed
   - Check for proper signal integrity
   - Verify CS timing

### Debug Commands

```c
// Check chip ID
uint8_t chip_id = ft813_get_chip_id(&display);
ESP_LOGI("FT813", "Chip ID: 0x%02X", chip_id);

// Read touch status
ft813_touch_t touch;
ft813_get_touch_points(&display, &touch, 1);
if (touch.pressed) {
    ESP_LOGI("TOUCH", "Touch at (%d, %d)", touch.x, touch.y);
}
```

## Performance Optimization

1. **Use PSRAM**: Store large graphics data in PSRAM
2. **Optimize Display Lists**: Minimize display list complexity
3. **Batch Commands**: Group related commands together
4. **Use Hardware Acceleration**: Leverage FT813's built-in graphics functions

## Power Consumption

- **Active**: ~200-500mA (depending on backlight)
- **Sleep**: <10mA with display off
- **Backlight**: 100-300mA (adjustable via PWM)

## Additional Resources

- FT813 Programming Guide
- EVE Graphics Controller Documentation
- ESP32-S3 SPI Driver Documentation