#include "StateMachine.h"

// --- CONSTRUCTOR ---
StateMachine::StateMachine(Aether_HAL* halPtr, PID_Controller* pidPtr, MovingAverage* filterPtr) {
    this->hal = halPtr;
    this->pid = pidPtr;
    this->filter = filterPtr;
    this->currentState = STATE_IDLE;
}

// --- INITIALIZATION ---
void StateMachine::init() {
    // Safe initial state
    currentState = STATE_IDLE;
    hal->setCoilPower(0.0f);
    hal->setLed(false);
}

// --- MAIN LOOP ---
void StateMachine::update() {
    // 1. Sensor Reading (Common to all states)
    int raw = hal->readSensorRaw();
    float filtered = filter->process((float)raw);
    
    // Raw conversion to Meters (same logic as previous main)
    // TODO: Calibrate these values with real hardware!
    float voltage = filtered * Config::Hardware::VOLTS_PER_BIT;;
    
    float distance = Config::Control::TARGET_DIST_M + 
                    (voltage - Config::Hardware::HALL_ZERO_V) * Config::Hardware::HALL_SENSITIVITY;

    // State Machine Switch
    switch (currentState) {
        case STATE_IDLE:
            // Coil off
            hal->setCoilPower(0.0f);
            hal->setLed(false);

            // Transition: If the angel is detected near the setpoint, activate control
            // ("Hand-over" feature: you bring it close by hand, and the system takes over)
            if (isAngelInRange(distance)) {
                pid->reset(); // Reset the integral term before starting
                currentState = STATE_LEVITATING;
            }
            break;

        case STATE_LEVITATING:
            {
                // Execute PID
                float output = pid->compute(Config::Control::TARGET_DIST_M, distance);
                hal->setCoilPower(output);
                hal->setLed(true); // LED on = System active

                // Safety Check: Has the angel fallen?
                if (isAngelInRange(distance)) {
                    lastTimeInRange = millis(); // Reset timer if in range
                }

                // If the angel is out of position for too long -> EMERGENCY
                if ((millis() - lastTimeInRange) > Config::Control::FALL_TIMEOUT_MS) {
                    currentState = STATE_ERROR;
                }
            }
            break;

        case STATE_ERROR:
            // Thermal protection: Shut everything down!
            hal->setCoilPower(0.0f);
            
            // Blinking LED (Simple error signaling using millis)
            // Fast blink (approx every 200ms cycle)
            if ((millis() / 200) % 2 == 0) hal->setLed(true);
            else hal->setLed(false);

            // To exit the error state, the user must remove the angel or reset.
            // Here we could implement an auto-reset if the angel is removed (voltage returns to center),
            // or require a manual reset via serial/button.
            break;
    }
}

// --- HELPER: CHECK POSITION ---
bool StateMachine::isAngelInRange(float distance) {
    // Define a "safe zone" around the target
    float target = Config::Control::TARGET_DIST_M;
    float tolerance = Config::Control::POS_TOLERANCE_M;

    if (distance > (target - tolerance) && distance < (target + tolerance)) {
        return true;
    }
    return false;
}

// --- HELPER: MANUAL RESET ---
void StateMachine::resetError() {
    currentState = STATE_IDLE;
}