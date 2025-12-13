#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H

#include <Arduino.h>
#include "Aether_HAL.h"
#include "PID_Controller.h"
#include "MovingAverage.h"
#include "Config.h"

// States definition
enum SystemState {
    STATE_IDLE,         // Coil off, waiting for positioning
    STATE_LEVITATING,   // Active PID control
    STATE_ERROR         // Thermal/fall protection activated
};

class StateMachine {
private:
    // Dependency Injection
    Aether_HAL* hal;
    PID_Controller* pid;
    MovingAverage* filter;

    SystemState currentState;

    // Variables for time management (Safety)
    unsigned long lastTimeInRange; // Last time angel was seen "nearby"

    // Safety constants (could go into Config, but for now we keep them here)
    const unsigned long FALL_TIMEOUT_MS = 1000; // If it falls for 1 sec, turn everything off

    // Private methods to handle each state
    void handleIdle();
    void handleLevitating();
    void handleError();

    // Helper to check if the angel is in the capture range
    bool isAngelInRange(float distance);

public:
    /**
     * @brief Constructor. Receives pointers to subsystems.
     */
    StateMachine(Aether_HAL* halPtr, PID_Controller* pidPtr, MovingAverage* filterPtr);

    /**
     * @brief Initialize the state machine.
     */
    void init();

    /**
     * @brief Run the main logic. To be called in the Loop or Timer.
     */
    void update();

    /**
     * @brief Forces a reset of errors (e.g. via serial command)
     */
    void resetError();
};

#endif