# ESP32-S3-N16R8 Project Instructions

This is an ESP-IDF project for the ESP32-S3-N16R8 module with 16MB flash and 8MB PSRAM.

## Project Structure
- Uses ESP-IDF framework
- Main application in `main/` directory
- CMake build system
- VS Code integration with tasks

## Development Guidelines
- Use ESP-IDF APIs for hardware access
- Configure PSRAM for larger memory applications
- Follow ESP32-S3 specific pin mappings
- Use proper error handling for ESP functions

## Build Commands
- `idf.py build` - Build the project
- `idf.py flash` - Flash to device
- `idf.py monitor` - Monitor serial output