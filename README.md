# Smart Vehicle Diagnostic Hub (`mycar-doc`)

An advanced, ESP32-based hardware diagnostic and vehicle state machine system designed for real-time OBD-II fault scanning, ECU memory querying, live PID telemetry, and intelligent vehicle battery management.

---

## 🚀 Features

* **Multi-State Vehicle Lifecycle**: Simulates and manages core operational states: `BOOT`, `LIVE_TELEMETRY`, `FAULT_ALERT`, and `SYSTEM_SLEEP`.
* **Comprehensive OBD-II Fault Database (`obd_codes.h`)**: Houses 20 generic diagnostic trouble codes (DTCs) complete with severity tiers (`CRIT`, `WARN`, `ADVIS`), color indicators, and plain-English **Layman Advice** for non-technical vehicle owners.
* **Analog Battery & Telemetry Monitoring**: Real-time 12V–14V battery voltage sensing via ESP32 ADC (`GPIO 34`) with moving-average noise filtering and threshold alerts (Critical `<11.5V`, Low Battery `<12.0V`, Alternator Failure `<13.0V` when running).
* **CAN Bus & ECU Communication (`MCP2515`)**: Integrates SPI-based MCP2515 CAN transceivers at 500kbps to poll live parameter PIDs (RPM, Coolant Temperature, Vehicle Speed, Throttle Position) and execute Mode 03 DTC memory scans (`0x7DF` broadcast).
* **I2C LCD User Interface**: Drives a 16x2 I2C LCD display (`SDA:21`, `SCL:22`) to present real-time telemetry dashboards and fault warnings.
* **Intelligent Power Management & Deep Sleep**: Protects vehicle batteries by automatically entering ESP32 Deep Sleep upon ignition shutoff or inactivity timeouts, with dual wake-up triggers (**EXT0 Ignition Sense** on `GPIO 35` and **Periodic Timer Wake** every 5 minutes for health checks).

---

## 📂 Project Structure

```text
smart-vehicle-diagnostics/
├── platformio.ini      # Project configuration, build flags, and library dependencies
├── src/
│   ├── main.cpp        # Core state machine, ADC battery monitoring, CAN bus loop, and UI logic
│   └── obd_codes.h     # OBD-II fault code structures, database, and layman translations
└── README.md
```

---

## 🔌 Hardware Wiring Guide

| Component | ESP32 Pin | Description |
| :--- | :--- | :--- |
| **MCP2515 SPI (SCK)** | GPIO 18 | SPI Clock for CAN Bus |
| **MCP2515 SPI (MISO)** | GPIO 19 | SPI Master In Slave Out |
| **MCP2515 SPI (MOSI)** | GPIO 23 | SPI Master Out Slave In |
| **MCP2515 SPI (CS)** | GPIO 5 | Chip Select for MCP2515 |
| **Battery ADC** | GPIO 34 | Analog input for 12V battery voltage divider ($\sim$4.7:1 ratio) |
| **Ignition Sense** | GPIO 35 | Hardware ignition detection pin |
| **I2C LCD (SDA)** | GPIO 21 | I2C Data line for 16x2 LCD display |
| **I2C LCD (SCL)** | GPIO 22 | I2C Clock line for 16x2 LCD display |

---

## 🛠️ Build & Flash (PlatformIO)

1. Clone or open this repository in VS Code with the **PlatformIO IDE** extension.
2. Connect your ESP32 dev board via USB.
3. Build and upload the firmware:
   ```bash
   platformio run --target upload
   ```
4. Open the Serial Monitor (`115200` baud) to view real-time diagnostics and telemetry.
