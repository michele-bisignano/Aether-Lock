/**
 * @file main.cpp
 * @brief Entry point of Aether-Lock Firmware.
 * @details Updated architecture: Timer acts as a metronome, logic runs in main loop
 *          to allow safe use of analogRead() and other non-ISR-safe functions.
 */

#include <Arduino.h>
#include "Config.h"
#include "Aether_HAL.h"
#include "PID_Controller.h"
#include "MovingAverage.h"
#include "StateMachine.h"
#include "SerialManager.h"

// --- GLOBAL OBJECTS ---
Aether_HAL hal;

PID_Controller pid(
    Config::Control::PID::KP,
    Config::Control::PID::KI,
    Config::Control::PID::KD,
    Config::Control::LOOP_PERIOD_S
);

MovingAverage filter; // Memory allocated in setup

// Pointers for dynamic objects
StateMachine* fsm = nullptr;
SerialManager* serialMgr = nullptr;

hw_timer_t *timer = NULL;

// --- SYNCHRONIZATION FLAG ---
// Communicates between the ISR and the Main Loop
volatile bool run_control_loop = false;

// --- ISR (Interrupt Service Routine) ---
// Acts only as a trigger. Extremely fast and safe.
void IRAM_ATTR onTimer() {
    run_control_loop = true;
}

void setup() {
    // 1. Initialize Serial Communication
    Serial.begin(115200);
    delay(1000); // Allow serial connection to stabilize
    Serial.println(">>> BOOTING AETHER-LOCK...");

    // 2. Initialize Hardware
    hal.init();
    Serial.println("HAL Initialized.");

    // 3. Filter Memory Allocation (CRITICAL)
    // Must be done before creating the FSM
    if (!filter.begin(Config::Control::FILTER_SIZE)) {
        Serial.println("❌ CRITICAL ERROR: Filter Memory Allocation Failed!");
        while (1); // Stop execution
    }
    Serial.println("Filter Memory Allocated.");

    // 4. Instantiate Logic Objects
    // We use dynamic allocation (new) to ensure order of initialization
    fsm = new StateMachine(&hal, &pid, &filter);
    fsm->init();
    Serial.println("FSM Initialized.");

    serialMgr = new SerialManager(&pid, fsm);
    serialMgr->init();
    Serial.println("Serial Manager Initialized.");

    // 5. Timer Configuration
    Serial.println("Starting Timer...");
    timer = timerBegin(0, 80, true);
    timerAttachInterrupt(timer, &onTimer, true);
    timerAlarmWrite(timer, 1000000 / Config::Control::LOOP_FREQ_HZ, true);
    
    // Enable timer only after everything is ready
    timerAlarmEnable(timer);

    Serial.println(">>> SYSTEM RUNNING. Control Loop Active.");
}

void loop() {
    // 1. High Priority: Control Logic (Triggered by Timer Flag)
    if (run_control_loop) {
        // Reset flag immediately to acknowledge the "tick"
        run_control_loop = false; 

        if (fsm != nullptr) {
            fsm->update();
        }
    }

    // 2. Low Priority: Serial Communication & Telemetry
    // These run as fast as possible in the remaining CPU time
    if (serialMgr != nullptr) {
        serialMgr->handleInput();
        serialMgr->streamTelemetry();
    }
}