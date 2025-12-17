/**
 * @file ThermalGuard.cpp
 * @brief Implementation of the ThermalGuard logic.
 */

#include "ThermalGuard.h"
#include "Config.h" // Requires the generated configuration file

// Constructor
ThermalGuard::ThermalGuard() {
    reset();
}

void ThermalGuard::reset() {
    // Assume coil starts at ambient temperature defined in Config
    _current_temp = Config::Control::Thermal::AMBIENT_TEMP_C;
}

float ThermalGuard::getTemperature() const {
    return _current_temp;
}

bool ThermalGuard::update(float pwm_duty) {
    // --- 1. Input Power Calculation (Joule Heating) ---
    // We estimate the effective DC voltage seen by the coil: V_eff = V_supply * Duty
    // P = I_avg^2 * R
    float effective_voltage = Config::Control::Thermal::SUPPLY_VOLTAGE * pwm_duty;
    
    float power_in = (effective_voltage * effective_voltage) / Config::Plant::COIL_RESISTANCE;

    // --- 2. Dissipation Calculation (Cooling) ---
    // Newton's Law of Cooling: Heat loss is proportional to temperature difference.
    // P_out = (T_coil - T_ambient) / R_thermal
    float delta_temp = _current_temp - Config::Control::Thermal::AMBIENT_TEMP_C;
    float power_out = delta_temp / Config::Control::Thermal::R_THERMAL;

    // --- 3. Energy Integration (Euler Method) ---
    // Net Power = Power In - Power Out
    // Temperature Change = Net Power * (dt / Heat_Capacity)
    float net_power = power_in - power_out;    
    _current_temp += net_power * Config::Control::Thermal::TEMP_RISE_PER_WATT;

    // --- 4. Safety Check ---
    // Return false immediately if we cook the coil
    if (_current_temp >= Config::Control::Thermal::MAX_TEMP_C) {
        return false; // ALARM: Overheating
    }

    return true; // OK: Temperature nominal
}