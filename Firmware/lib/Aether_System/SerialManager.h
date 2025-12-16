#ifndef SERIAL_MANAGER_H
#define SERIAL_MANAGER_H

#include <Arduino.h>
#include "PID_Controller.h"

class SerialManager {
private:
    PID_Controller* pid; // Pointer to PID for change earnings
    unsigned long last_telemetry_time;
    const unsigned long TELEMETRY_INTERVAL_MS = 50; // 20 Hz refresh rate

    void printHelp();

public:
    SerialManager(PID_Controller* pidPtr);

    void init();

    // Reads the serial and updates the gains if commands arrive
    void handleInput();

    // Prints the "Table" of data (CSV format for Excel or Serial Plotter)
    void streamTelemetry(int raw_sensor, float pwm_duty);
};

#endif