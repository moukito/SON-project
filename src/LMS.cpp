#include "LMS.h"

#define DELAY_LENGTH 10

LMSFilter::LMSFilter(int order, float step_size) : mu(step_size) {
    input_buffer.resize(order, 0.0f);
    delay_buffer.resize(DELAY_LENGTH, 0.0f);
    weights.resize(order, 0.0f);
}

// Update the input buffer by removing the oldest sample and adding the new sample
void LMSFilter::updateInputBuffer(float new_sample) {
    for (int i = input_buffer.size() - 1; i > 0; --i) {
        input_buffer[i] = input_buffer[i - 1];
    }
    input_buffer[0] = new_sample;
}

// Update the delay buffer by removing the oldest sample and adding the new sample
void LMSFilter::updateDelayBuffer(float new_sample) {
    for (int i = DELAY_LENGTH - 1; i > 0; --i) {
        delay_buffer[i] = delay_buffer[i - 1];
    }
    delay_buffer[0] = new_sample;
}

// Get the oldest sample from the delay buffer
float LMSFilter::getDelayedReference() {
    return delay_buffer[DELAY_LENGTH - 1];
}

// Compute the filter output
float LMSFilter::computeFilterOutput() {
    float output = 0.0;
    for (std::size_t i = 0; i < input_buffer.size(); ++i) {
        output += weights[i] * input_buffer[i];
    }
    return output;
}

// Update the filter weights
void LMSFilter::updateWeights(float desired) {
    float output = computeFilterOutput();
    float error = desired - output;

    for (std::size_t i = 0; i < weights.size(); ++i) {
        weights[i] += 2 * mu * error * input_buffer[i];
    }
}

// Treatment of one input sample by passing the LMS filter
float LMSFilter::process(float input) {
    updateInputBuffer(input);
    updateDelayBuffer(input); 
    float delayed_signal = getDelayedReference();
    updateWeights(delayed_signal);
    return computeFilterOutput();
}