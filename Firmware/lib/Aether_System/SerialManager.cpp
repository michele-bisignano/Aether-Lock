#include "SerialManager.h"

SerialManager::SerialManager(PID_Controller* pidPtr, StateMachine* fsmPtr) {
    this->pid = pidPtr;
    this->fsm = fsmPtr;
    this->last_telemetry_time = 0;
}

void SerialManager::init() {
    Serial.begin(115200);
    Serial.println("--- Aether-Lock Ready ---");
    printHelp();
    Serial.println("Time,RawADC,PWM");
}

void SerialManager::handleInput() {
    if (Serial.available() > 0) {
        char command = Serial.read();
        float value = Serial.parseFloat();
        while(Serial.available()) Serial.read(); // Flush

        switch (command) {
            case 'p': pid->setKp(value); Serial.printf(">> Kp: %.4f\n", value); break;
            case 'i': pid->setKi(value); Serial.printf(">> Ki: %.4f\n", value); break;
            case 'd': pid->setKd(value); Serial.printf(">> Kd: %.4f\n", value); break;
            case 'h': printHelp(); break;
        }
    }
}

void SerialManager::streamTelemetry() {
    if (millis() - last_telemetry_time > Config::Control::TELEMETRY_MS) {
        last_telemetry_time = millis();
        
        // Get data directly from FSM getters
        float dist = fsm->getRawDistance();
        float pwm = fsm->getPWMDuty();

        // CSV Format for Plotter
        Serial.print(millis());
        Serial.print(",");
        Serial.print((int)dist); 
        Serial.print(",");
        Serial.println((int)(pwm * 4095)); // Scaled for graph visibility
    }
}

void SerialManager::printHelp() {
    Serial.println("Cmds: p<val>, i<val>, d<val>");
}