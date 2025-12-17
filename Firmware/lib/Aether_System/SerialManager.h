#ifndef SERIAL_MANAGER_H
#define SERIAL_MANAGER_H

#include <Arduino.h>
#include "PID_Controller.h"
#include "StateMachine.h"

class SerialManager {
private:
    PID_Controller* pid;
    StateMachine* fsm; // Reference to FSM for getting telemetry
    unsigned long last_telemetry_time;
    

    void printHelp();

public:
    SerialManager(PID_Controller* pidPtr, StateMachine* fsmPtr);
    void init();
    void handleInput();
    void streamTelemetry();
};

#endif