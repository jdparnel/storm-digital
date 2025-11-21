# Differential Pressure Sensors Summary

Supported sensor ranges (each bipolar around zero):
- ±1" H2O (Full Scale Span = 2" inwc)
- ±5" H2O (FSS = 10")
- ±10" H2O (FSS = 20")
- ±24" H2O (FSS = 48" ≈ 60 mbar)
- ±138" H2O (FSS = 276" ≈ 5 psi)

All devices share common electrical output characteristics:
- Supply Voltage (Vs): 5.0 V nominal
- Output Voltage Range: 0.5 V (minimum) to 4.5 V (maximum)
- Usable Span: 4.0 V corresponding to Full Scale Span (FSS)
- Output Transfer Function (ideal): Vout = 0.5 V + (Pressure_Fraction * 4.0 V)
  - Pressure_Fraction = (Pressure_inwc + (FSS/2)) / FSS

Detailed Output Transfer Function
---------------------------------
General ratiometric form (any supply Vs):

   Vout = Vs * [ 0.10 + 0.80 * ( (P - Pmin) / FSS ) ]

Where:
  - Vs      = sensor supply voltage (nominal 5.0 V)
  - P       = applied differential pressure (inwc)
  - Pmin    = -FSS/2 (negative full-scale endpoint)
  - FSS     = full scale span (e.g. 20 for ±10")
  - 0.10Vs  = minimum output (~10% Vs)
  - 0.90Vs  = maximum output (~90% Vs)

Substituting Pmin = -FSS/2 and simplifying:

   Vout = Vs * [ 0.10 + 0.80 * ( (P + FSS/2) / FSS ) ]

For Vs = 5.0 V this reduces to the fixed-voltage expression:

   Vout = 0.5 V + (P + FSS/2)/FSS * 4.0 V

Inverse (solve for pressure):

   P = ( (Vout/Vs - 0.10) / 0.80 ) * FSS + Pmin

Simplified for Vs = 5.0 V:

   P = ( (Vout - 0.5) / 4.0 ) * FSS - FSS/2

Use the general ratiometric form if Vs deviates from 5.0 V; measuring actual Vs and applying it removes supply drift error. The sensor’s specified accuracy already assumes Vs within its recommended operating range.

Key Performance Specs (from datasheet family summary):
- Accuracy: ±0.25 %FSS (BFSL)   (Best Fit Straight Line)
- Output Resolution: 0.03 %FSS
- (Typical worst-case endpoint accuracy is higher; budget ~±0.35–0.4 %FSS if needed.)

Derived Metrics Per Range
-------------------------
For each sensor, FSS is the total span (e.g. ±10" → 20").

1. ±1" sensor (FSS = 2")
   - Accuracy: ±0.25% of 2" = ±0.005" inwc
   - Output Resolution: 0.03% of 2" = 0.0006" inwc
   - Voltage per inch: 4.0 V / 2" = 2.0 V/inwc
   - Accuracy Voltage: ±0.005" * 2.0 V = ±0.010 V
   - Resolution Voltage: 0.0006" * 2.0 V ≈ 0.0012 V (1.2 mV)

2. ±5" sensor (FSS = 10")
   - Accuracy: ±0.025" inwc
   - Output Resolution: 0.003" inwc
   - V per inch: 4.0 V / 10" = 0.4 V/inwc
   - Accuracy Voltage: ±0.025 * 0.4 = ±0.010 V
   - Resolution Voltage: 0.003 * 0.4 = 0.0012 V

3. ±10" sensor (FSS = 20")
   - Accuracy: ±0.05" inwc
   - Output Resolution: 0.006" inwc
   - V per inch: 4.0 V / 20" = 0.2 V/inwc
   - Accuracy Voltage: ±0.05 * 0.2 = ±0.010 V
   - Resolution Voltage: 0.006 * 0.2 = 0.0012 V

4. ±24" sensor (FSS = 48")
   - Accuracy: ±0.12" inwc
   - Output Resolution: 0.0144" inwc
   - V per inch: 4.0 V / 48" ≈ 0.08333 V/inwc
   - Accuracy Voltage: ±0.12 * 0.08333 ≈ ±0.010 V
   - Resolution Voltage: 0.0144 * 0.08333 ≈ 0.0012 V

