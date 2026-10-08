#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <mcp2515.h>
#include "obd_codes.h"

// System States
enum SystemState {
  STATE_BOOT,
  STATE_LIVE_TELEMETRY,
  STATE_FAULT_ALERT,
  STATE_SLEEP
};

SystemState currentState = STATE_BOOT;

// Configuration & Timers
const unsigned long IDLE_TIMEOUT = 60000; // 60 seconds of inactivity triggers Sleep
unsigned long lastActivityTime = 0;
unsigned long telemetryInterval = 3000;   // 3 seconds transmission delay
unsigned long lastTelemetryTime = 0;
String inputBuffer = "";

// Hardware Pins & Telemetry Configuration
const int BATTERY_ADC_PIN = 34;    // GPIO 34 (ADC1_CH6) for battery voltage
const int IGNITION_SENSE_PIN = 35; // GPIO 35 for hardware ignition sense
const float VOLTAGE_DIVIDER_RATIO = 4.7f;
const int NUM_SAMPLES = 10;
float voltageSamples[NUM_SAMPLES];
int sampleIndex = 0;

// CAN Bus / MCP2515 Configuration (CS pin on GPIO 5)
struct can_frame canMsg;
MCP2515 mcp2515(5); // CS pin = 5

// LCD I2C Configuration (Address 0x27, 16 columns, 2 rows)
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Function Prototypes
void handleBootState();
void handleTelemetryState();
void handleSleepState();
void processCode(String code);
void updateDisplay(const char* line1, const char* line2);
float readBatteryVoltage();
void checkBatteryHealth(float voltage, bool engineRunning);
void initCANBus();
void requestOBDPIDs();
void scanMode03DTCs();

void setup() {
  Serial.begin(115200);
  delay(500);

  // Check Wakeup Cause from Deep Sleep
  esp_sleep_wakeup_cause_t wakeup_cause = esp_sleep_get_wakeup_cause();
  Serial.print("\n[BOOT] Wakeup Cause: ");
  switch (wakeup_cause) {
    case ESP_SLEEP_WAKEUP_EXT0:
      Serial.println("Ignition Sense Trigger (EXT0) - Vehicle ON");
      break;
    case ESP_SLEEP_WAKEUP_TIMER:
      Serial.println("Periodic Timer Wake - Stationary Battery Health Check");
      break;
    default:
      Serial.println("Fresh Power-On / Hardware Reset");
      break;
  }

  // Initialize I2C and LCD
  Wire.begin(21, 22); // SDA = 21, SCL = 22 on ESP32
  lcd.init();
  lcd.backlight();
  lcd.clear();

  // Initialize Pins
  pinMode(BATTERY_ADC_PIN, INPUT);
  pinMode(IGNITION_SENSE_PIN, INPUT);
  for (int i = 0; i < NUM_SAMPLES; i++) {
    voltageSamples[i] = 12.6f;
  }
  
  // Initialize CAN Bus
  initCANBus();

  currentState = STATE_BOOT;
  handleBootState();
}

void loop() {
  // 1. Non-blocking Serial / CAN Reading
  while (Serial.available() > 0) {
    char c = (char)Serial.read();
    
    if (c == '\n' || c == '\r') {
      if (inputBuffer.length() > 0) {
        processCode(inputBuffer);
        inputBuffer = "";
        lastActivityTime = millis();
      }
    } else {
      inputBuffer += c;
    }
  }

  // Check incoming CAN messages if any
  if (mcp2515.readMessage(&canMsg) == MCP2515::ERROR_OK) {
    lastActivityTime = millis();
    if (canMsg.can_id == 0x7E8) {
      Serial.print("[CAN RX] ID: 0x");
      Serial.print(canMsg.can_id, HEX);
      Serial.print(" Data: ");
      for (int i = 0; i < canMsg.can_dlc; i++) {
        Serial.print(canMsg.data[i], HEX);
        Serial.print(" ");
      }
      Serial.println();
    }
  }

  // 2. State Machine Execution
  switch (currentState) {
    case STATE_BOOT:
      currentState = STATE_LIVE_TELEMETRY;
      lastActivityTime = millis();
      break;

    case STATE_LIVE_TELEMETRY:
      if (millis() - lastTelemetryTime > telemetryInterval) {
        handleTelemetryState();
        lastTelemetryTime = millis();
      }
      
      // Check idle timeout or ignition off
      if (millis() - lastActivityTime > IDLE_TIMEOUT || digitalRead(IGNITION_SENSE_PIN) == LOW) {
        currentState = STATE_SLEEP;
      }
      break;

    case STATE_FAULT_ALERT:
      if (millis() - lastActivityTime > 10000) {
        currentState = STATE_LIVE_TELEMETRY;
        updateDisplay("TELEMETRY", "Resuming live...");
      }
      break;

    case STATE_SLEEP:
      handleSleepState();
      break;
  }
}

