#include <Arduino.h>
#include "Config.h" 

void setup() {
    Serial.begin(115200);
    
    // float k = Config::Control::PID::KP; 
}

void loop() {
    Serial.println("Aether-Lock System Alive!");
    delay(1000);
}