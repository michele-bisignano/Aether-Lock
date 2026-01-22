/**
 * @file HardwareTest.cpp
 * @brief Physics Data Collector (CSV Generator)
 * PURPOSE: Generate Clean CSV data for R/MATLAB analysis (Linearity & Noise).
 */
#include <Arduino.h>
#include "Aether_HAL.h"

Aether_HAL hal;
bool running = false;
float pwm = 0.0f;
unsigned long lastT = 0;

void setup() {
    Serial.begin(115200);
    hal.init();
    Serial.println("--- DATA COLLECTOR ---");
    Serial.println("[1] Start Ramp (0-100%) -> Generates CSV");
    Serial.println("[0] Stop");
}

void loop() {
    if (Serial.available()) {
        char c = Serial.read();
        if (c == '1') { 
            running = true; pwm = 0; 
            Serial.println("Time_ms,PWM_Percent,Raw_ADC"); // CSV Header
        }
        if (c == '0') { running = false; hal.setCoilPower(0); }
    }

    if (running && millis() - lastT > 20) { // 50Hz
        lastT = millis();
        pwm += 0.002f; // Slow Ramp
        if (pwm > 1.0f) { pwm = 1.0f; running = false; }
        
        hal.setCoilPower(pwm);
        Serial.print(millis()); Serial.print(",");
        Serial.print(pwm * 100.0f); Serial.print(",");
        Serial.println(hal.readSensorRaw());
    }
}