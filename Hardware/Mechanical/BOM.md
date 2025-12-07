# Bill of Materials (BOM)

This document lists all the hardware components required to build the Aether-Lock system.

| Component | Description | Qty | Link / Notes |
| :--- | :--- | :---: | :--- |
| **Microcontroller** | ESP32-S2 DevKitM (Puthernm) - WROOM Module | 1 | [Link Amazon](https://www.amazon.it/dp/B0G43PGPQH) |
| **Voltage Regulator** | LM2596 Step-Down Converter (12V to 5V for MCU) | 1 | [Link Amazon](https://www.amazon.it/AZ-Delivery-LM2596S-Step-Down-Convertitore-Digitale/dp/B08T158WPS ) 
| **Hall Sensor** | SS49E Linear Hall Effect Sensor (Analog) - Pack of 10 | 1 | [Link Amazon](https://www.amazon.it/Reland-Sun-elemento-Sensore-Effetto/dp/B09CMP2RK9) |
| **Electromagnet** | 12V DC Holding Electromagnet (Model P25/20 or P20/15) - 5kg Force | 1 | [Link Amazon](https://www.amazon.it/Heschen-Elettromagnete-Solenoide-diametro-kilogram/dp/B078KN9D8V) |
| **MOSFET** | IRLZ44N Logic Level N-Channel MOSFET (Pack of 10) | 1 | [Link Amazon](https://www.amazon.it/dp/B09WV8W8ZG) |
| **Magnet** | Neodymium Magnet (NdFeB) - Disc 8mm x 3mm (N35/N42) | 1 | [Link Acquisto](INSERISCI_QUI_IL_LINK) |
| **Power Supply** | 12V 2A DC Adapter | 1 | [Link Acquisto](https://www.techmaker.it/altro/88-diodo-1n4007) |
| **Protection** | Diode 1N4007 (Flyback protection) | 1 || [Link Acquisto](https://www.techmaker.it/altro/88-diodo-1n4007) |
| **Resistors** | 10kΩ (Pull-down) | 1 |  [Link Acquisto](https://www.techmaker.it/resistori/104-resistore-da-10-k-ohm) |

## Assembly Notes
*   **Sensor Placement:** The SS49E must be glued exactly at the center of the electromagnet's core.
*   **Power:** The 12V supply powers both the coil and the ESP32 (via VIN pin).