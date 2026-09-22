# Smart Vehicle Diagnostic Hub

An ESP32-based hardware diagnostic and state machine simulation designed to model real-world vehicle OBD-II fault scanning, ECU memory clearing, and power management cycles.

## 🚀 Features
* **Multi-State Vehicle Lifecycle**: Simulates core automotive operational states including Ignition ON, Live Diagnostic Scanning, DTC Memory Clearing, and Low-Power Sleep Mode.
* **OBD-II Fault Database**: Houses a structured database of 20 generic diagnostic trouble codes (DTCs) complete with severity tiers and color indicators.
* **Precision Timing Loop**: Incorporates calibrated 3-second transmission delays to emulate real-world sensor polling intervals.
* **PlatformIO Environment**: Built and tested in a clean, modern embedded C++ toolchain targeting the ESP32 microcontroller.

## 📂 Project Structure
```text
smart-vehicle-diagnostics/
├── platformio.ini      # Project configuration and target build flags
├── src/
│   ├── main.cpp        # Core state machine logic and serial control loops
│   └── diagnostic.h    # OBD-II fault code structures and database
└── README.md