void initCANBus() {
  SPI.begin(18, 19, 23, 5); // SCK, MISO, MOSI, CS for ESP32 SPI
  mcp2515.reset();
  if (mcp2515.setBitrate(CAN_500KBPS, MCP_8MHZ) == MCP2515::ERROR_OK) {
    Serial.println("[CAN] MCP2515 Initialized at 500kbps successfully.");
  } else {
    Serial.println("[CAN] MCP2515 Initialization Failed! Check SPI wiring.");
  }
  mcp2515.setNormalMode();
}

void requestOBDPIDs() {
  struct can_frame requestFrame;
  requestFrame.can_id = 0x7DF; // OBD-II broadcast ID
  requestFrame.can_dlc = 8;
  requestFrame.data[0] = 0x02; // Number of additional bytes
  requestFrame.data[1] = 0x01; // Mode 1 (Current Data)
  requestFrame.data[2] = 0x0C; // PID 0x0C: Engine RPM
  requestFrame.data[3] = 0x05; // PID 0x05: Coolant Temperature
  requestFrame.data[4] = 0x0D; // PID 0x0D: Vehicle Speed
  requestFrame.data[5] = 0x11; // PID 0x11: Throttle Position
  requestFrame.data[6] = 0x00;
  requestFrame.data[7] = 0x00;

  mcp2515.sendMessage(&requestFrame);
  Serial.println("[CAN TX] Sent OBD-II PID Request (RPM, Coolant, Speed, Throttle)");
}

void scanMode03DTCs() {
  struct can_frame dtcRequest;
  dtcRequest.can_id = 0x7DF;
  dtcRequest.can_dlc = 8;
  dtcRequest.data[0] = 0x01; // Number of bytes
  dtcRequest.data[1] = 0x03; // Mode 03 (Request Diagnostic Trouble Codes)
  dtcRequest.data[2] = 0x00;
  dtcRequest.data[3] = 0x00;
  dtcRequest.data[4] = 0x00;
  dtcRequest.data[5] = 0x00;
  dtcRequest.data[6] = 0x00;
  dtcRequest.data[7] = 0x00;

  mcp2515.sendMessage(&dtcRequest);
  Serial.println("[CAN TX] Sent Mode 03 DTC Scan Request");
}

void handleBootState() {
  Serial.println("\n========================================");
  Serial.println("   MYCAR-DOC - VEHICLE DIAGNOSTIC HUB   ");
  Serial.println("========================================");
  Serial.println("System Initializing & Starting CAN Bus...\n");
  updateDisplay("MYCAR-DOC HUB", "System Online");
}

float readBatteryVoltage() {
  int rawADC = analogRead(BATTERY_ADC_PIN);
  float pinVoltage = (rawADC / 4095.0f) * 3.3f;
  float measuredVoltage = pinVoltage * VOLTAGE_DIVIDER_RATIO;
  
  voltageSamples[sampleIndex] = measuredVoltage;
  sampleIndex = (sampleIndex + 1) % NUM_SAMPLES;
  
  float sum = 0;
  for (int i = 0; i < NUM_SAMPLES; i++) {
    sum += voltageSamples[i];
  }
  return sum / NUM_SAMPLES;
}

