#include <Arduino.h>
#include "diagnostic.h"

// Configuration
const unsigned long IDLE_TIMEOUT = 60000; // 60 seconds of inactivity triggers Sleep
unsigned long lastActivityTime = 0;
String inputBuffer = "";

// Function Prototypes
void processCode(String code);
void goToSleep();
void updateDisplay(const char* title, const char* msg);

void setup() {
  Serial.begin(115200);
  delay(500);
  
  lastActivityTime = millis();
  
  Serial.println("\n========================================");
  Serial.println("     MYCAR-DOC - READY FOR ECU DATA     ");
  Serial.println("========================================");
  Serial.println("System online. Listening for incoming ECU fault codes...\n");
  
  updateDisplay("SYSTEM READY", "Waiting for ECU...");
}

void loop() {
  // 1. Non-blocking Serial Reading
  // Prevents the system from hanging while waiting for data
  while (Serial.available() > 0) {
    char c = (char)Serial.read();
    
    if (c == '\n' || c == '\r') {
      if (inputBuffer.length() > 0) {
        processCode(inputBuffer);
        inputBuffer = ""; // Reset buffer for next code
        lastActivityTime = millis(); // Reset idle timer
      }
    } else {
      inputBuffer += c;
    }
  }

  // 2. Power Management: Deep Sleep Logic
  // Automatically shuts down the ESP32 if no data is received within the timeout period
  if (millis() - lastActivityTime > IDLE_TIMEOUT) {
    goToSleep();
  }
}

void processCode(String code) {
  code.trim();
  bool found = false;
  
  for (int i = 0; i < FAULT_COUNT; i++) {
    if (code.equalsIgnoreCase(FAULT_DATABASE[i].code)) {
      Serial.println("\n----------------------------------------");
      Serial.print("Incoming Code: ");
      Serial.println(FAULT_DATABASE[i].code);
      Serial.print("👉 This is what your car is experiencing: ");
      Serial.println(FAULT_DATABASE[i].description);
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
    Serial.print("👉 This is what your car is experiencing: ");
    Serial.println("Unrecognized powertrain code.");
    Serial.println("----------------------------------------\n");
    
    updateDisplay("UNKNOWN CODE", code.c_str());
  }
}

void updateDisplay(const char* title, const char* msg) {
  // Logic for your LCD would go here.
  // Using Serial output as a placeholder for the physical display.
  Serial.print("[DISPLAY UPDATE] Title: ");
  Serial.print(title);
  Serial.print(" | Message: ");
  Serial.println(msg);
}

void goToSleep() {
  Serial.println("\nNo activity detected. Entering Deep Sleep to protect vehicle battery...");
  Serial.flush();
  
  // Configure wake-up source here (e.g., GPIO or timer)
  // esp_sleep_enable_timer_wakeup(10 * 1000000); // Example: wake every 10s
  
  esp_deep_sleep_start();
}
