#include "StateMachine.h"

StateMachine::StateMachine(Aether_HAL* halPtr, PID_Controller* pidPtr, MovingAverage* filterPtr) {
    this->hal = halPtr;
    this->pid = pidPtr;
    this->filter = filterPtr;
    this->currentState = STATE_IDLE;
    this->_debug_distance_adc = 0;
    this->_debug_pwm_duty = 0;
}

void StateMachine::init() {
    currentState = STATE_IDLE;
    hal->setCoilPower(0.0f);
    hal->setLed(false);
}

void StateMachine::update() {
    // 1. Read & Filter
    int raw = hal->readSensorRaw();
    float current_val = filter->process((float)raw);
    
    // Update Telemetry Data
    _debug_distance_adc = current_val;

    // 2. State Logic
    switch (currentState) {
        case STATE_IDLE:
            hal->setCoilPower(0.0f);
            hal->setLed(false);
            _debug_pwm_duty = 0.0f;

            // Activation condition: Angel is close enough
            // Target - Threshold (e.g., 2200 - 100 = 2100)
            if (current_val > (Config::Control::TARGET_ADC - Config::Control::IDLE_THRESHOLD)) {
                pid->reset();
                currentState = STATE_LEVITATING;
            }
            break;

        case STATE_LEVITATING:
            {
                // Compute PID based on Raw ADC values
                float output = pid->compute(Config::Control::TARGET_ADC, current_val);
                
                hal->setCoilPower(output);
                hal->setLed(true);
                _debug_pwm_duty = output;

                // Safety Check
                if (isAngelInRange(current_val)) {
                    lastTimeInRange = millis();
                }

                if ((millis() - lastTimeInRange) > Config::Control::FALL_TIMEOUT_MS) {
                    currentState = STATE_ERROR;
                }
            }
            break;

        case STATE_ERROR:
            hal->setCoilPower(0.0f);
            _debug_pwm_duty = 0.0f;
            
            // Blink LED to indicate error
            if ((millis() / 200) % 2 == 0) hal->setLed(true); else hal->setLed(false);

            // Auto-reset if magnet is removed (value goes low)
            if (current_val < (Config::Control::TARGET_ADC - 200)) { // Hysteresis
                currentState = STATE_IDLE;
            }
            break;
    }
}

bool StateMachine::isAngelInRange(float raw_value) {
    // Check if value is within safe bounds defined in JSON
    if (raw_value > Config::Control::SAFETY_MIN_ADC && 
        raw_value < Config::Control::SAFETY_MAX_ADC) {
        return true;
    }
    return false;
}

// --- TELEMETRY GETTERS ---
float StateMachine::getRawDistance() const {
    return _debug_distance_adc;
}

float StateMachine::getPWMDuty() const {
    return _debug_pwm_duty;
}