## 🔌 Hardware Design

The circuit is designed using EasyEDA. It includes the power stage (12V), the logic stage (5V/3.3V), and the signal conditioning for the Hall Sensor.

[![Circuit Schematic](Hardware/Schematics/Aether_Lock_Schematic_v1.png)](Hardware/Schematics/Aether_Lock_Schematic_v1.pdf)

*   **View Schematic:** [Schematic PDF](Hardware/Schematics/Aether_Lock_Schematic_v1.pdf)
*   **Bill of Materials:** [BOM List](Hardware/Mechanical/BOM.md)

### Circuit Overview
*   **MCU:** ESP32-S2 DevKitM
*   **Actuator:** IRLZ44N Logic-Level MOSFET driving a 12V Solenoid (with Flyback diode).
*   **Sensor:** SS49E Hall Effect Sensor with RC Low-Pass Filter ($R=1k\Omega, C=100nF$).
*   **Safety:** LM2596 Buck Converter for logic power & Thermal Warning LED.