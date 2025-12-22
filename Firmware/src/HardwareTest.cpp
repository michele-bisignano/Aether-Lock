/**
 * @file HardwareTest.cpp
 * @brief Linear Ramp Test for Data Collection
 * 
 * PURPOSE: 
 * Generate a precise dataset to correlate PWM Duty Cycle vs Sensor Reading.
 * 
 * SETTINGS:
 * - Ramp Speed: +0.5% every 500ms
 * - Sampling Rate: 50 Hz (Every 20ms)
 * - Output Format: CSV (Time, PWM, SensorRaw) -> Ready for Excel
 */

#include <Arduino.h>
#include "Config.h"
#include "Aether_HAL.h"
// Nota: Non includiamo PID o MovingAverage, vogliamo i dati GREZZI.

// --- OBJECTS ---
Aether_HAL hal;

// --- VARIABLES ---
bool testRunning = false;
float currentPwm = 0.0f;

// Timing counters
unsigned long lastRampTime = 0;
unsigned long lastLogTime = 0;
unsigned long startTime = 0;

// Constants requested
const unsigned long RAMP_INTERVAL_MS = 500; // Step every 0.5 seconds
const float PWM_STEP_SIZE = 0.005f;         // +0.5% (0.005)
const unsigned long LOG_INTERVAL_MS = 20;   // 50 Hz (1000ms / 50 = 20ms)

void setup() {
    // 1. Init Serial
    Serial.begin(115200);
    
    // 2. Init Hardware
    hal.init();
    
    // Safety init
    hal.setCoilPower(0.0f);
    hal.setWarningLed(false);

    delay(1000);
    Serial.println("--- RAMP TEST DATA COLLECTOR ---");
    Serial.println("COMMANDS:");
    Serial.println(" [1] -> START RAMP (0% -> 100%, +0.5% every 0.5s)");
    Serial.println(" [0] -> STOP IMMEDIATELY");
    Serial.println("--------------------------------");
}

void loop() {
    // --- 1. SERIAL COMMANDS ---
    if (Serial.available() > 0) {
        char cmd = Serial.read();
        while(Serial.available()) Serial.read(); // Flush buffer

        if (cmd == '1') {
            // START
            testRunning = true;
            currentPwm = 0.0f;
            hal.setCoilPower(currentPwm);
            hal.setWarningLed(true);
            
            startTime = millis();
            lastRampTime = millis();
            lastLogTime = millis();

            // Print CSV Header for Excel
            Serial.println("Time_ms,PWM_Percent,Raw_ADC");
        }
        else if (cmd == '0') {
            // STOP
            testRunning = false;
            currentPwm = 0.0f;
            hal.setCoilPower(0.0f);
            hal.setWarningLed(false);
            Serial.println(">>> TEST STOPPED.");
        }
    }

    // --- 2. TEST LOGIC ---
    if (testRunning) {
        unsigned long now = millis();

        // A. RAMP UP LOGIC (Every 500ms)
        if (now - lastRampTime >= RAMP_INTERVAL_MS) {
            lastRampTime = now;
            
            // Increment PWM
            currentPwm += PWM_STEP_SIZE;

            // Safety Cap at 100%
            if (currentPwm >= 1.0f) {
                currentPwm = 1.0f;
                // Optional: Stop automatically at 100%?
                // testRunning = false; 
                // Serial.println(">>> MAX POWER REACHED");
            }

            // Apply to Coil
            hal.setCoilPower(currentPwm);
        }

        // B. DATA LOGGING (50 Hz / Every 20ms)
        if (now - lastLogTime >= LOG_INTERVAL_MS) {
            lastLogTime = now;

            // Read Sensor
            int rawVal = hal.readSensorRaw();

            // Print CSV Line
            // Format: Time (from start), PWM (0-100), RawValue
           // Serial.print(now - startTime);
            Serial.print(";");
            Serial.print(currentPwm * 100.0f, 1); // 1 decimal place
            Serial.print(";");
            Serial.println(rawVal);
        }
    }
}