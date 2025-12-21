/**
 * @file HardwareTest.cpp
 * @brief Equilibrium Current Discovery (Drop-off Method)
 * 
 * PROCEDURE:
 * 1. Send '1' to start: Coil goes to 100%. Place the angel against the spacer.
 * 2. The system automatically decreases power by 1% every second.
 * 3. Watch carefully. The moment the angel drops, Send '0'.
 * 4. The Serial Monitor will show the exact PWM value where gravity won.
 */

#include <Arduino.h>
#include "Config.h"
#include "Aether_HAL.h"

Aether_HAL hal;

bool testRunning = false;
float currentPwm = 0.0f;
unsigned long lastStepTime = 0;

void setup() {
    Serial.begin(115200);
    hal.init();
    
    // Safety init
    hal.setCoilPower(0.0f);
    hal.setWarningLed(false);

    delay(1000);
    Serial.println("--- DROP-OFF TEST READY ---");
    Serial.println("1. Put spacer (e.g., 2cm) under the coil.");
    Serial.println("2. Type '1' + Enter to START (Coil Max Power).");
    Serial.println("3. Attach the angel.");
    Serial.println("4. Wait... Power decreases 1% per second.");
    Serial.println("5. When it drops, Type '0' + Enter IMMEDIATELY.");
}

void loop() {
    // --- GESTIONE INPUT SERIALE ---
    if (Serial.available() > 0) {
        char cmd = Serial.read();
        // Svuota buffer
        while(Serial.available()) Serial.read();

        if (cmd == '1') {
            // AVVIO TEST
            testRunning = true;
            currentPwm = 0.54f; // Parte dal 100%
            lastStepTime = millis();
            hal.setCoilPower(currentPwm);
            hal.setWarningLed(true);
            Serial.println(">>> STARTED: 100% Power. Attach Angel NOW.");
        }
        else if (cmd == '0') {
            // STOP TEST (Caduta)
            testRunning = false;
            hal.setCoilPower(0.0f);
            hal.setWarningLed(false);
            
            Serial.println("\n--- TEST RESULT ---");
            Serial.print("DROP DETECTED AT PWM: ");
            Serial.print(currentPwm * 100.0f, 3);
            Serial.println(" %");
            Serial.println("This is your Equilibrium Limit (Lower Bound).");
            Serial.println("-------------------");
        }
    }

    // --- LOGICA AUTOMATICA (Decremento) ---
    if (testRunning) {
        // Ogni 1000ms (1 secondo) scende dell'1%
        if (millis() - lastStepTime > 1000) {
            lastStepTime = millis();
            
            currentPwm -= 0.0001f; // Scende di 0.001 (0.1%)
            
            // Sicurezza: non andare sotto zero
            if (currentPwm < 0.0f) {
                currentPwm = 0.0f;
                testRunning = false;
                Serial.println("Reached 0%. Test ended.");
            }

            // Applica nuova potenza
            hal.setCoilPower(currentPwm);
            
            // Feedback visivo (stampa ogni step)
            Serial.print("Current PWM: ");
            Serial.print(currentPwm * 100.0f, 3);
            Serial.println("%");
        }
    }
}