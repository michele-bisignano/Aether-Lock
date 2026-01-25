/**
 * @file HardwareTest.cpp
 * @brief Physics Data Collector (CSV Generator)
 * PURPOSE: Generate Clean CSV data for R/MATLAB analysis (Linearity & Noise).
 */
#include <Arduino.h>
#include "Aether_HAL.h"

Aether_HAL hal;
bool running = false;
bool maxMode = false;
float pwm = 0.0f;
unsigned long lastTimestamp = 0;
bool increasing = true;

void setup() {
    Serial.begin(115200);
    hal.init();
    Serial.println("--- DATA COLLECTOR ---");
    Serial.println("[1] Start Ramp (0-100%) -> Generates CSV");
    Serial.println("[0] Stop");
}

void loop() {
    // 1. Handle Serial Commands
    if (Serial.available()) {
        char command = Serial.read();
        
        if (command == '1') { 
            running = true; 
            pwm = 0.0f; 
            increasing = true; 
            lastTimestamp = millis();
            // Header for CSV logging
            Serial.println("Time_ms,PWM_Percent,Raw_ADC"); 
        }
        
        if (command == '0') { 
            running = false; 
            hal.setCoilPower(0); 
            Serial.println("STOPPED"); 
        }

        if(command == '2'){
            running = true;
            maxMode = true; 
            pwm = 1.0f;
        }
    }

    // 2. Execute Test Logic (50Hz Sampling Rate)

    if (running && (millis() - lastTimestamp >= 20) && !maxMode) {
        lastTimestamp = millis();
        
        // Calculate PWM Ramp
        if (increasing) {
            pwm += 0.002f; // Ramp up (approx. 2 seconds to reach 100%)
            if (pwm >= 1.0f) {
                pwm = 1.0f;
                increasing = false; // Reverse direction
            }
        } else {
            pwm -= 0.002f; // Ramp down
            if (pwm <= 0.0f) {
                pwm = 0.0f;
                running = false; // Test completed
            }
        }
        
        // Apply Hardware Output
        hal.setCoilPower(pwm);
        
        // Log Data in CSV format
        Serial.print(millis());
        Serial.print(",");
        Serial.print(pwm * 100.0f, 2); // Print with 2 decimal places
        Serial.print(",");
        Serial.println(hal.readSensorRaw());

        // Handle Test Completion
        if (!running) {
            hal.setCoilPower(0);
            Serial.println("FINISHED");
        }
    } else if (running && maxMode) {
        // Max Power Mode
        hal.setCoilPower(1.0f);
        Serial.print(millis());
        Serial.print(",");
        Serial.print(100.0f, 2); // 100%
        Serial.print(",");
        Serial.println(hal.readSensorRaw());
    }
}