5. ±138" sensor (FSS = 276")
   - Accuracy: ±0.345" inwc
   - Output Resolution: 0.0828" inwc
   - V per inch: 4.0 V / 276" ≈ 0.01449 V/inwc
   - Accuracy Voltage: ±0.345 * 0.01449 ≈ ±0.010 V
   - Resolution Voltage: 0.0828 * 0.01449 ≈ 0.0012 V

Observation: Accuracy voltage (±0.010 V) and resolution voltage (~1.2 mV) remain constant across ranges because both are percentages of the fixed 4.0 V span.

ESP32-S3 ADC Interface
----------------------
- Native ADC full-scale (configured target): 0–3.1 V (12-bit, 4096 counts).
- Required voltage divider to map 0.5–4.5 V into ≤3.1 V: Ratio ≈ 0.6875 (example: R1=10 kΩ (top), R2=22 kΩ (bottom)).
  - Scaled span: 4.0 V * 0.6875 = 2.75 V across FSS.
  - LSB size: 2.75 V / 4096 ≈ 0.000671 V = 0.671 mV.
  - Sensor intrinsic resolution at ADC pin: 1.2 mV * 0.6875 ≈ 0.825 mV (~1.23 ADC counts).

ADC Pressure Resolution per Range (after divider):
- Step (inwc) = FSS / 4096.
  - ±1" (FSS=2"): 0.000488"/count
  - ±5" (FSS=10"): 0.00244"/count
  - ±10" (FSS=20"): 0.00488"/count
  - ±24" (FSS=48"): 0.01172"/count
  - ±138" (FSS=276"): 0.06738"/count

Comparison Hierarchy (All Ranges):
Sensor Accuracy > Sensor Resolution > ADC Quantization (except the largest range which still maintains ADC < sensor resolution). The ADC does not limit achievable accuracy; real-world error dominated by sensor specs and installation offsets.

Conversion Formulas
-------------------
Variables:
- `adc_counts` : raw 12-bit reading (0–4095)
- `Vadc` : voltage at ADC pin
- `Vsensor` : reconstructed original sensor output voltage
- `FSS` : full scale span in inches of water column

Steps:
1. `Vadc = (adc_counts * 3.1f) / 4095.0f;`
2. `Vsensor = Vadc / 0.6875f;`  // Undo divider ratio
3. `pressure_inwc = ((Vsensor - 0.5f) / 4.0f) * FSS - (FSS / 2.0f);`
   - Derived from linear mapping: 0.5 V → -FSS/2 ; 4.5 V → +FSS/2.

Simplified direct equation:
`pressure_inwc = ((Vsensor / 5.0f) * FSS) - (FSS / 2.0f);`
(Equivalent since Vsensor ranges 0.5–4.5 V.)

Zeroing & Calibration
---------------------
1. At startup with no differential pressure applied, collect N samples (e.g. N=32) and compute average raw pressure.
2. Store zero offset in NVS (ESP-IDF) for persistence across resets.
3. Apply offset after conversion: `pressure_corrected = pressure_inwc - zero_offset;`
4. Periodic re-zero option (user-initiated) to account for tubing or installation changes.

Filtering & Stability
---------------------
- Moving average (e.g. 8–16 samples) smooths quantization noise and sensor jitter.
- Optional oversampling (sum multiple ADC readings) can add effective bits; primarily aesthetic once below sensor resolution.
- Track min/max over operational intervals to detect drift (power or wiring issues).

