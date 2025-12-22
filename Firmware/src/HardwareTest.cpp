/**
 * @file HardwareTest.cpp
 * @brief Calibration & Tuning Laboratory
 * 
 * MODES:
 * 1. SENSOR TEST: Read raw ADC values to calibrate the project_config.json.
 * 2. KP AUTO-TUNING: Gradually increase Kp until the system starts oscillating.
 */

#include <Arduino.h>
#include "Config.h"
#include "Aether_HAL.h"
#include "PID_Controller.h"
#include "MovingAverage.h"

// --- TEST OBJECTS ---
Aether_HAL hal;
// Temporary PID controller for tuning purposes
PID_Controller pid(0.0, 0.0, 0.0, Config::Control::LOOP_PERIOD_S);
MovingAverage filter(Config::Control::FILTER_SIZE);

// Auto-Tuning Variables
bool tuningActive = false;
float testKp = 0.0f;
unsigned long lastStepTime = 0;
unsigned long startTime = 0;
const int STEP_DELAY_MS = 500;      // Increment rate (5 times per second)
const float KP_INCREMENT = 0.0001f; // Fine adjustment step
const float GRAPH_SCALE = 1000000.0f; // Multiplier for Kp telemetry to see it on graph

void setup() {
    Serial.begin(115200);
    hal.init();
    
    startTime = millis();

    // Safety: Ensure actuator is off at startup
    hal.setCoilPower(0.0f);
    
    delay(1000);
    Serial.println("--- AETHER-LOCK TUNING LAB ---");
    Serial.println("[m] -> Monitor Mode (Read sensor only for calibration)");
    Serial.println("[a] -> AUTO-TUNE Kp (Increases Kp until STOP is pressed)");
    Serial.println("[s] -> STOP / EMERGENCY SHUTDOWN");
    Serial.println("------------------------------");
}

void loop() {
    // 1. Sensor Reading & Filtering
    int raw = hal.readSensorRaw();
    float filtered = filter.process((float)raw);
    float currentOutput = 0.0f; // Default 0% (Safe state)
    
    // Target value from Config (ensure this matches your updated JSON!)
    float target = Config::Control::TARGET_ADC;

    // Automatic Start Logic (Optional demo start)

    if (!tuningActive && millis() - startTime > 5000) {
        tuningActive = true;
        testKp = 0.0f; 
        pid.setKp(0.0f); pid.setKi(0.0f); pid.setKd(0.0f);
        Serial.println("\n>>> AUTO-TUNING STARTED (Demo Mode).");
    }

    // 2. Command Handling
    if (Serial.available() > 0) {
        char cmd = Serial.read();
        while(Serial.available()) Serial.read(); // Buffer flush

        if (cmd == 's') {
            tuningActive = false;
            testKp = 0.0f;
            pid.setKp(0.0f);
            hal.setCoilPower(0.0f);
            Serial.println("\n>>> STOP. System deactivated.");
        }
        else if (cmd == 'a') {
            tuningActive = true;
            testKp = 0.0f; // Start from zero
            pid.setKp(0.0f);
            pid.setKi(0.0f); // Proportional only for Ziegler-Nichols test
            pid.setKd(0.0f);
            Serial.println("\n>>> AUTO-TUNING STARTED. Keep your hand near the 's' key!");
        }
    }

    // 3. Auto-Tuning Logic
    if (tuningActive) {
        // Increment Kp at defined intervals
        if (millis() - lastStepTime > STEP_DELAY_MS) {
            lastStepTime = millis();
            testKp += KP_INCREMENT;
            pid.setKp(testKp);
        }

        // Compute PID output
        // PID returns a value between 0.0 and 1.0
        currentOutput = pid.compute(target, filtered);

        // Safety Clamp (Assicura che non superi mai il 100% matematico)
        if (currentOutput > 1.0f) currentOutput = 1.0f;
        if (currentOutput < 0.0f) currentOutput = 0.0f;

        hal.setCoilPower(currentOutput);
        hal.setWarningLed(true); 
    } else {
        // Manual / Safe Mode
        currentOutput = 0.0f;
        hal.setCoilPower(0.0f);
        hal.setWarningLed(false);
    }

    // 4. Telemetry (Optimized for Serial Plotter)
    static unsigned long lastPrint = 0;
    if (millis() - lastPrint > 50) {
        lastPrint = millis();
        
        // Format: >Label:Value for modern Serial Plotters
        Serial.print(">Raw:");
        Serial.print((int)filtered);
        Serial.print(",Target:");
        Serial.print((int)target);
        Serial.print(",Kp_scaled:"); 
        Serial.print(testKp * GRAPH_SCALE); 
        
        // MOSTRA PERCENTUALE (0% - 100%)
        // Moltiplichiamo il valore normalizzato (0.0-1.0) per 100
        Serial.print(",PWM_Percent:");
        Serial.println(currentOutput * 100.0f);
    }
}