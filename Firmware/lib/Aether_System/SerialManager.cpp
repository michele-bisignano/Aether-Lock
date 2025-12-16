#include "SerialManager.h"

SerialManager::SerialManager(PID_Controller* pidPtr) {
    this->pid = pidPtr;
    this->last_telemetry_time = 0;
}

void SerialManager::init() {
    Serial.begin(115200);
    // Wait for USB
    while(!Serial) delay(10); 
    
    Serial.println("--- Aether-Lock Serial Interface Ready ---");
    printHelp();
    
    //  CSV Header
    Serial.println("Time(ms),RawADC,PWM(%)");
}

void SerialManager::handleInput() {
    if (Serial.available() > 0) {
        char command = Serial.read();
        float value = Serial.parseFloat();
        
        // Buffer cleanup
        while(Serial.available()) Serial.read(); 

        switch (command) {
            case 'p': pid->setKp(value); Serial.printf(">> SET Kp: %.2f\n", value); break;
            case 'i': pid->setKi(value); Serial.printf(">> SET Ki: %.2f\n", value); break;
            case 'd': pid->setKd(value); Serial.printf(">> SET Kd: %.2f\n", value); break;
            case 'h': printHelp(); break;
            default: break;
        }
    }
}

void SerialManager::streamTelemetry(int raw_sensor, float pwm_duty) {
    if (millis() - last_telemetry_time > TELEMETRY_INTERVAL_MS) {
        last_telemetry_time = millis();

        Serial.print(millis());
        Serial.print(",");
        Serial.print(raw_sensor); 
        Serial.print(",");
        Serial.println(pwm_duty * 4095.0f); 
    }
}

void SerialManager::printHelp() {
    Serial.println("--- COMMANDS ---");
    Serial.println("p<val> : Set Kp (e.g., p1000)");
    Serial.println("i<val> : Set Ki (e.g., i50)");
    Serial.println("d<val> : Set Kd (e.g., d25)");
    Serial.println("----------------");
}