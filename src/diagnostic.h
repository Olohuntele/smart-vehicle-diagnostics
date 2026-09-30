#ifndef DIAGNOSTIC_H
#define DIAGNOSTIC_H

#include <Arduino.h>

enum SeverityLevel {
    CRIT,
    WARN,
    ADVIS
};

struct FaultCode {
    const char* code;           
    const char* description;    
    SeverityLevel severity;     
    const char* tierTag;        
    const char* colorIndicator; 
};

// All 20 generic OBD-II fault codes
// Storing as const char* pointers allows these strings to stay in Flash (PROGMEM)
const FaultCode FAULT_DATABASE[] = {
    {"P0300", "Engine Misfire", CRIT, "[CRIT]", "RED"},
    {"P0217", "Engine Overheat", CRIT, "[CRIT]", "RED"},
    {"P0700", "Transmission Fault", CRIT, "[CRIT]", "RED"},
    {"P0335", "Crankshaft Sensor", CRIT, "[CRIT]", "RED"},
    {"P0340", "Camshaft Sensor", CRIT, "[CRIT]", "RED"},
    {"P0600", "Serial Comm Fault", CRIT, "[CRIT]", "RED"},
    {"P0230", "Fuel Pump Circuit", CRIT, "[CRIT]", "RED"},
    
    {"P0171", "System Too Lean", WARN, "[WARN]", "YELLOW"},
    {"P0420", "Catalyst Efficiency", WARN, "[WARN]", "YELLOW"},
    {"P0135", "O2 Sensor Heater", WARN, "[WARN]", "YELLOW"},
    {"P0500", "Speed Sensor Fault", WARN, "[WARN]", "YELLOW"},
    {"P0102", "MAF Sensor Low", WARN, "[WARN]", "YELLOW"},
    {"P0122", "Throttle Position", WARN, "[WARN]", "YELLOW"},
    {"P0325", "Knock Sensor Fault", WARN, "[WARN]", "YELLOW"},
    {"P0401", "EGR Flow Insuff.", WARN, "[WARN]", "YELLOW"},
    {"P0118", "Coolant Temp High", WARN, "[WARN]", "YELLOW"},
    
    {"P0455", "Evap Leak (Large)", ADVIS, "[ADVIS]", "GREEN"},
    {"P0113", "Intake Air Temp", ADVIS, "[ADVIS]", "GREEN"},
    {"P0442", "Evap Leak (Small)", ADVIS, "[ADVIS]", "GREEN"},
    {"P0141", "O2 Sensor Heater2", ADVIS, "[ADVIS]", "GREEN"}
};

// Automatically calculate count to avoid manual errors when adding new codes
const int FAULT_COUNT = sizeof(FAULT_DATABASE) / sizeof(FAULT_DATABASE[0]);

#endif
