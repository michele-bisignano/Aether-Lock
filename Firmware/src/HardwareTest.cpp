/**
 * @file HardwareTest.cpp
 * @brief Max Power Lift Test
 * 
 * SCOPO: Misurare la distanza massima di aggancio (Lift-off) a piena potenza.
 * 
 * ISTRUZIONI:
 * 1. Carica il codice.
 * 2. Apri il Monitor Seriale.
 * 3. Scrivi '1' e premi Invio per attivare la bobina al 100%.
 * 4. Avvicina il magnete dal basso finché non viene catturato.
 * 5. Misura la distanza.
 * 6. Scrivi '0' immediatamente per spegnere e raffreddare.
 * 
 * ATTENZIONE: A 100% la bobina scalda molto rapidamente! 
 * Non tenerla accesa per più di 10-15 secondi consecutivi.
 */

#include <Arduino.h>
#include "Config.h"
#include "Aether_HAL.h"

Aether_HAL hal;
bool isMaxPower = false;

void setup() {
    Serial.begin(115200);
    hal.init();
    
    // Assicuriamoci che parta spento
    hal.setCoilPower(0.0f);
    hal.setWarningLed(false);

    delay(1000);
    Serial.println("--- AETHER-LOCK MAX POWER TEST ---");
    Serial.println("COMMANDS:");
    Serial.println(" [1] -> ATTIVA Bobina al 100% (Warning LED ON)");
    Serial.println(" [0] -> SPEGNI Bobina (Warning LED OFF)");
    Serial.println("----------------------------------");
    Serial.println("Time(ms),State(0/1),RawSensor");
}

void loop() {
    // --- 1. GESTIONE COMANDI SERIALI ---
    if (Serial.available() > 0) {
        char cmd = Serial.read();
        
        // Pulisce il buffer da caratteri extra (es. a capo)
        while(Serial.available()) Serial.read(); 

        if (cmd == '1') {
            isMaxPower = true;
            hal.setCoilPower(1.0f); // 100% Potenza
            hal.setWarningLed(true); // LED Acceso = PERICOLO/CALORE
            Serial.println(">>> COIL ON (100%) - ATTENZIONE AL CALORE!");
        } 
        else if (cmd == '0') {
            isMaxPower = false;
            hal.setCoilPower(0.0f); // 0% Potenza
            hal.setWarningLed(false);
            Serial.println(">>> COIL OFF - Safe");
        }
    }

    // --- 2. TELEMETRIA ---
    // Stampiamo i dati per vedere come reagisce il sensore al campo massimo
    // Nota: A 100% il campo della bobina potrebbe saturare il sensore o spostare lo zero!
    static unsigned long lastPrint = 0;
    if (millis() - lastPrint > 200) {
        lastPrint = millis();
        int raw = hal.readSensorRaw();
        
        Serial.print(millis());
        Serial.print(",");
        Serial.print(isMaxPower ? 1 : 0);
        Serial.print(",");
        Serial.println(raw);
    }
}