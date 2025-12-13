#include "Aether_HAL.h"

void Aether_HAL::init() {
    // 1. Hall Sensor Configuration
    pinMode(Config::Hardware::PIN_HALL, INPUT);
    
    // Set ADC resolution (typically 12-bit for ESP32-S2)
    analogReadResolution(Config::Hardware::ADC_BITS); 

    // 2. Coil PWM Configuration (ESP32 API v2.x)
    // Configure the Timer on the specific logical Channel
    ledcSetup(
        Config::Hardware::PWM_CHANNEL, 
        Config::Hardware::PWM_FREQ, 
        Config::Hardware::PWM_BITS
    );
    
    // Attach the physical Pin to the logical Channel
    ledcAttachPin(Config::Hardware::PIN_PWM, Config::Hardware::PWM_CHANNEL);

    // Initialize with coil OFF for safety
    setCoilPower(0.0f);

    // 3. Status LED Initialization (if available on board)
    #ifdef PIN_LED_BUILTIN
    pinMode(PIN_LED_BUILTIN, OUTPUT);
    #endif
}

int Aether_HAL::readSensorRaw() {
    // Direct ADC read
    return analogRead(Config::Hardware::PIN_HALL);
}

void Aether_HAL::setCoilPower(float duty_cycle) {
    // 1. Safety Saturation (Clamping 0.0 - 1.0)
    // Ensures we never send invalid values to the hardware
    if (duty_cycle > 1.0f) duty_cycle = 1.0f;
    if (duty_cycle < 0.0f) duty_cycle = 0.0f;

    // 2. Conversion using pre-calculated constant from Python script
    // Maps floating point 0.0-1.0 to integer hardware range (e.g., 0-1023)
    int pwm_value = (int)(duty_cycle * Config::Hardware::PWM_MAX_DUTY);
    
    // 3. Hardware Write
    // IMPORTANT: We write to the PWM CHANNEL, not the physical PIN
    ledcWrite(Config::Hardware::PWM_CHANNEL, pwm_value);
}

void Aether_HAL::setLed(bool state) {
    #ifdef PIN_LED_BUILTIN
    digitalWrite(PIN_LED_BUILTIN, state ? HIGH : LOW);
    #endif
}