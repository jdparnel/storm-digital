# Integrated Device PCB Architecture

## Overview
Single PCB integrating: USB-C 5V input, 1S2P LiPo battery pack, charger + power-path, soft power switch, 5V boost converter, 3.3V regulation, ESP32-S3-N16R8 module, FT813 display interface + backlight drive, MAX31856 thermocouple interface, Honeywell differential pressure sensors (±1, ±5, ±10, ±24, ±138 inwc), voltage divider networks, and supporting protection/ESD components.

Goal: Efficient, low-noise mixed-signal board enabling simultaneous temperature and differential pressure measurements with robust power management (USB or battery) and clean analog acquisition.

## Functional Blocks
1. USB-C Input & Protection
2. Battery Subsystem (1S2P LiPo pack)
3. Charger & Power-Path Management
4. Soft Power / Load Switch
5. DC/DC Conversion (Boost to 5V, Buck to 3.3V)
6. ESP32-S3 Core + PSRAM/Flash
7. Display (FT813) + Backlight
8. Thermocouple Front-End (MAX31856)
9. Differential Pressure Sensors (Honeywell analog)
10. Analog Conditioning & Voltage Dividers
11. Monitoring (Battery voltage, rails, temperature)
12. ESD & EMI Mitigation

## Power Path Summary
- Primary Source: USB-C 5V (sink only with CC pull-downs) OR LiPo pack.
- Battery Pack: 1S2P (two matched 18650 cells in parallel) for higher capacity; requires cell-matching and shared protection.
- Charger IC: Provides charge regulation and load sharing (e.g. MCP73871 or power-path charger like BQ24074).
- Boost Converter: Raises battery voltage (3.0–4.2V) to regulated 5V when USB absent; must supply sensors & display backlight peak current.
- 3.3V Regulator: High-efficiency buck from 5V (USB or boost). All digital logic and MAX31856 powered from 3.3V.

## Estimated Current Loads (Worst Case)
- Display Backlight: 300–400 mA (depending on brightness)
- ESP32-S3 Peak: ~250 mA (Wi-Fi + PSRAM operations spikes)
- Differential Sensors + MAX31856 + logic overhead: <100 mA
- Total Peak (5V domain): ~700–800 mA (size boost & USB traces accordingly)

## Charger & Battery Considerations
- Charge Current: Balance thermal vs. user charge time (e.g. 1.5A for 2x18650 may need thermal management).
- Protection: Include pack protection PCB or dedicated IC (over/under voltage, overcurrent, short circuit).
- Optional Fuel Gauge: Integrate MAX17048 or similar for accurate SOC reporting.

## Soft Power Switch
- Use high-side load switch (TPS22918 or similar) gating main 3.3V/5V distribution.
- Control Logic: Momentary button → MCU enable latch; MCU controls EN for graceful shutdown.
- Fallback / Recovery: Hardware test pads to bypass if firmware fault.

## DC/DC Components
- Boost (Battery→5V): Synchronous, low ripple, adequate thermal dissipation (metal shield region or copper pour).
- Buck (5V→3.3V): Low-noise regulator with good transient response (switching frequency >1MHz to minimize display interference).
- Place proper input/output bulk caps, local 0.1µF decoupling near each IC power pin.

## ESP32-S3 Integration
- Follow Espressif reference layout for RF: Keep ground under antenna clear (no copper where specified), maintain ground stitching vias.
- PSRAM/Flash already integrated in module variant (N16R8). Ensure stable 3.3V rail and adequate decoupling.

## Display (FT813) Interface
- SPI bus isolation from MAX31856 (separate host or careful routing to reduce crosstalk).
- Backlight PWM: Keep trace lengths short; add low-ESR capacitor near LED driver.
- EMI: Route high-current LED return directly to power ground star point.

## MAX31856 Thermocouple Section
- Slow SPI clock (10kHz init / 50kHz run) validated; keep traces short, avoid aggressive coupling with display clocks.
- Thermocouple inputs: Differential pair with guard ground pours; optional RC filter if long external cable.

