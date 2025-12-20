/**
 * @file HardwareTest.cpp
 * @brief Hardware Sanity Check for ESP32 WROOM.
 * 
 * FEATURES:
 * 1. Warning LED Blinking (Pin 15) every 2 seconds (Heartbeat).
 * 2. Hall Sensor Data Printing (2 times per second).
 * 3. Coil Ramp Test (Background) - Ramps up to 30% and down to test the MOSFET.
 */

#include <Arduino.h>
#include "Config.h"
#include "Aether_HAL.h"

// Instantiate HAL
Aether_HAL hal;

// Timing variables (Non-blocking)
unsigned long lastLedTime = 0;
unsigned long lastPrintTime = 0;
unsigned long lastRampTime = 0;

// State variables
bool ledState = false;
int pwm_step = 0;
int direction = 1;

void setup() {
    // 1. Initialize Serial
    Serial.begin(115200);
    // Short delay to allow the USB chip to wake up
    delay(1000); 

    Serial.println("--- AETHER-LOCK HARDWARE TEST (WROOM) ---");
    Serial.println("1. LED: Blinking (2s period)");
    Serial.println("2. SENSOR: Printing (2 Hz)");
    Serial.println("3. COIL: Safe Ramp (Max 30%)");

    // 2. Initialize Hardware (Pins, PWM, ADC)
    hal.init();
    
    // Ensure everything starts off
    hal.setCoilPower(0.0f);
    hal.setWarningLed(false);
}

void loop() {
    unsigned long currentMillis = millis();

    // --- TASK 1: LED BLINK (Toggle state every 1000ms -> 2 second cycle) ---
    if (currentMillis - lastLedTime >= 1000) {
        lastLedTime = currentMillis;
        
        ledState = !ledState; // Toggle state
        hal.setWarningLed(ledState);
        
        // LED Debug message (Optional, uncomment if you want to see it)
        // if(ledState) Serial.println("[LED] ON"); else Serial.println("[LED] OFF");
    }

    // --- TASK 2: PRINT SENSOR DATA (Every 500ms -> 2 Hz) ---
    if (currentMillis - lastPrintTime >= 500) {
        lastPrintTime = currentMillis;

        // Read raw value (0-4095)
        int raw = hal.readSensorRaw();
        
        // Calculate voltage (for human reference only)
        float volts = raw * Config::Hardware::VOLTS_PER_BIT;

        // Formatted print
        Serial.print(">> SENSOR RAW: ");
        Serial.print(raw);
        Serial.print("  |  VOLTAGE: ");
        Serial.print(volts, 3);
        Serial.println(" V");
    }

    // --- TASK 3: COIL TEST (Very fast - every 10ms for smoothness) ---
    // This verifies that the MOSFET opens and the coil pulls slightly
    if (currentMillis - lastRampTime >= 10) {
        lastRampTime = currentMillis;

        // Calculate duty cycle (0.0 -> 0.3)
        float duty = (float)pwm_step / 1000.0f;
        hal.setCoilPower(duty);

        // Update ramp step
        pwm_step += direction;
        if (pwm_step >= 300) direction = -1; // Max 30% for thermal safety
        if (pwm_step <= 0)   direction = 1;
    }
}