Error Budget Snapshot (Example ±10" Sensor)
- Accuracy (BFSL): ±0.05" inwc
- Worst-case endpoint (est.): ±0.07–0.08" inwc (design margin)
- Resolution: 0.006" inwc (sensor), 0.00488" (ADC step)
- Practical displayed noise after smoothing: ≈0.005–0.01" inwc.

Design Recommendations
----------------------
- Use precision resistors (1% or better) for divider; optionally measure actual ratio for calibration.
- Keep sensor ground and ESP32 ground low impedance; avoid ground bounce (affects millivolt-level readings).
- Shield or twist differential pressure tubing if long runs to reduce mechanical noise.
- Periodically log raw Vsensor and computed pressure for diagnostics.
- Provide user-facing zero function on UI.

Example Pseudocode
------------------
```c
#define DIVIDER_RATIO 0.6875f
#define ADC_FS_VOLT   3.1f
#define VSUPPLY       5.0f

float read_pressure_inwc(uint32_t adc_counts, float fss_span, float zero_offset) {
    float vadc = (adc_counts * ADC_FS_VOLT) / 4095.0f;        // Voltage at ADC pin
    float vsensor = vadc / DIVIDER_RATIO;                     // Original sensor voltage
    float pressure = ((vsensor / VSUPPLY) * fss_span) - (fss_span * 0.5f); // Bipolar mapping
    return pressure - zero_offset;                            // Apply stored zero calibration
}
```

Constants & Macros
------------------
To avoid magic numbers and keep formulas consistent across ranges, define shared constants:
```c
#define PRESS_VS                       5.0f        // Nominal sensor supply voltage
#define PRESS_OFFSET_FRACTION          0.10f       // 10% of Vs at negative full-scale
#define PRESS_SPAN_FRACTION            0.80f       // 80% of Vs dynamic span
#define PRESS_DIVIDER_RATIO            0.6875f     // Voltage divider ratio (Vadc = Vsensor * ratio)
#define PRESS_ACCURACY_FRACTION_BFSL   0.0025f     // ±0.25% FSS (BFSL) expressed as fraction
#define PRESS_RESOLUTION_FRACTION      0.0003f     // 0.03% FSS output resolution

// Precompute from fractions (general ratiometric form):
// Vout = Vs * (PRESS_OFFSET_FRACTION + PRESS_SPAN_FRACTION * ((P - Pmin)/FSS))
// Pmin = -FSS/2
// Inverse: P = ( (Vout/Vs - PRESS_OFFSET_FRACTION) / PRESS_SPAN_FRACTION ) * FSS + Pmin

static inline float pressure_from_voltage(float vout, float fss) {
   float pmin = -0.5f * fss;
   return ( (vout / PRESS_VS - PRESS_OFFSET_FRACTION) / PRESS_SPAN_FRACTION ) * fss + pmin;
}

static inline float voltage_from_pressure(float p, float fss) {
   float pmin = -0.5f * fss;
   return PRESS_VS * (PRESS_OFFSET_FRACTION + PRESS_SPAN_FRACTION * ((p - pmin) / fss));
}

static inline float pressure_from_adc_counts(uint32_t counts, float fss) {
   // Convert ADC counts -> Vsensor voltage -> pressure
   float vadc    = (counts * ADC_FS_VOLT) / 4095.0f; // ADC pin voltage
   float vsensor = vadc / PRESS_DIVIDER_RATIO;       // Restore sensor output voltage
   return pressure_from_voltage(vsensor, fss);
}
```

Why Condense Values:
- Enhances readability (domain terms instead of raw decimals).
- Enables easy adaptation if using a slightly different sensor family (change fractions only).
- Simplifies calibration: measuring actual `PRESS_VS` lets you recompute pressure without extra drift.
- Clear separation between hardware scaling (divider ratio) and sensor model (offset/span fractions).

Calibration Helpers (Optional):
```c
typedef struct {
   float fss;          // Full scale span for this sensor
   float zero_offset;  // Learned zero offset after N-sample averaging
} pressure_channel_t;

static inline float pressure_apply_zero(float p, const pressure_channel_t *ch) {
   return p - ch->zero_offset;
}
```

Precomputed Range Equations
---------------------------
Let n = Vout / Vs (normalized sensor output). Using the condensed form:

   P = (1.25 * FSS) * n - (0.625 * FSS)

Per supported range (FSS = 2 * range):
- ±1"   (FSS=2)   : P = 2.5  * n - 1.25
- ±5"   (FSS=10)  : P = 12.5 * n - 6.25
- ±10"  (FSS=20)  : P = 25   * n - 12.5
- ±24"  (FSS=48)  : P = 60   * n - 30
- ±138" (FSS=276) : P = 345  * n - 172.5

These constants eliminate on-the-fly multiplication by fractions (0.10, 0.80) and reduce floating point operations inside high-rate sampling loops. For zero capture, evaluate P at the averaged baseline n_zero and store as the channel's `zero_offset`.

With these macros and helpers, application code becomes concise:
```c
uint32_t adc_counts = adc1_get_raw(channel);
float p_raw = pressure_from_adc_counts(adc_counts, sensor_channel.fss);
float p     = pressure_apply_zero(p_raw, &sensor_channel);
```

Next Steps
----------
- Implement driver (`pressure_sensor.c/.h`) with range configuration, zeroing, moving average filter, and periodic diagnostic logging.
- Integrate new screen for pressure display and calibration controls.

This document summarizes the electrical interface, performance, and conversion approach for all supported differential pressure sensor ranges.
