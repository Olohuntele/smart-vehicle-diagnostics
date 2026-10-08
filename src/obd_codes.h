#ifndef OBD_CODES_H
#define OBD_CODES_H

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
    const char* laymanAdvice;
};

// All 20 generic OBD-II fault codes with layman advice
const FaultCode FAULT_DATABASE[] = {
    {"P0300", "Engine Misfire", CRIT, "[CRIT]", "RED", "Engine is misfiring. Avoid high speeds; check spark plugs and ignition coils."},
    {"P0217", "Engine Overheat", CRIT, "[CRIT]", "RED", "Engine is overheating. Pull over safely immediately and check coolant levels."},
    {"P0700", "Transmission Fault", CRIT, "[CRIT]", "RED", "Transmission issue detected. Avoid hard acceleration and service transmission soon."},
    {"P0335", "Crankshaft Sensor", CRIT, "[CRIT]", "RED", "Crankshaft position sensor circuit failure. Engine may stall or fail to start."},
    {"P0340", "Camshaft Sensor", CRIT, "[CRIT]", "RED", "Camshaft position sensor circuit failure. Expect poor performance and rough idling."},
    {"P0600", "Serial Comm Fault", CRIT, "[CRIT]", "RED", "ECU internal communication link error. Professional diagnostic inspection recommended."},
    {"P0230", "Fuel Pump Circuit", CRIT, "[CRIT]", "RED", "Fuel pump primary circuit malfunction. Vehicle may fail to start or cut out."},
    
    {"P0171", "System Too Lean", WARN, "[WARN]", "YELLOW", "Air-fuel mixture too lean (too much air). Check for vacuum leaks or failing MAF sensor."},
    {"P0420", "Catalyst Efficiency", WARN, "[WARN]", "YELLOW", "Catalytic converter efficiency below threshold. Check exhaust system or oxygen sensors."},
    {"P0135", "O2 Sensor Heater", WARN, "[WARN]", "YELLOW", "Oxygen sensor heater circuit malfunction. Expect reduced fuel economy."},
    {"P0500", "Speed Sensor Fault", WARN, "[WARN]", "YELLOW", "Vehicle speed sensor malfunction. Speedometer or cruise control may fail."},
    {"P0102", "MAF Sensor Low", WARN, "[WARN]", "YELLOW", "Mass air flow sensor circuit low input. Check air filter and sensor wiring."},
    {"P0122", "Throttle Position", WARN, "[WARN]", "YELLOW", "Throttle/pedal position sensor low input. Expect sluggish acceleration or limp mode."},
    {"P0325", "Knock Sensor Fault", WARN, "[WARN]", "YELLOW", "Knock sensor circuit malfunction. Engine timing may retard to prevent damage."},
    {"P0401", "EGR Flow Insuff.", WARN, "[WARN]", "YELLOW", "Exhaust gas recirculation flow insufficient. Clean EGR valve and intake ports."},
    {"P0118", "Coolant Temp High", WARN, "[WARN]", "YELLOW", "Engine coolant temperature circuit high input. Check thermostat and coolant sensor."},
    
    {"P0455", "Evap Leak (Large)", ADVIS, "[ADVIS]", "GREEN", "Evaporative emission control system leak detected. Check gas cap tightness."},
    {"P0113", "Intake Air Temp", ADVIS, "[ADVIS]", "GREEN", "Intake air temperature circuit high input. Check intake sensor connection."},
    {"P0442", "Evap Leak (Small)", ADVIS, "[ADVIS]", "GREEN", "Small evaporative emission leak. Inspect gas cap and EVAP canister hoses."},
    {"P0141", "O2 Sensor Heater2", ADVIS, "[ADVIS]", "GREEN", "Downstream oxygen sensor heater circuit fault. Replace sensor soon."}
};

const int FAULT_COUNT = sizeof(FAULT_DATABASE) / sizeof(FAULT_DATABASE[0]);

#endif
