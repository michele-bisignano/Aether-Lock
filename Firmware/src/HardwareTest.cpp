/**
 * @file HardwareTest.cpp
 * @brief Hardware Sanity Check.
 * 
 * USE THIS SKETCH TO:
 * 1. Verify soldering connections.
 * 2. Calibrate the Hall Sensor (Read Zero and Max values).
 * 3. Test MOSFET/Coil heating without PID risks.
 */

#include <Arduino.h>
#include "Config.h"
#include "Aether_HAL.h"

// Use the HAL class you already wrote (so we test it as well)
Aether_HAL hal;

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("--- Aether-Lock HARDWARE TEST ---");
    Serial.println("1. Testing LEDs...");
    Serial.println("2. Reading Sensor...");
    Serial.println("3. Ramping Coil Power...");

    // Initialize hardware using pins from Config.h
    hal.init();
}

void loop() {
    static int pwm_step = 0;
    static int direction = 1;
    
    // --- 1. TEST SENSOR ---
    // Read raw value (0-4095)
    int sensorRaw = hal.readSensorRaw();
    
    // Convert to Volts (for human reading only)
    float voltage = sensorRaw * Config::Hardware::VOLTS_PER_BIT;

    // --- 2. TEST COIL (Slow Ramp) ---
    // Increase and decrease power gradually to test the MOSFET
    // PWM goes from 0.0 to 0.3 (30% max for safety during testing)
    float pwm_duty = (float)pwm_step / 1000.0f; 
    hal.setCoilPower(pwm_duty);

    // Update ramp step (triangle wave 0% -> 30% -> 0%)
    pwm_step += direction;
    if (pwm_step >= 300) direction = -1; // Max 30%
    if (pwm_step <= 0)   direction = 1;

    // --- 3. TEST LED ---
    // Turn on the LED based on the PWM ramp cycle to verify it works
    if (pwm_step > 150) {
        hal.setWarningLed(true);
    } else {
        hal.setWarningLed(false);
    }

    // --- LOGGING ---
    // CSV format for Serial Plotter
    // Legend: RawSensor, Voltage(x1000), PWM_Duty(x4095)
    Serial.print("RawADC:");
    Serial.print(sensorRaw);
    Serial.print(",Volt_mV:");
    Serial.print(voltage * 1000); 
    Serial.print(",PWM_Val:");
    Serial.println(pwm_duty * 4095); // Scalato per vederlo nel grafico

    delay(10); // 100Hz refresh rate
}