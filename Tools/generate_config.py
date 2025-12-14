"""
@file generate_config.py
@brief Configuration Propagation Tool (SSOT Implementation).
@details Bridges project_config.json to C++, MATLAB, and Markdown docs.
"""

import json
import os
import sys

if sys.platform.startswith('win'):
    sys.stdout.reconfigure(encoding='utf-8')

# --- PATH CONFIGURATION ---   
SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
PROJECT_ROOT = os.path.dirname(SCRIPT_DIR)

JSON_PATH = os.path.join(PROJECT_ROOT, "project_config.json") 
CPP_PATH = os.path.join(PROJECT_ROOT, "Firmware", "include", "Config.h")
MATLAB_PATH = os.path.join(PROJECT_ROOT, "Simulation", "MATLAB", "load_params.m")
DOCS_PATH = os.path.join(PROJECT_ROOT, "Docs", "Theory", "System_Parameters.md")

def load_config():
    print(f"🔄 Reading configuration from: {JSON_PATH}")
    with open(JSON_PATH, 'r', encoding='utf-8') as f:
        return json.load(f)

def generate_cpp_header(data):
    print(f"🔨 Generating C++ Header: {CPP_PATH}")
    
    phy = data["physical_plant"]
    ctrl = data["control_system"]
    hw = data["hardware_mapping"]
    
    sensor = hw['sensor_specs']
    adc = hw['adc_settings']

    # --- PRE-COMPUTATIONS ---
    pwm_bits = hw['pwm_settings']['resolution_bits']
    pwm_max_val = (1 << pwm_bits) - 1

    adc_bits = adc['resolution_bits']
    adc_max_val = (1 << adc_bits) - 1
    adc_ref_v = adc['reference_voltage_v']

    volts_per_bit = adc_ref_v / adc_max_val

    content = f"""/**
 * @file Config.h
 * @brief AUTO-GENERATED FILE from project_config.json.
 */

#pragma once

namespace Config {{

    namespace Plant {{
        constexpr float MASS_KG = {phy['mass_kg']};
        constexpr float GRAVITY = {phy['gravity_mss']};
        constexpr float EQUILIBRIUM_DIST_M = {phy['equilibrium_distance_m']};
        constexpr float COIL_RESISTANCE = {phy['coil_resistance_ohm']};
    }}

    namespace Control {{
        constexpr float LOOP_FREQ_HZ = {ctrl['loop_frequency_hz']};
        constexpr float LOOP_PERIOD_S = 1.0f / {ctrl['loop_frequency_hz']};
        constexpr float TARGET_DIST_M = Plant::EQUILIBRIUM_DIST_M;
        constexpr int FILTER_SIZE = {ctrl['filter_window_size']};

        constexpr unsigned long FALL_TIMEOUT_MS = {ctrl['safety']['fall_timeout_ms']};
        constexpr float POS_TOLERANCE_M = {ctrl['safety']['position_tolerance_m']};        
        
        namespace PID {{
            constexpr float KP = {ctrl['pid_gains']['kp']};
            constexpr float KI = {ctrl['pid_gains']['ki']};
            constexpr float KD = {ctrl['pid_gains']['kd']};
        }}
    }}

    namespace Hardware {{
        constexpr int PIN_HALL = {hw['pins']['hall_sensor_pin']};
        constexpr int PIN_PWM = {hw['pins']['pwm_coil_pin']};
        
        constexpr int ADC_BITS = {adc_bits};
        constexpr float ADC_REF_V = {adc_ref_v};
        constexpr float VOLTS_PER_BIT = {volts_per_bit};
        
        constexpr float HALL_ZERO_V = {sensor['quiescent_output_v']}; 
        constexpr float HALL_SENSITIVITY = {sensor['sensitivity_m_per_v']}; 
        
        constexpr int PWM_FREQ = {hw['pwm_settings']['frequency_hz']};
        constexpr int PWM_BITS = {hw['pwm_settings']['resolution_bits']};
        constexpr int PWM_CHANNEL = {hw['pwm_settings']['pwm_channel']};
        constexpr int PWM_MAX_DUTY = {pwm_max_val};
    }}
}}
"""
    os.makedirs(os.path.dirname(CPP_PATH), exist_ok=True)
    with open(CPP_PATH, 'w', encoding='utf-8') as f:
        f.write(content)

