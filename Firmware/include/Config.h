/**
 * @file Config.h
 * @brief AUTO-GENERATED FILE from project_config.json.
 */

#pragma once

namespace Config {

    namespace Plant {
        constexpr float MASS_KG = 0.007;
        constexpr float GRAVITY = 9.81;
        constexpr float COIL_RESISTANCE = 28.57;
    }

    namespace Control {
        constexpr float LOOP_FREQ_HZ = 5000;
        constexpr float LOOP_PERIOD_S = 1.0f / 5000;
        constexpr int FILTER_SIZE = 10;
        constexpr unsigned long TELEMETRY_MS = 50;

        // Raw ADC Setpoints
        constexpr float TARGET_ADC = 2200;
        constexpr float IDLE_THRESHOLD = 100;
        constexpr float SAFETY_MIN_ADC = 1000;
        constexpr float SAFETY_MAX_ADC = 4000;
        
        // Safety Constants
        constexpr unsigned long FALL_TIMEOUT_MS = 1000;
        constexpr float POS_TOLERANCE_M = 0.01;   
             
        
        namespace PID {
            constexpr float KP = 101.85;
            constexpr float KI = 509.24;
            constexpr float KD = 2.72;
        }

        namespace Thermal {
            constexpr float MAX_TEMP_C = 75.0;
            constexpr float WARNING_TEMP_C = 60.0;
            constexpr float AMBIENT_TEMP_C = 25.0;
            constexpr float SUPPLY_VOLTAGE = 12.0;
            constexpr float R_THERMAL = 15.0;
            
            // Fattore pre-calcolato: Gradi guadagnati per Watt in un ciclo
            constexpr float TEMP_RISE_PER_WATT = 1e-05;
        }
    }

    namespace Hardware {
        constexpr int PIN_HALL = 4;
        constexpr int PIN_PWM = 25;
        constexpr int PIN_WARN_LED = 15;
        
        // ADC & Sensor
        constexpr int ADC_BITS = 12;
        constexpr float ADC_REF_V = 3.3;
        constexpr float VOLTS_PER_BIT = 0.0008058608058608059;

        constexpr float HALL_ZERO_V = 1.65; 
        constexpr float HALL_SENSITIVITY = 0.157301; 
        
        // PWM Settings
        constexpr int PWM_FREQ = 20000;
        constexpr int PWM_BITS = 10;
        constexpr int PWM_CHANNEL = 0;
        constexpr int PWM_MAX_DUTY = 1023; 
    }
}
