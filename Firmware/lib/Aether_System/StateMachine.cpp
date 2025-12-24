#include "StateMachine.h"

// --- CONSTRUCTOR ---
StateMachine::StateMachine(Aether_HAL* halPtr, PID_Controller* pidPtr, MovingAverage* filterPtr) {
    this->hal = halPtr;
    this->pid = pidPtr;
    this->filter = filterPtr;
    
    // Initialize state
    this->currentState = STATE_IDLE;
    this->_debug_distance_adc = 0.0f;
    this->_debug_pwm_duty = 0.0f;
}

// --- INITIALIZATION ---
void StateMachine::init() {
    currentState = STATE_IDLE;
    hal->setCoilPower(0.0f);
    hal->setLed(false);
    hal->setWarningLed(false);
    thermalProtection.reset();
}

// --- MAIN LOOP (Called by ISR) ---
void StateMachine::update() {
    // 1. HARDWARE READ & FILTERING
    int raw = hal->readSensorRaw();
    float filtered = filter->process((float)raw);

    // 2. FEEDFORWARD COMPENSATION (Coil Noise Cancellation)
    // The coil magnetic field affects the sensor reading. We must subtract/add 
    // this "self-sensing" noise to get the true position of the magnet.
    // We use the PWM duty from the *previous* cycle as an estimate of current field strength.
    float coil_noise_correction = _debug_pwm_duty * Config::Hardware::HALL_COIL_COUPLING;
    
    // Corrected value: The "True" position of the angel
    float current_val = filtered + coil_noise_correction;

    // Update telemetry variable (for SerialManager)
    _debug_distance_adc = current_val;

    // 3. THERMAL PROTECTION
    // Update the thermal model with current power usage
    bool temp_safe = thermalProtection.update(_debug_pwm_duty);
    float current_temp = thermalProtection.getTemperature();

    // Visual Warning if temp is getting high
    if (current_temp > Config::Control::Thermal::WARNING_TEMP_C) {
        hal->setWarningLed(true);
    } else {
        hal->setWarningLed(false);
    }

    // Emergency Cut-off if temp is critical
    if (!temp_safe && currentState != STATE_ERROR) {
        currentState = STATE_ERROR;
    }

    // 4. STATE MACHINE LOGIC
    switch (currentState) {
        case STATE_IDLE:
            // System is ready but coil is off
            hal->setCoilPower(0.0f);
            hal->setLed(false);
            _debug_pwm_duty = 0.0f;

            // Hand-over detection: If the angel is brought close to the target manually
            // Condition: Current Value > (Target - Threshold)
            if (current_val > (Config::Control::TARGET_ADC - Config::Control::IDLE_THRESHOLD)) {
                pid->reset(); // Reset Integral term to avoid jumps
                currentState = STATE_LEVITATING;
            }
            break;

        case STATE_LEVITATING:
            {
                // Execute Control Loop
                float output = pid->compute(Config::Control::TARGET_ADC, current_val);
                
                // Actuate Hardware
                hal->setCoilPower(output);
                hal->setLed(true); // Status LED ON indicates active control
                
                // Save output for next cycle (Feedforward) and Telemetry
                _debug_pwm_duty = output;

                // Safety Check: Is the angel still within controllable range?
                if (isAngelInRange(current_val)) {
                    lastTimeInRange = millis();
                }

                // If angel is out of range for too long -> Safety Shutdown
                if ((millis() - lastTimeInRange) > Config::Control::FALL_TIMEOUT_MS) {
                    currentState = STATE_ERROR;
                }
            }
            break;

        case STATE_ERROR:
            // Safety Lockout State (Overheat or Drop)
            hal->setCoilPower(0.0f);
            _debug_pwm_duty = 0.0f;
            
            // Blink LED to indicate error
            if ((millis() / 200) % 2 == 0) hal->setLed(true); else hal->setLed(false);

            // Auto-Reset Logic:
            // If the magnet is removed (value drops significantly below target),
            // reset to IDLE state to allow a new attempt.
            if (current_val < (Config::Control::TARGET_ADC - 200)) { 
                currentState = STATE_IDLE;
            }
            break;
    }
}

// --- HELPER METHODS ---

bool StateMachine::isAngelInRange(float raw_value) {
    // Check if the value is within the physical safety bounds defined in JSON
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

float StateMachine::getTemperature() const {
    return thermalProtection.getTemperature();
}