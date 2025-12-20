/**
 * @file HardwareTest.cpp
 * @brief Coil & Sensor Relationship Test
 * 
 * This script is used to verify:
 * 1. The sensor's response to the generated magnetic field.
 * 2. The proper functioning of the MOSFET and the Coil.
 * 3. The relationship between PWM and heating (touch the coil with caution).
 */

#include <Arduino.h>
#include "Config.h"
#include "Aether_HAL.h"

// Hardware Abstraction Layer instance
Aether_HAL hal;

// Ramp Parameters
float current_pwm = 0.0f; // Current duty cycle (0.0 - 1.0)
float step_size = 0.01f;  // 1% increment per cycle
int direction = 1;        // 1 = Increasing, -1 = Decreasing

void setup() {
    // 1. Serial Initialization
    Serial.begin(115200);
    delay(1000); // Safety wait

    Serial.println("--- AETHER-LOCK COIL TEST ---");
    Serial.println("Warning: Coil might get hot at 100% duty cycle.");
    
    // CSV Table Header
    Serial.println("Raw_Sensor_Value,PWM_Percent");

    // 2. Hardware Initialization
    hal.init();
}

void loop() {
    // --- 1. ACTION: Set Coil Power ---
    hal.setCoilPower(current_pwm);

    // --- 2. ACTION: LED Feedback ---
    // ON when the field increases (charging), OFF when it decreases (discharging)
    if (direction > 0) {
        hal.setWarningLed(true);
    } else {
        hal.setWarningLed(false);
    }

    // --- 3. READING: Hall Sensor ---
    int rawSensor = hal.readSensorRaw();

    // --- 4. OUTPUT: Data Table ---
    // Print: Raw Sensor Value, Coil Power Percentage
    Serial.print(rawSensor);
    Serial.print(",");
    Serial.println(current_pwm * 100.0f);

    // --- 5. RAMP LOGIC ---
    current_pwm += (step_size * direction);

    // Direction reversal management (Bounce)
    if (current_pwm >= 1.0f) {
        current_pwm = 1.0f;
        direction = -1; // Start decreasing
        // Brief pause at maximum to check if the sensor is stable
        delay(200); 
    } else if (current_pwm <= 0.0f) {
        current_pwm = 0.0f;
        direction = 1; // Start increasing
        // Brief pause at zero
        delay(200);
    }

    // Test speed calculation:
    // 50ms delay * 100 steps = 5 seconds to ramp from 0 to 100%
    delay(50); 
}