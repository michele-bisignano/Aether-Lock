#include <Arduino.h>
#include "Config.h"
#include "Aether_HAL.h"
#include "PID_Controller.h"
#include "MovingAverage.h"
#include "StateMachine.h"
#include "SerialManager.h"

// --- OBJECT INSTANTIATION ---
Aether_HAL hal;
PID_Controller pid(Config::Control::PID::KP, Config::Control::PID::KI, Config::Control::PID::KD, Config::Control::LOOP_PERIOD_S);
MovingAverage filter(Config::Control::FILTER_SIZE);
StateMachine fsm(&hal, &pid, &filter);
SerialManager serialMgr(&pid, &fsm); // Pass both pointers

hw_timer_t *timer = NULL;

// --- ISR ---
void IRAM_ATTR onTimer() {
    fsm.update();
}

void setup() {
    serialMgr.init();
    hal.init();
    fsm.init();

    // Timer Setup
    timer = timerBegin(0, 80, true);
    timerAttachInterrupt(timer, &onTimer, true);
    timerAlarmWrite(timer, 1000000 / Config::Control::LOOP_FREQ_HZ, true);
    timerAlarmEnable(timer);
}

void loop() {
    serialMgr.handleInput();
    serialMgr.streamTelemetry();
}