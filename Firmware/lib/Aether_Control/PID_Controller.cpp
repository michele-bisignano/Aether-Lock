#include "PID_Controller.h"

// --- CONSTRUCTOR ---
PID_Controller::PID_Controller(float kp, float ki, float kd, float ts)
    : Kp(kp), Ki(ki), Kd(kd), Ts(ts), previous_error(0.0f), integral(0.0f)
{
    // Pre-calculate the maximum allowed integral value to prevent windup.
    // We want the Integral term (Ki * integral) to contribute at most 100% (1.0) to the output.
    if (Ki > 1e-6f) { // Check to avoid division by zero (using float epsilon)
        max_integral_value = 1.0f / Ki;
    } else { 
        max_integral_value = 0.0f; // If Ki is 0, integral action is disabled
    }
}

// --- MAIN COMPUTATION LOOP ---
float PID_Controller::compute(float setpoint, float measured_value)
{
    // 1. Calculate Error
    // Error is positive if we are below the setpoint (need to go up)
    float error = setpoint - measured_value;

    // 2. Proportional Term
    // Provides immediate reaction to the error magnitude
    float P = Kp * error;

    // 3. Integral Term (Backward Euler Discretization)
    // Accumulates error over time to eliminate steady-state error
    integral += error * Ts;

    // Anti-Windup (Clamping)
    // Prevents the integrator from building up beyond the actuator's physical limits.
    // Critical for MagLev to avoid "sticking" at max power.
    if (integral > max_integral_value) {
        integral = max_integral_value;
    } else if (integral < -max_integral_value) {
        integral = -max_integral_value;
    }

    // Calculate I term AFTER clamping to ensure the output is bounded in this cycle
    float I = Ki * integral;

    // 4. Derivative Term (Backward Finite Difference)
    // Provides damping by reacting to the rate of change of the error.
    // derivative = (current_error - previous_error) / delta_time
    float derivative = (error - previous_error) / Ts;
    float D = Kd * derivative;

    // 5. Update Memory
    // Store current error for the next derivative calculation
    previous_error = error;

    // 6. Compute Total Output
    float output = P + I + D;

    // 7. Output Saturation (Actuator Limits)
    // The PWM duty cycle must be strictly between 0.0 (0%) and 1.0 (100%).
    if (output > 1.0f) {
        output = 1.0f;
    } else if (output < 0.0f) {
        output = 0.0f;
    }

    return output;
}

// --- RESET UTILITY ---
void PID_Controller::reset() {
    previous_error = 0.0f;
    integral = 0.0f;
}

// --- DYNAMIC TUNING SETTERS ---

void PID_Controller::setKp(float kp) {
    this->Kp = kp;
}

void PID_Controller::setKd(float kd) {
    this->Kd = kd;
}

void PID_Controller::setKi(float ki) {
    this->Ki = ki;

    if (Ki > 1e-6f) {
        max_integral_value = 1.0f / Ki;
    } else {
        max_integral_value = 0.0f;
    }
}