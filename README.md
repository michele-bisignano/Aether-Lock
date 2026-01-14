# Aether-Lock: Active Magnetic Levitation System

![Language](https://img.shields.io/badge/language-C%2B%2B17-blue.svg)
![Platform](https://img.shields.io/badge/platform-ESP32_WROOM-green.svg)
![Method](https://img.shields.io/badge/method-Model--Based-orange)
![License](https://img.shields.io/badge/license-MIT-lightgrey.svg)

**Aether-Lock** is a high-precision, closed-loop electromagnetic suspension system designed with a **Firmware-First** and **Model-Based** engineering approach.
It stabilizes an inherently unstable system (magnetic levitation) using a custom discrete PID controller running at 5kHz on an ESP32-S2.

---

## 🚀 Key Engineering Features

### 1. Model-Based Design
Instead of heuristic tuning, the control parameters were derived from a physical model.
*   **System Identification:** The magnetic force law $F \propto i/x^n$ was identified via **MATLAB** regression on experimental data.
*   **Linearization:** The unstable plant was linearized around the operating point using Taylor expansion to derive stiffness ($k_x$) and current gain ($k_i$).
*   **Pole Placement:** PID gains were calculated analytically to place closed-loop poles for a damping factor $\zeta \approx 0.707$.

### 2. Single Source of Truth (SSOT) Architecture
The project avoids "magic numbers" in the code.
*   **`project_config.json`**: A central configuration file holding physical constants, pinouts, and safety limits.
*   **Automation:** A Python script (`Tools/generate_config.py`) automatically generates the C++ headers (`Config.h`) and MATLAB simulation parameters (`load_params.m`) before every build, ensuring mathematical consistency.

### 3. Advanced Control Logic
*   **Feedforward Compensation:** Real-time cancellation of the sensor-coil electromagnetic coupling interference.
*   **Thermal Protection:** A software-based thermal model estimates coil temperature in real-time and triggers an emergency shutdown to prevent overheating.
*   **State Machine:** A robust FSM handles system states (`IDLE`, `LEVITATING`, `ERROR`) for safe operation.

---

## 🛠️ System Architecture

### 🔌 Hardware Design

The circuit is designed using **EasyEDA**. It includes the power stage (12V), the logic stage (5V/3.3V), and the signal conditioning for the Hall Sensor.

<!-- Inserisci qui l'immagine del tuo schema se disponibile, altrimenti usa un placeholder -->
[![Circuit Schematic](Hardware/Schematics/Aether_Lock_Schematic_v1.png)](Hardware/Schematics/Aether_Lock_Schematic_v1.pdf)

*   **View Schematic:** [Schematic PDF](Hardware/Schematics/Aether_Lock_Schematic_v1.pdf)
*   **Bill of Materials:** [BOM List](Hardware/BOM.md)

**Circuit Overview:**
*   **MCU:** ESP32-S2 DevKitM (240MHz Single Core).
*   **Actuator:** IRLZ44N Logic-Level MOSFET driving a 12V Solenoid (P25/20) with Flyback diode protection.
*   **Sensor:** SS49E Linear Hall Effect Sensor with an **RC Low-Pass Filter** ($R=1k\Omega, C=100nF$) to suppress PWM noise.
*   **Power & Safety:**
    *   **LM2596** Buck Converter for efficient 12V $\to$ 5V logic power.
    *   **1000µF Capacitor** for bulk power decoupling and voltage sag prevention.
    *   **Thermal Warning LED** triggered by the software model.

### Directory Structure
The project follows a strict modular architecture separating Firmware, Hardware, and Simulation files.

👉 [**View Full Repository Tree**](Docs/Project_Structure/repository_tree.md)

## 📚 Technical Documentation

Detailed engineering reports and theoretical derivations are available in the `Docs` folder:

| Document | Description |
| :--- | :--- |
| [**Mathematical Model**](Docs/Theory/Mathematical_Model.md) | Physical derivation, linearization, and Pole Placement design strategy. |
| [**Hardware Characterization**](Hardware/Theory/Report.md) | Sensor calibration data, noise analysis (R), and actuator coupling analysis. |
| [**System Parameters**](Docs/Theory/System_Parameters.md) | Auto-generated table of current physical constants and controller settings. |