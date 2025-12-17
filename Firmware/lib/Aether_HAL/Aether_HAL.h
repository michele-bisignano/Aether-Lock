#ifndef AETHER_HAL_H
#define AETHER_HAL_H

#include <Arduino.h>
#include "Config.h"

/**
 * @brief Hardware Abstraction Layer for Aether-Lock.
 * 
 * This class encapsulates all direct interactions with the ESP32 hardware
 * (PWM, ADC, GPIO). It decouples the control logic from the specific
 * microcontroller implementation.
 */
class Aether_HAL {
public:
    /**
     * @brief Configures Pins, PWM frequency/resolution and ADC settings.
     * Call this once in setup().
     */
    void init();

    /**
     * @brief Reads the raw value from the Hall Sensor.
     * @return int Raw ADC value (0-4095 for 12-bit).
     */
    int readSensorRaw();

    /**
     * @brief Sets the power of the Electromagnet.
     * 
     * @param duty_cycle A float between 0.0 (OFF) and 1.0 (MAX Power).
     *                   Values outside this range are clamped safely.
     */
    void setCoilPower(float duty_cycle);

    /**
     * @brief Toggles the onboard Status LED (useful for debugging loop timing).
     * @param state true = ON, false = OFF
     */
    void setLed(bool state);

    /**
     * @brief Toggles the onboard Warning LED.
     * @param state true = ON, false = OFF
     */
    void setWarningLed(bool state);
};

#endif