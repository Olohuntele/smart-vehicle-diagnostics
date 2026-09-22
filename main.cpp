#include <Arduino.h>
#include "diagnostic.h"

// Vehicle Operational States
enum VehicleState {
    IGNITION_ON,
    DIAGNOSTIC_SCAN,
    CLEARING_CODES,
    IGNITION_OFF_SLEEP
};

VehicleState currentState = IGNITION_ON;
int cycleCount = 0;

void setup() {
    Serial.begin(115200);
    delay(1000); // Allow serial monitor to stabilize
    
    Serial.println("==================================================");
    Serial.println("   SMART VEHICLE DIAGNOSTIC SIMULATION ENGINE     ");
    Serial.println("==================================================");
}

void loop() {
    switch (currentState) {
        case IGNITION_ON:
            Serial.println("\n[IGNITION] Vehicle turned ON. Initializing ECU link...");
            delay(2000);
            currentState = DIAGNOSTIC_SCAN;
            break;

        case DIAGNOSTIC_SCAN:
            Serial.print("[DIAGNOSTIC] Scanning live OBD-II data stream (");
            Serial.print(FAULT_COUNT);
            Serial.println(" codes loaded)...\n");
            
            for (int i = 0; i < FAULT_COUNT; ++i) {
                const FaultCode& current = FAULT_DATABASE[i];

                Serial.print("[SCAN] Code: ");
                Serial.print(current.code);
                Serial.print(" | Tier: ");
                Serial.print(current.tierTag);
                Serial.print(" | Color: ");
                Serial.print(current.colorIndicator);
                Serial.print(" | Desc: ");
                Serial.println(current.description);

                // **3-second transition delay between codes**
                delay(3000);
            }

            cycleCount++;
            
            if (cycleCount == 1) {
                currentState = CLEARING_CODES;
            } else {
                currentState = IGNITION_OFF_SLEEP;
            }
            break;

        case CLEARING_CODES:
            Serial.println("\n[SYSTEM] Clearing DTC Fault Codes from ECU memory...");
            delay(2000);
            Serial.println("[SYSTEM] ECU Memory Cleared Successfully. Status: CLEAN.\n");
            currentState = DIAGNOSTIC_SCAN;
            break;

        case IGNITION_OFF_SLEEP:
            Serial.println("\n[POWER MANAGEMENT] Vehicle ignition turned OFF.");
            Serial.println("[SLEEP MODE] Entering low-power sleep state. Disabling display and ECU polling to save battery...");
            delay(4000);
            Serial.println("[SLEEP MODE] Wake-up trigger received. Restarting cycle...");
            
            cycleCount = 0;
            currentState = IGNITION_ON;
            break;
    }
}
