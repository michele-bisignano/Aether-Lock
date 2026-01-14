# Aether-Lock: Active Magnetic Levitation System

![Language](https://img.shields.io/badge/language-C%2B%2B17-blue.svg)
![Platform](https://img.shields.io/badge/platform-ESP32_WROOM-green.svg)
![Method](https://img.shields.io/badge/method-Model--Based-orange)
![License](https://img.shields.io/badge/License-Apache_2.0-blue.svg)

**Aether-Lock** is a high-precision, closed-loop electromagnetic suspension system designed with a **Firmware-First** and **Model-Based** engineering approach.
It stabilizes an inherently unstable system (magnetic levitation) using a custom discrete PID controller running at 5kHz on an ESP32-S2.

---

## 🚀 Key Engineering Features

### 1. Model-Based Design
Control parameters were derived analytically from physical modeling.
*   **System Identification:** The magnetic force law $F \propto i/x^n$ was identified via [**MATLAB Regression Scripts**](Simulation/MATLAB/identify_physics.m) on experimental data.
*   **Linearization & Tuning:** PID gains were calculated via [**Pole Placement**](Simulation/MATLAB/design_pid.m) to ensure a damping factor $\zeta \approx 0.707$.
*   **Theory Documentation:** See [**Mathematical Model & Design**](Docs/Theory/Mathematical_Model.md).

### 2. Single Source of Truth (SSOT) Architecture
The project enforces consistency across Firmware, Simulation, and Documentation.
*   [**`project_config.json`**](project_config.json): The central registry for physical constants, pinouts, and safety limits.
*   [**`Tools/generate_config.py`**](Tools/generate_config.py): A build script that auto-generates C++ headers and MATLAB variables from the JSON.

### 3. Advanced Control Logic
*   **Feedforward Compensation:** Real-time cancellation of the sensor-coil electromagnetic coupling.
    *   *Implementation:* [StateMachine.cpp](Firmware/lib/Aether_System/StateMachine.cpp)
*   **Thermal Protection:** A software-based thermal model estimates coil temperature.
    *   *Implementation:* [ThermalGuard.cpp](Firmware/lib/Aether_System/ThermalGuard.cpp)
---

## 🛠️ Hardware & Wiring

The system is built on the ESP32 WROOM architecture, featuring a custom power stage for the solenoid and signal conditioning for the Hall sensor.

For detailed schematics, the complete Bill of Materials (BOM), and assembly instructions, please refer to the dedicated documentation:

👉 **[Read Hardware Design & Assembly Guide](Docs/Hardware/Design_and_Assembly.md)**
### Directory Structure
The project follows a strict modular architecture separating Firmware, Hardware, and Simulation files.

👉 [**View Full Repository Tree**](Docs/Project_Structure/repository_tree.md)

## 📚 Technical Documentation

Detailed engineering reports and theoretical derivations are available in the `Docs` folder:

| Document | Description |
| :--- | :--- |
| [**Mathematical Model**](Docs/Theory/Mathematical_Model.md) | Physical derivation, linearization, and Pole Placement design strategy. |
| [**Hardware Characterization**](Docs/Hardware/Design_and_Assembly.md) | Sensor calibration data, noise analysis (R), and actuator coupling analysis. |
| [**System Parameters**](Docs/Theory/System_Parameters.md) | Auto-generated table of current physical constants and controller settings. |