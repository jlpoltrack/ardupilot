# RP2350B Flight Controller — Hardware Architecture Plan

*date: 2026-03-02*

## RP2350B Native Peripheral Budget (QFN-80)

- **48 GPIO** pins (5V-tolerant)
- **2× SPI**, **2× I2C**, **2× UART** (hardware)
- **8× ADC** channels (12-bit, 500 kSPS) — GP40–GP47
- **12 PWM slices** × 2 channels = **24 PWM** channels
- **3× PIO** blocks (12 state machines total)
- **USB** 1.1 device/host

## Design Goals

- 1× IMU (SPI), 1× Baro (I2C), no compass, no OSD
- SD card logging (SPI)
- 4× DShot motor outputs (PIO0)
- 8× PWM servo/auxiliary outputs (hardware PWM)
- 4× PIO UARTs (PIO1 + PIO2) — 6 UART + USB = 7 serial total
- 2–4 relay/PINIO outputs
- Flash-based parameter storage (onboard QSPI)
- All-LDO power supply (no SMPS)
- No buzzer, no safety switch

---

## ADC Analysis: LDO Power Is Sufficient

The RP2350 ADC uses **ADC_AVDD as its reference** (no internal ref). The dominant noise source on Pico 2 boards is the buck-boost SMPS feeding that pin.

**Using all-LDO power eliminates this entirely.** With an LDO + RC filter (100Ω + 100nF) on ADC_AVDD:

- **RP2350 ENOB**: 9.2 bits (up from RP2040's 8.7, DNL spikes fixed)
- **At 9 ENOB with 11:1 divider**: 36.3V / 512 ≈ 71 mV resolution
- **With 4× oversampling**: pushes to ~10 effective bits ≈ 35 mV

ArduPilot battery failsafes use ~100 mV thresholds — **no external reference needed.**

> [!TIP]
> PCB notes: dedicated LDO for ADC_AVDD, RC filter, short traces, 0.1% divider resistors for voltage sensing. Software oversampling recovers 1–2 extra bits.

---

## Complete Pin Map (all 48 GPIOs)

### Serial — 2× HW UART

| Port | Function | TX | RX |
|------|----------|----|----|
| SERIAL1 | Telemetry | GP0 | GP1 |
| SERIAL2 | GPS | GP8 | GP9 |

### SPI0 — IMU (ICM-42688P, MODE3, 2/16 MHz)

| Signal | GPIO |
|--------|------|
| SCK | GP2 |
| MOSI | GP3 |
| MISO | GP4 |
| CS | GP5 |

### SPI1 — SD Card (MODE0, 400 kHz / 25 MHz)

| Signal | GPIO |
|--------|------|
| SCK | GP10 |
| MOSI | GP11 |
| MISO | GP12 |
| CS | GP13 |

### I2C0 — Barometer (DPS310 / BMP280 at 0x76)

| Signal | GPIO |
|--------|------|
| SCL | GP16 |
| SDA | GP17 |

### I2C1 — External Peripherals

| Signal | GPIO |
|--------|------|
| SCL | GP18 |
| SDA | GP19 |

### PIO0 — DShot Motor Outputs (4 state machines)

| Motor | GPIO |
|-------|------|
| Motor 1 | GP20 |
| Motor 2 | GP21 |
| Motor 3 | GP22 |
| Motor 4 | GP23 |

### Hardware PWM — Servo / Aux Outputs (8 channels)

| Output | GPIO | Slice |
|--------|------|-------|
| PWM 1 | GP24 | 4A |
| PWM 2 | GP25 | 4B |
| PWM 3 | GP26 | 5A |
| PWM 4 | GP27 | 5B |
| PWM 5 | GP28 | 6A |
| PWM 6 | GP29 | 6B |
| PWM 7 | GP30 | 7A |
| PWM 8 | GP31 | 7B |

### PIO1 — Soft UARTs (4 state machines)

| Port | Function | TX | RX |
|------|----------|----|----|
| SERIAL3 | RC input | GP32 | GP33 |
| SERIAL4 | Spare | GP34 | GP35 |

### PIO2 — Soft UARTs (4 state machines)

| Port | Function | TX | RX |
|------|----------|----|----|
| SERIAL5 | Spare | GP36 | GP37 |
| SERIAL6 | Spare | GP38 | GP39 |

### ADC — Power Monitoring (GP40–GP47)

| Channel | GPIO | Function |
|---------|------|----------|
| ADC0 | GP40 | Battery voltage |
| ADC1 | GP41 | Battery current |
| ADC2 | GP42 | 5V rail sense |
| ADC3–7 | GP43–GP47 | **Spare** — usable as analog OR digital GPIO |

> [!NOTE]
> ADC pins are full GPIOs when not configured for analog. GP43–GP47 are 5 free I/O pins available for relay/PINIO, digital inputs, or future analog (2nd battery, airspeed, RSSI).

### Misc

| GPIO | Function |
|------|----------|
| GP6 | PINIO1 (relay output) |
| GP7 | PINIO2 (relay output) |
| GP14 | PINIO3 (relay output) |
| GP15 | Status LED |

### Storage

- **Parameters**: onboard QSPI flash (wear-leveled)
- **Logging**: SD card via SPI1

---

## Resource Summary

| Resource | Count | Notes |
|----------|-------|-------|
| Serial ports | 7 | 2 HW UART + 4 PIO UART + USB CDC |
| SPI buses | 2 | IMU + SD card |
| I2C buses | 2 | Baro + external |
| DShot outputs | 4 | PIO0 |
| PWM outputs | 8 | Hardware PWM slices 4–7 |
| ADC channels | 3 used | 5 spare (GP43–47, also usable as digital) |
| Relay / PINIO | 3+ | GP6, GP7, GP14 (+ spare ADC pins if needed) |
| LED | 1 | GP15 |
| **Free I/O** | **8** | **3 dedicated (GP6/7/14) + 5 spare ADC/GPIO (GP43–47)** |
| IMU | 1 | ICM-42688P (SPI) |
| Barometer | 1 | DPS310/BMP280 (I2C) |
| Compass | 0 | External probing via I2C1 |
| GPIOs assigned | **48 / 48** | 40 fixed-function, 8 spare/flexible |
| PIO blocks | **3 / 3** | DShot + 2× soft UART banks |

---

## Comparison vs MatekH743

| Feature | MatekH743 | RP2350B | Delta |
|---------|-----------|---------|-------|
| IMU | 2 | 1 | **−1** (no redundancy) |
| Baro | 1 | 1 | — |
| Compass | 0 (ext probe) | 0 (ext probe) | — |
| Serial ports | 9 | 7 | −2 (adequate) |
| PWM + DShot | 13 | 12 (8+4) | −1 |
| ADC channels | 4 | 8 | **+4** |
| I2C buses | 2 | 2 | — |
| SPI buses | 4 | 2 | −2 (sufficient) |
| CAN | 1 | 0 | **excluded** |
| OSD | MAX7456 | none | −1 (digital video era) |
| Relay / PINIO | 2 | 3 | +1 |
| Param storage | Flash | QSPI Flash | — |
| SD logging | SDMMC | SPI SD | slightly slower |
| MCU cost | ~$8–12 | ~$1.50 | **~80% cheaper** |

> [!IMPORTANT]
> The only functional gap vs MatekH743 (besides CAN) is **single-IMU** — no redundancy. Everything else is at parity or better. The OSD gap is irrelevant for digital video systems.
