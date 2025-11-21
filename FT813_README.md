# FT813 Display Driver - Working Implementation

## Overview

This is a working GD2-style driver for the FT813 EVE graphics controller, successfully displaying text, buttons, and graphics on an 800x480 LCD display.

## Hardware

- **Display**: NHD-7.0-800480FT-CSXV-CTP (7" 800x480 with FT813)
- **MCU**: ESP32-S3-N16R8
- **Connection**: SPI at 10MHz

## Current Status ✅

**WORKING!** The display shows:
- White background
- "Hello World!" text centered at (400, 150)
- "FT813 + GD2 Style" text centered at (400, 250)
- "Click Me" button at (300, 320)

## Architecture

Based on the GD2 library pattern from jamesbowman/gd2-lib:

### Key Concepts

1. **FT813 is FT81x Generation**: Uses REG_CMDB_WRITE (0x302578) which is auto-incrementing
2. **Command Buffering**: Commands are buffered locally, then written in batches
3. **No Manual Pointer Management**: Hardware handles the write pointer automatically
4. **Simple Flush Model**: Just write accumulated bytes to REG_CMDB_WRITE

### Files

- `main/ft813_gd2_test.c` - Working GD2-style driver implementation
- `main/ft813_display.h` - Register definitions and function declarations
- `archive/` - Old non-working approaches (for reference)

## API Functions

### Initialization
```c
esp_err_t ft813_gd2_init(void);
```
Initializes the FT813, configures display timing for 800x480, and shows a black screen.

### Hello World Demo
```c
esp_err_t ft813_gd2_hello_world(void);
```
Displays text and a button using the coprocessor.

### Low-Level Functions

```c
static void cmd32(uint32_t val);           // Write 32-bit command
static void cmd_str(const char *s);        // Write string (auto-padded)
static void flush(void);                   // Write buffered commands to chip
static void finish(void);                  // Wait for coprocessor completion
```

### Coprocessor Commands

```c
static void cmd_dlstart(void);             // Start display list
static void cmd_swap(void);                // Swap display list
static void cmd_text(int16_t x, int16_t y, int16_t font, uint16_t options, const char *s);
static void cmd_button(int16_t x, int16_t y, int16_t w, int16_t h, int16_t font, uint16_t options, const char *s);
```

## SPI Communication

### Timing
- **Clock**: 10 MHz
- **Mode**: 0 (CPOL=0, CPHA=0)
- **Read timing**: Requires 1 dummy byte after 3-byte address

### Register Access

**Read 32-bit**:
```
TX: [addr_h] [addr_m] [addr_l] [dummy] [00] [00] [00] [00]
RX: [xx]     [xx]     [xx]     [xx]     [d0] [d1] [d2] [d3]
```
Data starts at rx[4].

**Write 32-bit to REG_CMDB_WRITE**:
```
TX: [0xB0] [0x25] [0x78] [d0] [d1] [d2] [d3]
```

## Display Configuration

**Timing (800x480 @ 60Hz)**:
- HCYCLE: 928, HOFFSET: 88, HSYNC0: 0, HSYNC1: 48
- VCYCLE: 525, VOFFSET: 32, VSYNC0: 0, VSYNC1: 3
- PCLK: 2, PCLK_POL: 1

## Building and Running

```bash
# Build
idf.py build

# Flash and monitor
idf.py flash monitor

# Expected output:
# I (1290) FT813_GD2: Chip ID: 0x7C (expect 0x7C)
# I (1290) FT813_GD2: FT813 initialized - black screen should be visible
# I (1790) FT813_GD2: Commands complete: RP=WP=112
# I (1790) FT813_GD2: Done! Check display for text and button
```

## Next Steps

- [ ] Add touch handling
- [ ] Implement more coprocessor commands (gauges, sliders, etc.)
- [ ] Add color graphics primitives
- [ ] Build actual UI for application

## Reference

- [GD2 Library](https://github.com/jamesbowman/gd2-lib) - Arduino library for FT8xx chips
- [FT813 Datasheet](https://brtchip.com/wp-content/uploads/Support/Documentation/Datasheets/ICs/DS_FT81x.pdf)
- [FT81x Programming Guide](https://brtchip.com/wp-content/uploads/Support/Documentation/Programming_Guides/ICs/EVE/FT81X_Series_Programmer_Guide.pdf)

## Breakthrough Moment

After multiple failed approaches, the breakthrough came from studying the GD2 library's transport layer. The key insight: **FT81x chips (like FT813) use REG_CMDB_WRITE for direct command writes**, which auto-increments internally. No need to track write pointers manually like FT800/801!

---

**Status**: ✅ Working as of November 21, 2025
**Author**: Storm Digital
