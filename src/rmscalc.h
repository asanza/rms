#ifndef RMSCALC_H
#define RMSCALC_H

#include <stdint.h>

/**
 * @brief Adjust this sampling rate according to your system.
 *        This is the sample rate at which the samples for the function
 *        rmscalc are generated.
 */
#define SAMPLE_RATE 1500

#define MAX_FREQ 100
#define MIN_FREQ  30

/**
 * @brief Calculate the rms value from the current sample.
 *        The rms is calculated as the square root of the moving average of
 *        the squared samples, divided by the number of samples in a period
 *        for the given frequency.
 * 
 *        RMS = 1/N * sqrt(sample[1]^2 + .... + sample[N]^2)
 * 
 * @param sample current sample value.
 * @param freq current measured frequency.
 * @return uint32_t the current rms value.
 */
int16_t rmscalc(int16_t sample, uint16_t freq);

#endif // RMSCALC_H
