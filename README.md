# ESP32-S3-N16R8 Project with FT813 Display

This is an ESP-IDF project for the ESP32-S3-N16R8 module with FT813 EVE graphics controller.

## Hardware Specifications

### ESP32-S3-N16R8
- **Chip**: ESP32-S3
- **Flash**: 16MB
- **PSRAM**: 8MB (Octal SPI)
- **CPU**: Dual-core Xtensa LX7 @ 240MHz
- **Wi-Fi**: 802.11 b/g/n
- **Bluetooth**: Bluetooth 5.0 (LE)

### Display
- **Controller**: FT813 EVE Graphics
- **Display**: NHD-7.0-800480FT-CSXV-CTP
- **Resolution**: 800x480
- **Interface**: SPI @ 10MHz
- **Touch**: Capacitive (CTP)

## Hardware Connections

### FT813 Display to ESP32-S3 Wiring

| FT813 Signal | ESP32-S3 GPIO | Description |
|--------------|---------------|-------------|
| MOSI (SDI)   | GPIO 11       | SPI Master Out |
| MISO (SDO)   | GPIO 13       | SPI Master In |
| SCK          | GPIO 12       | SPI Clock |
| CS           | GPIO 10       | Chip Select |
| PD (Reset)   | GPIO 5        | Power Down / Reset |

**SPI Configuration:**
- **Mode**: 0 (CPOL=0, CPHA=0)
- **Speed**: 10 MHz
- **Host**: SPI2_HOST

**Important Notes:**
- ESP32-S3 GPIO26-37 are reserved for flash/PSRAM - do not use for FT813
- Ensure 3.3V power supply for FT813 display
- Connect GND between ESP32-S3 and display

## Features

- ✅ ESP32-S3 dual-core configuration
- ✅ 16MB flash memory support
- ✅ 8MB PSRAM initialization
- ✅ **FT813 GD2-style driver (WORKING!)**
- ✅ Text rendering with coprocessor
- ✅ Button widgets
- ✅ Graphics primitives
- 🚧 Touch input (planned)
- 🚧 Advanced widgets (planned)

## Prerequisites

Before building this project, make sure you have:

1. **ESP-IDF v5.5.1** (or compatible version) installed
2. **Espressif toolchain** properly configured
3. **VS Code** with ESP-IDF extension (optional but recommended)

## Quick Start

### Building and Flashing

```bash
# Set up ESP-IDF environment
source ~/esp/v5.5.1/esp-idf/export.sh

# Build the project
idf.py build

# Flash and monitor
idf.py flash monitor
```

### Expected Output

```
I (1290) FT813_GD2: Chip ID: 0x7C (expect 0x7C)
I (1290) FT813_GD2: FT813 initialized - black screen should be visible
I (1790) FT813_GD2: Commands complete: RP=WP=112
I (1790) FT813_GD2: Done! Check display for text and button
```

Display shows:
- White background
- "Hello World!" centered text
- "FT813 + GD2 Style" text
- "Click Me" button

## Project Structure

```
├── main/
│   ├── main.c                 # Application entry point
│   ├── ft813.c               # FT813 driver (GD2-style)
│   ├── ft813.h               # FT813 register definitions and API
│   └── CMakeLists.txt
├── docs/                      # Datasheets and reference docs
├── FT813_README.md           # Detailed FT813 documentation
└── README.md                 # This file
```

## Documentation

- **[FT813_README.md](FT813_README.md)** - Detailed FT813 driver documentation
- **[docs/](docs/)** - Hardware datasheets and programming guides

## VS Code Tasks

Pre-configured tasks available via **Ctrl+Shift+P** → **Tasks: Run Task**:
- `ESP-IDF Build` - Build the project
- `ESP-IDF Flash` - Flash to device
- `ESP-IDF Monitor` - Monitor serial output
- `ESP-IDF Clean` - Clean build files

Or use **Ctrl+Shift+B** to run the default build task.

## Project Structure

```
├── .github/
│   └── copilot-instructions.md    # Copilot workspace instructions
├── .vscode/
│   └── tasks.json                 # VS Code build tasks
├── main/
│   ├── CMakeLists.txt            # Main component CMake file
│   └── main.c                    # Main application code
├── CMakeLists.txt                # Root CMake file
├── sdkconfig                     # ESP-IDF configuration
└── README.md                     # This file
```

## Configuration

The project is pre-configured with optimal settings for ESP32-S3-N16R8:

- **Flash Size**: 16MB
- **PSRAM**: 8MB Octal SPI mode enabled
- **Flash Mode**: QIO (Quad I/O)
- **Flash Frequency**: 80MHz
- **PSRAM Speed**: 80MHz
- **Partition Table**: Single app with large partition

### Key Configuration Options

```
CONFIG_IDF_TARGET_ESP32S3=y
CONFIG_ESPTOOLPY_FLASHSIZE_16MB=y
CONFIG_ESP32S3_SPIRAM_SUPPORT=y
CONFIG_SPIRAM_SIZE=8388608
CONFIG_SPIRAM_MODE_OCT=y
CONFIG_SPIRAM_SPEED_80M=y
```

## Application Features

The main application demonstrates:

1. **System Information**: Displays chip info, flash size, and memory details
2. **PSRAM Testing**: Allocates and tests 1MB from PSRAM
3. **Memory Monitoring**: Continuous monitoring of heap and PSRAM usage
4. **Logging**: Structured logging with ESP_LOG macros

## Serial Monitor Output

Expected output on successful run:

```
I (xxx) ESP32S3-N16R8: ESP32-S3 Chip Information:
I (xxx) ESP32S3-N16R8: Model: esp32s3
I (xxx) ESP32S3-N16R8: Cores: 2
I (xxx) ESP32S3-N16R8: Features: /802.11bgn/BLE/Embedded-Flash:16 MB
I (xxx) ESP32S3-N16R8: Flash size: 16 MB
I (xxx) ESP32S3-N16R8: Free heap: xxxx bytes
I (xxx) ESP32S3-N16R8: Free PSRAM: xxxx bytes
I (xxx) ESP32S3-N16R8: Successfully allocated 1048576 bytes from PSRAM
I (xxx) ESP32S3-N16R8: Hello from ESP32-S3-N16R8! Counter: 0
```


## Next Steps

- [ ] Implement touch input handling
- [ ] Add more widget types (gauges, sliders, toggles)
- [ ] Create custom UI layouts
- [ ] Integrate with application logic

## Troubleshooting

### Common Issues

1. **No display output**: Check SPI connections and power
2. **Chip ID read fails**: Verify CS and SPI clock signals
3. **Commands not executing**: Check REG_CMD_READ vs REG_CMD_WRITE in logs
4. **Build fails**: Ensure ESP-IDF v5.5.1 environment is sourced

### Debug Tips

- Monitor serial output for FT813_GD2 log messages
- Check RP (read pointer) and WP (write pointer) values
- Verify Chip ID is 0x7C (FT813)

## Resources

- [ESP32-S3 Datasheet](https://www.espressif.com/sites/default/files/documentation/esp32-s3_datasheet_en.pdf)
- [FT813 Datasheet](docs/FT81x.pdf)
- [FT81X Programmer Guide](docs/FT81X_Series_Programmer_Guide.pdf)
- [GD2 Library Reference](https://github.com/jamesbowman/gd2-lib)

---

**Status**: ✅ FT813 display working as of November 21, 2025