void checkBatteryHealth(float voltage, bool engineRunning) {
  Serial.print("[BATTERY] Voltage: ");
  Serial.print(voltage, 2);
  Serial.println(" V");

  if (voltage < 11.5f) {
    Serial.println("🚨 CRITICAL VOLTAGE ALERT: Risk of no-start condition!");
    updateDisplay("CRIT ALERT!", "Batt < 11.5V");
  } else if (!engineRunning && voltage < 12.0f) {
    Serial.println("⚠️ Low Battery Warning: Battery is below 12.0V.");
    updateDisplay("LOW BATTERY", "Charge Soon");
  } else if (engineRunning && voltage < 13.0f) {
    Serial.println("⚠️ Alternator Failure Warning: Charging voltage below 13.0V while running.");
    updateDisplay("ALT FAULT", "Check Alternator");
  }
}

void handleTelemetryState() {
  float battVoltage = readBatteryVoltage();
  bool engineRunning = (battVoltage > 13.2f);

  Serial.println("\n[ECU POLL] Requesting Live Telemetry & OBD-II PIDs...");
  checkBatteryHealth(battVoltage, engineRunning);
  requestOBDPIDs();

  char line2Buf[17];
  snprintf(line2Buf, sizeof(line2Buf), "B:%.1fV R:850 T:90C", battVoltage);
  updateDisplay("LIVE TELEMETRY", line2Buf);
}

void processCode(String code) {
  code.trim();
  bool found = false;
  
  for (int i = 0; i < FAULT_COUNT; i++) {
    if (code.equalsIgnoreCase(FAULT_DATABASE[i].code)) {
      currentState = STATE_FAULT_ALERT;
      lastActivityTime = millis();
      
      Serial.println("\n----------------------------------------");
      Serial.print("DTC Detected: ");
      Serial.print(FAULT_DATABASE[i].code);
      Serial.print(" ");
      Serial.println(FAULT_DATABASE[i].tierTag);
      Serial.print("Description: ");
      Serial.println(FAULT_DATABASE[i].description);
      Serial.print("Layman Advice: ");
      Serial.println(FAULT_DATABASE[i].laymanAdvice);
      Serial.println("----------------------------------------\n");
      
      updateDisplay(FAULT_DATABASE[i].code, FAULT_DATABASE[i].description);
      found = true;
      break;
    }
  }
  
  if (!found) {
    Serial.println("\n----------------------------------------");
    Serial.print("Incoming Code: ");
    Serial.println(code);
    Serial.println("👉 Unrecognized powertrain code.");
    Serial.println("----------------------------------------\n");
    
    updateDisplay("UNKNOWN CODE", code.c_str());
  }
}

void updateDisplay(const char* line1, const char* line2) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(line1);
  lcd.setCursor(0, 1);
  lcd.print(line2);

  Serial.print("[LCD DISPLAY] Line 1: ");
  Serial.print(line1);
  Serial.print(" | Line 2: ");
  Serial.println(line2);
}

void handleSleepState() {
  Serial.println("\n[POWER MANAGEMENT] No activity or Ignition OFF detected. Entering Deep Sleep...");
  Serial.flush();
  
  // Turn off LCD backlight to save battery
  lcd.noBacklight();
  updateDisplay("SYSTEM SLEEP", "Battery Protected");

  // Configure Wake-up Sources:
  // 1. Timer wakeup every 5 minutes for stationary battery health check
  esp_sleep_enable_timer_wakeup(5 * 60 * 1000000ULL);

  // 2. Ext0 GPIO wakeup on Ignition Sense pin (GPIO 35 goes HIGH when ignition turns ON)
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_35, 1);

  Serial.println("[POWER MANAGEMENT] ESP32 Deep Sleep initiated.");
  Serial.flush();

  // Enter ESP32 Deep Sleep Mode
  esp_deep_sleep_start();
}
