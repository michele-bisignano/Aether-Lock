#ifndef MOVING_AVERAGE_H
#define MOVING_AVERAGE_H

/**
 * @brief Efficient Moving Average Filter (Circular Buffer implementation).
 */
class MovingAverage {
private:
    float* buffer;      
    int size;           
    int index;          
    float sum;          
    bool is_filled;     
    bool initialized;   

public:
    MovingAverage();
    ~MovingAverage();

    /**
     * @brief Initializes the filter and allocates memory.
     * @param size Number of samples.
     * @return true if memory allocation was successful, false otherwise.
     */
    bool begin(int size); // <--- CAMBIATO DA void A bool

    float process(float input);
    void reset();
};

#endif