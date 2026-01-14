# Bill of Materials (BOM)

This document lists all the hardware components required to build the Aether-Lock system.

| Component | Description | Qty | Link / Notes |
| :--- | :--- | :---: | :--- |
| **Microcontroller** | ESP32-30Pin CP2102 | 1 | [Link Aliexpress](https://it.aliexpress.com/item/1005007084904270.html) |
| **Voltage Regulator** | LM2596 Step-Down Converter (12V to 5V for MCU) | 1 | [Link Aliexpress](https://it.aliexpress.com/item/1005007724306361.html) |
| **Hall Sensor** | SS49E Linear Hall Effect Sensor (Analog) | 1 | [Link Aliexpress](https://it.aliexpress.com/item/1005006136921399.html) |
| **Electromagnet** | 12V DC Holding Electromagnet (Model P25/20) - 5kg Force | 1 | [Link AliExpress](https://it.aliexpress.com/item/1005010259832011.html) |
| **MOSFET** | IRLZ44N Logic Level N-Channel MOSFET | 1 | [Link Aliexpress](https://it.aliexpress.com/item/1005008120572312.html) |
| **Power Supply** | 12V 2A DC Adapter | 1 | [Link Aliexpress](https://it.aliexpress.com/item/4000521124523.html) |
| **Protection** | Diode 1N4007 (Flyback protection) | 1 | Arduino Kit |
| **Resistors** | 10kΩ (Pull-down) x2, 1kΩ (Filter) x1, 220Ω (LED) x2 | 1 set |  Arduino Kit |
| **Capacitor** | 100nF, 1000uF | 1 set |  Arduino Kit |
| **Levitating Object** | Neodymium Magnet (Permanent Magnet) | 1 | Small & Strong magnet |

## Assembly Notes
*   **Sensor Placement:** The SS49E must be glued exactly at the center of the electromagnet's core.
*   **Power:** The 12V supply powers both the coil and the ESP32 (via VIN pin).