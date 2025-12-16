#include <Arduino.h>
#include "Config.h"
#include "Aether_HAL.h"
#include "PID_Controller.h"
#include "MovingAverage.h"
#include "StateMachine.h"
#include "SerialManager.h"

Aether_HAL hal;
PID_Controller pid(Config::Control::PID::KP, Config::Control::PID::KI, Config::Control::PID::KD, Config::Control::LOOP_PERIOD_S);
MovingAverage filter(Config::Control::FILTER_SIZE);
StateMachine fsm(&hal, &pid, &filter);

SerialManager serialMgr(&pid); 

hw_timer_t *timer = NULL;

// --- SCHARE DATAS (INTERRUPT -> LOOP) ---
volatile float shared_distance = 0.0f;
volatile float shared_pwm = 0.0f;

// --- ISR (5kHz) ---
void IRAM_ATTR onTimer() {
    fsm.update();

    // 2. Salva i dati per la telemetria (Snapshot)
    // Dobbiamo estrarre i dati dagli oggetti. 
    // Nota: L'ideale sarebbe avere dei getter nella FSM, 
    // ma per ora possiamo leggere l'ultimo valore grezzo o calcolarlo.
    // *Per semplicità, facciamo che la FSM esponga questi valori o li salviamo qui.*
    
    // TRUCCO PER ORA:
    // Poiché fsm.update() fa tutto dentro e non ritorna valori, 
    // l'approccio più pulito senza stravolgere la FSM è aggiungere due metodi getter alla FSM:
    // fsm.getLastDistance() e fsm.getLastOutput()
    
    // Assumiamo di averli aggiunti (Vedi sotto Step 4)
    shared_distance = fsm.getLastDistance();
    shared_pwm = fsm.getLastOutput();
}

void setup() {
    serialMgr.init(); 

    hal.init();
    fsm.init();

    timer = timerBegin(0, 80, true);
    timerAttachInterrupt(timer, &onTimer, true);
    timerAlarmWrite(timer, 1000000 / Config::Control::LOOP_FREQ_HZ, true);
    timerAlarmEnable(timer);
}

void loop() {
    serialMgr.handleInput();

    float d = shared_distance;
    float p = shared_pwm;
    
    serialMgr.streamTelemetry(d, p);
}