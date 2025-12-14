/**
 * @file Config.h
 * @brief AUTO-GENERATED FILE from project_config.json.
 */

#pragma once

namespace Config {

    namespace Plant {
        constexpr float MASS_KG = 0.007;
        constexpr float GRAVITY = 9.81;
        constexpr float EQUILIBRIUM_DIST_M = 0.03;
        constexpr float COIL_RESISTANCE = 36.0;
    }

    namespace Control {
        constexpr float LOOP_FREQ_HZ = 5000;
        constexpr float LOOP_PERIOD_S = 1.0f / 5000;
        constexpr float TARGET_DIST_M = Plant::EQUILIBRIUM_DIST_M;
        constexpr int FILTER_SIZE = 10;

        constexpr unsigned long FALL_TIMEOUT_MS = 1000;
        constexpr float POS_TOLERANCE_M = 0.01;        
        
        namespace PID {
            constexpr float KP = 101.85;
            constexpr float KI = 509.24;
            constexpr float KD = 2.72;
        }
    }

    namespace Hardware {
        constexpr int PIN_HALL = 34;
        constexpr int PIN_PWM = 25;
        
        constexpr int ADC_BITS = 12;
        constexpr float ADC_REF_V = 3.3;
        constexpr float VOLTS_PER_BIT = 0.0008058608058608059;
        
        constexpr float HALL_ZERO_V = 1.65; 
        constexpr float HALL_SENSITIVITY = 0.157301; 
        
        constexpr int PWM_FREQ = 20000;
        constexpr int PWM_BITS = 10;
        constexpr int PWM_CHANNEL = 0;
        constexpr int PWM_MAX_DUTY = 1023;
    }
}