def generate_matlab_script(data):
    print(f"🔨 Generating MATLAB Script: {MATLAB_PATH}")
    
    phy = data["physical_plant"]
    geo = phy["coil_geometry"]
    ctrl = data["control_system"]
    hw = data["hardware_mapping"]
    sensor = hw['sensor_specs']
    adc = hw['adc_settings']
    pwm = hw['pwm_settings']
    
    adc_max = (1 << adc['resolution_bits']) - 1
    pwm_max = (1 << pwm['resolution_bits']) - 1

    content = f"""% AUTO-GENERATED FILE from project_config.json
% Run this script to load parameters into the workspace

%% Physical Plant (Lumped)
mass = {phy['mass_kg']};
g = {phy['gravity_mss']};
x_eq = {phy['equilibrium_distance_m']};
R_coil = {phy['coil_resistance_ohm']};
L_coil = {phy['coil_inductance_henry']};

%% Magnetic Properties
m_mag_val = {phy['magnet_dipole_moment_Am2']}; 
core_amp_factor = {phy['core_amplification_factor']};

%% Coil Geometry
geom_R1 = {geo['inner_radius_m']};
geom_R2 = {geo['outer_radius_m']};
geom_L = {geo['length_m']};
geom_N = {geo['estimated_turns']};
mu0_val = 4*pi*1e-7;

%% Sensor & Hardware Specs
V_supply = {sensor['supply_voltage_v']};
Hall_Sens_mV_G = {sensor['sensitivity_mv_per_gauss_datasheet']}; 
Sensor_Offset = {sensor['sensor_offset_from_coil_m']};
V_zero_theoretical = {sensor['quiescent_output_v']};
ADC_max = {adc_max};
V_ref = {adc['reference_voltage_v']};
PWM_max = {pwm_max};


%% Control System Targets & Initial Gains
Ts = 1 / {ctrl['loop_frequency_hz']};
Kp_config = {ctrl['pid_gains']['kp']};
Ki_config = {ctrl['pid_gains']['ki']};
Kd_config = {ctrl['pid_gains']['kd']};

disp('✅ Aether-Lock Parameters Loaded Successfully');
"""
    os.makedirs(os.path.dirname(MATLAB_PATH), exist_ok=True)
    with open(MATLAB_PATH, 'w', encoding='utf-8') as f:
        f.write(content)

def generate_markdown_doc(data):
    print(f"🔨 Generating Documentation: {DOCS_PATH}")
    
    phy = data["physical_plant"]
    ctrl = data["control_system"]
    
    content = f"""# System Parameters

> **Note:** This file is auto-generated from `project_config.json`. Do not edit manually.

## Physical Plant
| Parameter | Value | Unit | Description |
| :--- | :--- | :--- | :--- |
| Mass | `{phy['mass_kg']}` | kg | Mass of the Angel |
| Equilibrium Distance | `{phy['equilibrium_distance_m']}` | m | Target air gap |
| Coil Resistance | `{phy['coil_resistance_ohm']}` | Ohm | Electromagnet DC resistance |
| Dipole Moment | `{phy['magnet_dipole_moment_Am2']}` | A·m² | Magnetic Moment |

## Control Settings
| Parameter | Value | Unit | Description |
| :--- | :--- | :--- | :--- |
| Loop Frequency | `{ctrl['loop_frequency_hz']}` | Hz | PID execution rate |
| Kp | `{ctrl['pid_gains']['kp']}` | - | Proportional Gain |
| Ki | `{ctrl['pid_gains']['ki']}` | - | Integral Gain |
| Kd | `{ctrl['pid_gains']['kd']}` | - | Derivative Gain |
"""
    os.makedirs(os.path.dirname(DOCS_PATH), exist_ok=True)
    with open(DOCS_PATH, 'w', encoding='utf-8') as f:
        f.write(content)

if __name__ == "__main__":
    try:
        data = load_config()
        generate_cpp_header(data)
        generate_matlab_script(data)
        generate_markdown_doc(data)
        print("\n✨ All configuration files generated successfully!")
    except FileNotFoundError:
        print("❌ Error: project_config.json not found in the project root.")
    except Exception as e:
        print(f"❌ Error: {e}")