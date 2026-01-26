#include "MovingAverage.h"
#include <stddef.h> // Per NULL

MovingAverage::MovingAverage() {
    buffer = NULL;
    size = 0;
    index = 0;
    sum = 0.0f;
    is_filled = false;
    initialized = false;
}

MovingAverage::~MovingAverage() {
    if (buffer != NULL) {
        delete[] buffer;
        buffer = NULL;
    }
}

// Ora restituisce bool
bool MovingAverage::begin(int size_val) {
    if (initialized) return true; // Già inizializzato, tutto ok
    
    // Protezione input
    if (size_val <= 0) return false; 

    this->size = size_val;
    
    // Allocazione memoria
    this->buffer = new float[size];
    
    // Controllo se l'allocazione è fallita (Mancanza di RAM)
    if (this->buffer == NULL) {
        return false; 
    }
    
    reset();
    initialized = true;
    return true; // Successo
}

void MovingAverage::reset() {
    index = 0;
    sum = 0.0f;
    is_filled = false;
    if (buffer != NULL) {
        for (int i = 0; i < size; i++) {
            buffer[i] = 0.0f;
        }
    }
}

float MovingAverage::process(float input) {
    // Se non inizializzato o memoria fallita, ritorna input grezzo
    if (!initialized || buffer == NULL) {
        return input;
    }

    sum -= buffer[index];
    sum += input;
    buffer[index] = input;

    index++;
    if (index >= size) {
        index = 0;
        is_filled = true;
    }

    if (is_filled) {
        return sum / size;
    } else {
        if (index == 0) return input;
        return sum / index;
    }
}