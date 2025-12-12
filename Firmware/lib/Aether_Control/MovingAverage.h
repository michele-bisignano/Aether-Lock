#ifndef MOVING_AVERAGE_H
#define MOVING_AVERAGE_H

/**
 * @brief Efficient Moving Average Filter (Circular Buffer implementation).
 * 
 * Used to smooth out sensor noise before the PID loop.
 * Features O(1) time complexity: calculation time does not increase with filter size.
 */
class MovingAverage {
private:
    float* buffer;      // Dynamic array to store history
    const int size;     // Total size of the buffer (N samples)
    int index;          // Current write position (head of the circular buffer)
    float sum;          // Running sum of all elements in buffer
    bool is_filled;     // Flag to handle the startup phase gracefully

public:
    /**
     * @brief Constructor. Allocates memory for the filter.
     * @param size Number of samples to average (e.g., 4, 8, 10).
     */
    MovingAverage(int size);

    /**
     * @brief Destructor. Frees the allocated memory to prevent leaks.
     * Important because we use 'new' in the constructor.
     */
    ~MovingAverage();

    /**
     * @brief Processes a new sample and returns the filtered value.
     * 
     * @param input The raw noisy value from the sensor.
     * @return float The smoothed average value.
     */
    float process(float input);

    /**
     * @brief Resets the filter.
     * Clears the buffer and resets the sum (e.g., on system startup).
     */
    void reset();
};

#endif