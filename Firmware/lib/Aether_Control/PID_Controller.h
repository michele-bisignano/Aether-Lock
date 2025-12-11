#ifndef PID_CONTROLLER_H
#define PID_CONTROLLER_H

/**
 * @brief Discrete PID Controller Implementation.
 * 
 * Uses Backward Euler discretization for the Integral and Derivative terms
 * to ensure stability even at varying sampling frequencies.
 * Designed for the ESP32-S2 floating point unit (FPU).
 */
class PID_Controller {
private:
    // --- Tuning Parameters ---
    float Kp; // Proportional gain
    float Ki; // Integral gain
    float Kd; // Derivative gain
    float Ts; // Sampling time [seconds]

    // --- Memory Variables (State) ---
    float previous_error; // Error at step (k-1) for derivative calculation
    float integral;       // Accumulated sum for integral term

    float max_integral_value;

public:
    /**
     * @brief Constructor. Initializes gains and resets internal state.
     * @param kp Proportional Gain
     * @param ki Integral Gain
     * @param kd Derivative Gain
     * @param ts Sampling Time in seconds (e.g., 0.0002 for 5kHz)
     */
    PID_Controller(float kp, float ki, float kd, float ts);

    /**
     * @brief Calculates the control output for the current time step (k).
     * 
     * Implements the positional PID algorithm: u(k) = P + I + D
     * 
     * @param setpoint The desired target value (e.g., Target Distance)
     * @param measured_value The actual sensor reading (e.g., Current Distance)
     * @return float The control signal (e.g., PWM duty cycle 0.0-1.0 or raw correction)
     */
    float compute(float setpoint, float measured_value);

    /**
     * @brief Resets the internal memory (Integral term and Previous Error).
     * 
     * Call this when the system is turned on or after an emergency stop
     * to prevent "integral windup" jumps.
     */
    void reset();
};

#endif