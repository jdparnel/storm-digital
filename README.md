# ESP32-S3-N16R8 Project

This is an ESP-IDF project specifically configured for the ESP32-S3-N16R8 module featuring 16MB flash memory and 8MB PSRAM.

## Hardware Specifications

- **Chip**: ESP32-S3
- **Flash**: 16MB
- **PSRAM**: 8MB (Octal SPI)
- **CPU**: Dual-core Xtensa LX7 @ 240MHz
- **Wi-Fi**: 802.11 b/g/n
- **Bluetooth**: Bluetooth 5.0 (LE)

## Features

- ✅ ESP32-S3 dual-core configuration
- ✅ 16MB flash memory support
- ✅ 8MB PSRAM initialization and testing
- ✅ System information display
- ✅ Memory allocation testing
- ✅ VS Code integration with tasks

## Prerequisites

Before building this project, make sure you have:

1. **ESP-IDF v5.5.1** (or compatible version) installed
2. **Espressif toolchain** properly configured
3. **VS Code** with ESP-IDF extension (optional but recommended)

## Installation

### 1. ESP-IDF Setup

If you haven't already, install ESP-IDF:

```bash
# Clone ESP-IDF
git clone --recursive https://github.com/espressif/esp-idf.git ~/esp/v5.5.1/esp-idf

# Navigate to ESP-IDF directory
cd ~/esp/v5.5.1/esp-idf

# Install ESP-IDF
./install.sh esp32s3

# Set up environment
. ./export.sh
```

### 2. Project Setup

```bash
# Clone or navigate to this project
cd /path/to/your/project

# Set the target to ESP32-S3
idf.py set-target esp32s3
```

## Building and Flashing

### Using Command Line

```bash
# Build the project
idf.py build

# Flash to the ESP32-S3 device
idf.py flash

# Monitor serial output
idf.py monitor

# Flash and monitor in one command
idf.py flash monitor
```

### Using VS Code

This project includes pre-configured VS Code tasks:

- **Ctrl+Shift+P** → **Tasks: Run Task**
- Choose from:
  - `ESP-IDF Build` - Build the project
  - `ESP-IDF Flash` - Flash to device
  - `ESP-IDF Monitor` - Monitor serial output
  - `ESP-IDF Clean` - Clean build files
  - `ESP-IDF Set Target ESP32-S3` - Set target chip

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

## Customization

To customize this project for your needs:

1. **Modify `main/main.c`** - Add your application logic
2. **Update `sdkconfig`** - Adjust configuration via `idf.py menuconfig`
3. **Add components** - Create additional components in separate directories
4. **Configure pins** - Use ESP32-S3 specific pin definitions

## Pin Configuration

ESP32-S3-N16R8 specific considerations:

- **GPIO0**: Boot mode selection
- **GPIO19, GPIO20**: USB D-, D+ (USB-Serial-JTAG)
- **GPIO26-32**: Octal SPI PSRAM (reserved)
- **GPIO33-37**: Octal SPI Flash (reserved)

Refer to ESP32-S3 datasheet for complete pin mapping.

## Troubleshooting

### Common Issues

1. **Build fails**: Ensure ESP-IDF environment is properly sourced
2. **Flash fails**: Check USB connection and port permissions
3. **PSRAM not detected**: Verify hardware and configuration
4. **Tasks not found**: Make sure ESP-IDF tools are in PATH

### Environment Variables

The tasks are configured to work with standard ESP-IDF installation paths. If your installation differs, update the PATH and IDF_PATH in `.vscode/tasks.json`.

## License

This project is provided as-is for development purposes.

## Resources

- [ESP32-S3 Datasheet](https://www.espressif.com/sites/default/files/documentation/esp32-s3_datasheet_en.pdf)
- [ESP-IDF Programming Guide](https://docs.espressif.com/projects/esp-idf/en/latest/)
- [ESP32-S3 Hardware Reference](https://docs.espressif.com/projects/esp-idf/en/latest/esp32s3/hw-reference/index.html)