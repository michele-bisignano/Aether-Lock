#include "MovingAverage.h"

MovingAverage::MovingAverage(int size_val) 
    : size(size_val), index(0), sum(0.0f), is_filled(false)
{
    this->buffer = new float[size];
    
    // Inizializza tutto a zero per sicurezza
    reset();
}

MovingAverage::~MovingAverage() {
    delete[] buffer;
}

void MovingAverage::reset() {
    index = 0;
    sum = 0.0f;
    is_filled = false;
    
    for (int i = 0; i < size; i++) {
        buffer[i] = 0.0f;
    }
}

float MovingAverage::process(float input) {
    // 1. Subtract the old value we are about to overwrite from the sum
    sum -= buffer[index];

    // 2. Add the new value to the sum
    sum += input;

    // 3. Save the new value in the buffer (overwriting the old one)
    buffer[index] = input;

    // 4. Advance the index and handle circularity
    index++;
    if (index >= size) {
        index = 0;
        is_filled = true;  // The buffer has been filled at least once
    }

    // 5. Calculate the average
    if (is_filled) {
        return sum / size;
    } else {
        // This avoids the average starting from 0 and rising slowly.
        // 'index' here equals the number of elements inserted so far.
        return sum / index;
    }
}