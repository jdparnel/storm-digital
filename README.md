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

### Thermocouple Sensor
- **Controller**: MAX31856 Precision Thermocouple-to-Digital Converter
- **Board**: Adafruit MAX31856 Breakout
- **Type**: K-Type Thermocouple (configurable for B, E, J, K, N, R, S, T)
- **Interface**: SPI @ 5MHz
- **Accuracy**: ±0.15% typical
- **Resolution**: 19-bit (0.0078125°C)
- **Cold Junction**: Built-in compensation with 14-bit resolution

## Power Supply

### ESP32-S3 Board Regulators

The ESP32-S3 development board uses one of the following 3.3V linear regulators:
- **AMS1117-3.3**: SOT-223 package, 1A max, ~1.1V dropout @ 800mA
- **SGM2212-3.3**: SOT-23-5/SOT-89 package, 300-600mA (variant dependent), ~200-300mV dropout

**Input**: USB 5V (typically 4.75-5.25V under load)  
**Output**: 3.3V regulated (expect 3.25-3.35V under normal load)

### Power Consumption Estimates

| Component | Idle/Typical | Peak | Notes |
|-----------|--------------|------|-------|
| ESP32-S3 Module | 40-70 mA | 240 mA | WiFi off, CPU active + PSRAM |
| FT813 + Display Logic | 100 mA | 100 mA | Per NH datasheet (NHD-7.0-800480FT) |
| Display Backlight | 760 mA | 760 mA | **@ 3.3V per NH datasheet; major load** |
| MAX31856 Sensor | 1-2 mA | 5 mA | Active continuous conversion |
| **Total (typical)** | **901-932 mA** | **1105 mA** | **Without WiFi; exceeds typical linear regulator capacity** |
| **Total (with WiFi)** | **1021-1112 mA** | **1285+ mA** | WiFi TX adds ~120-180 mA; **requires high-current supply** |

### Thermal Considerations

**Linear Regulator Heat Dissipation:**
- Power dissipated: *P = (Vin - Vout) × I*
- **Critical**: Display backlight draws 760 mA @ 3.3V per NH datasheet
- Example at 900 mA total: *(5.0V - 3.3V) × 0.9A = 1.53W*
- **AMS1117 (1A max) will be operating at/near limit with backlight on**
- **SGM2212 (300-600mA typical) CANNOT supply full backlight current**
- Case temperature will easily exceed **80-100°C** at 900+ mA continuous load

**⚠️ IMPORTANT: Backlight Power Considerations**
- **760 mA backlight @ 3.3V = 2.5W** is too high for most dev board linear regulators
- Many NH displays have **separate backlight power input** (check your specific model)
- If backlight shares 3.3V rail: **dedicated high-current supply or buck converter required**
- Typical solutions:
  1. **Separate backlight driver**: External constant-current LED driver with PWM dimming
  2. **Direct 5V backlight**: Some panels accept 5V directly (verify datasheet)
  3. **Buck converter**: High-efficiency DC/DC (e.g., LM2596, MP1584) for 3.3V @ 1.5A+
  4. **Reduce brightness**: PWM dimming to 30-50% can reduce current to 230-380 mA

**Normal Temperature Range:**
- Warm to touch (40-55°C): Normal under moderate load
- Hot but briefly touchable (55-70°C): Acceptable if within regulator specs; consider mitigation
- Too hot to touch (>70°C): **Expected with 760mA backlight on linear regulator**
- **>80°C**: Review load immediately; linear regulator likely inadequate

**Thermal Mitigation Options:**
1. **Reduce backlight brightness**: PWM duty cycle adjustment (biggest impact; 50% duty → ~380mA)
2. **Add heatsinking**: Copper tape or small clip-on heatsink to SOT-223 tab (AMS1117)
3. **External 3.3V supply**: Bypass USB 5V→3.3V drop with regulated switching supply
4. **Upgrade to buck converter**: **STRONGLY RECOMMENDED** - Replace linear regulator with high-efficiency DC/DC module (>85% efficiency vs ~66% for linear from 5V)
5. **Separate backlight supply**: Independent constant-current driver or boost converter for LED backlight

### Power Supply Pins

**FT813 Display:**
- VCC: 3.3V from ESP32-S3 board regulator
- GND: Common ground with ESP32-S3
- Display Logic: 100 mA (per NH datasheet)
- **Backlight: 760 mA @ 3.3V (per NH datasheet) - see thermal considerations above**
- **⚠️ Total display current (860 mA) likely exceeds onboard regulator capacity**

**MAX31856 Breakout:**
- VIN: 3.3V from ESP32-S3 board (breakout accepts 3.0-5.5V; onboard LDO provides 3.3V to IC)
- GND: Common ground with ESP32-S3
- Current: 1-2 mA typical (negligible impact on thermal budget)
- **Do NOT use 3VO pin as input** (it's a regulated output from the breakout's onboard LDO)

**Safety Notes:**
- Measure 3.3V rail under load before adding peripherals
- Ensure stable voltage (≥3.25V) during peak current draw
- Monitor regulator temperature during initial testing
- Consider USB power meter for real-time current monitoring

## Hardware Connections

### FT813 Display to ESP32-S3 Wiring

| FT813 Signal | ESP32-S3 GPIO | Description |
|--------------|---------------|-------------|
| MOSI (SDI)   | GPIO 11       | SPI Master Out |
| MISO (SDO)   | GPIO 13       | SPI Master In |
| SCK          | GPIO 12       | SPI Clock |
| CS           | GPIO 10       | Chip Select |
| PD (Reset)   | GPIO 5        | Power Down / Reset |
| VCC          | 3.3V          | Power supply |
| GND          | GND           | Ground |

**SPI Configuration:**
- **Mode**: 0 (CPOL=0, CPHA=0)
- **Speed**: 10 MHz
- **Host**: SPI2_HOST

### MAX31856 Thermocouple to ESP32-S3 Wiring

| MAX31856 Signal | ESP32-S3 GPIO | Description |
|-----------------|---------------|-------------|
| SDI (MOSI)      | GPIO 35       | SPI Master Out |
| SDO (MISO)      | GPIO 37       | SPI Master In |
| SCK             | GPIO 36       | SPI Clock |
| CS              | GPIO 15       | Chip Select (non-JTAG pin) |
| VIN             | 3.3V          | Power supply (3.0-5.5V input) |
| GND             | GND           | Ground |

**SPI Configuration:**
- **Mode**: 1 (CPOL=0, CPHA=1)
- **Initial Speed**: 10 kHz (configuration phase)
- **Runtime Speed**: 50 kHz (after successful config)
- **Host**: SPI3_HOST
- **Note**: Slow speeds required due to marginal MOSI signal integrity. Higher speeds (100kHz+) cause register write failures.

**Conversion Mode:**
- Continuous auto-conversion (CR0=0xB0)
- K-type thermocouple with 16x averaging (CR1=0x43)
- Open-circuit fault detection enabled
- Temperature updates every 500ms

**Thermocouple Connection:**
- **Yellow wire (+)** → TC+ terminal (Chromel, positive)
- **Red wire (−)** → TC− terminal (Alumel, negative)
- K-type thermocouple range: -200°C to +1372°C
- Cold junction compensation: automatic via onboard sensor

**Important Notes:**
- ESP32-S3 GPIO26-37 are **NOT** reserved on ESP32-S3 (safe for peripherals)
- Two independent SPI buses allow simultaneous display and sensor operation
- Ensure 3.3V power supply for both FT813 display and MAX31856
- Connect common GND between ESP32-S3, display, and thermocouple board

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