/**
 * @file AutoTuning.cpp
 * @brief Ziegler-Nichols Kp Search Lab
 * 
 * PURPOSE:
 * Increase Kp automatically until the system oscillates to find the "Critical Gain" (Ku).
 * Does NOT generate CSV data for physics, but telemetry for the Plotter.
 */

#include <Arduino.h>
#include "Config.h"
#include "Aether_HAL.h"
#include "PID_Controller.h" 
#include "MovingAverage.h"

Aether_HAL hal;
PID_Controller pid(0.0, 0.0, 0.0, Config::Control::LOOP_PERIOD_S);
MovingAverage filter(Config::Control::FILTER_SIZE);

bool tuningActive = false;
float testKp = 0.0f;
unsigned long lastStepTime = 0;
const int STEP_DELAY_MS = 500;       
const float KP_INCREMENT = 0.0001f; 
const float GRAPH_SCALE = 100000.0f; 

void setup() {
    Serial.begin(115200);
    hal.init();
    hal.setCoilPower(0.0f);
    
    delay(1000);
    Serial.println("--- PID AUTO-TUNING LAB ---");
    Serial.println("[a] -> START Auto-Tune (Increases Kp)");
    Serial.println("[s] -> STOP / Safety Off");
}

void loop() {
    int raw = hal.readSensorRaw();
    float filtered = filter.process((float)raw);
    float currentOutput = 0.0f;
    float target = Config::Control::TARGET_ADC;

    // Command Handling
    if (Serial.available() > 0) {
        char cmd = Serial.read();
        while(Serial.available()) Serial.read();

        if (cmd == 's') {
            tuningActive = false;
            testKp = 0.0f;
            pid.setKp(0.0f);
            hal.setCoilPower(0.0f);
            Serial.println("\n>>> STOP. Gains reset.");
        }
        else if (cmd == 'a') {
            tuningActive = true;
            testKp = 0.0f;
            pid.setKp(0.0f); pid.setKi(0.0f); pid.setKd(0.0f);
            Serial.println("\n>>> TUNING STARTED.");
        }
    }

    // Tuning Logic
    if (tuningActive) {
        if (millis() - lastStepTime > STEP_DELAY_MS) {
            lastStepTime = millis();
            testKp += KP_INCREMENT;
            pid.setKp(testKp);
        }

        currentOutput = pid.compute(target, filtered);
        hal.setCoilPower(currentOutput);
        hal.setWarningLed(true); 
    } else {
        hal.setCoilPower(0.0f);
        hal.setWarningLed(false);
    }

    // Telemetria per Serial Plotter
    static unsigned long lastPrint = 0;
    if (millis() - lastPrint > 50) {
        lastPrint = millis();
        Serial.print(">Raw:"); Serial.print((int)filtered);
        Serial.print(",Target:"); Serial.print((int)target);
        Serial.print(",Kp_scaled:"); Serial.print(testKp * GRAPH_SCALE); 
        Serial.print(",PWM_Percent:"); Serial.println(currentOutput * 100.0f);
    }
}