## Differential Pressure Sensors
- Supply with regulated 5V (USB or boosted) with local decoupling (10µF + 0.1µF).
- Output lines → RC anti-alias (e.g. 100Ω series, 10nF to ground) before ADC pin/divider.
- Voltage Divider: 10kΩ / 22kΩ → ratio ~0.6875, ensures 4.5V scaled to ~3.09V.
- Group sensor inputs away from switching nodes; use analog ground reference tying into single-point system ground.

## Analog & Ground Strategy
- Single solid ground plane; star high-current returns (boost, backlight) at input power region.
- Keep analog sensor traces short, shielded by ground where possible.
- Avoid ground splits—use Ferrite bead(s) if necessary to isolate noisy domains.

## ESD & Protection
- USB-C: TVS diode for VBUS (e.g. SMCJ5.0 or smaller depending on expected surges) and ESD protection arrays for CC/D+ lines.
- External Connectors: Add TVS or series resistors for thermocouple and sensor harness if user-accessible.
- Reverse Polarity: Consider PFET ideal diode on battery input if pack can be user replaced.

## Monitoring & Telemetry
- Battery voltage ADC channel (divider sized for ~1mA max draw, e.g. 200k/47k + filter cap).
- Optional temperature sensor near boost or charger IC for thermal management.
- Fault flags: Charger status, thermocouple fault bits, sensor enumeration/zero status.

## Firmware Modules (Planned)
- power_manager: source selection, charger status, soft-switch control.
- pressure_sensor: per-range config, zero calibration, moving average filtering, conversion formula.
- thermocouple: existing MAX31856 continuous read, fault decode.
- battery_monitor: pack voltage → SOC estimation, low-voltage alarm.
- diagnostics_screen: aggregated system metrics.

## Preliminary BOM (Key ICs)
- Charger: MCP73871 / BQ24074
- Boost: TPS61236 / MP3429
- Buck 3.3V: TPS62133 / RT8059 series
- Load Switch: TPS22918
- Fuel Gauge (optional): MAX17048
- ESD: USBLC6-2 / ESD9L5.0
- Thermocouple ADC: MAX31856
- Differential Sensors: Honeywell analog ±1" to ±138" family
- MCU Module: ESP32-S3-N16R8
- Display Controller: FT813

## Layout Priorities
1. Power input & charging region
2. High-current paths (boost, backlight) with wide pours
3. RF/ESP32 isolation and keep-out rules
4. Sensitive analog (pressure, thermocouple) tucked away from switch nodes
5. Ground stitching vias near every decoupling cap
6. Thermal relief under high dissipation ICs (boost, charger)

## Bring-Up Checklist
- Verify 5V and 3.3V rails (no load → nominal) then with display + sensors active.
- Confirm charger behavior (current profile, termination voltage).
- Exercise soft switch on/off and ensure MCU retains latch control.
- Read ADC raw counts for each pressure sensor; perform zero calibration.
- Validate MAX31856 register writes at chosen SPI speeds.
- Check display backlight ripple does not corrupt analog readings (scope measurement on sensor output).
- Thermal soak test at max backlight + Wi-Fi toggling.

## Risks & Mitigations
- SPI integrity: Keep slow clock for MAX31856, route cleanly.
- Analog noise: Decouple, RC filter, ground shielding.
- Battery safety: Proper protection board and charger thermal management.
- USB surge/ESD: Robust TVS placement and short trace lengths.
- RF desense: Avoid noisy DC/DC near antenna region.

## Next Steps
1. Finalize component selection (charger, boost, buck).
2. Create schematic pages per block.
3. Review power sequencing & soft switch logic.
4. Start pressure_sensor firmware stub.
5. Layout initial placement and iterate on analog isolation.

This document will evolve alongside schematic capture and layout reviews.
