#include "LMSFilter.h"

LMSFilter::LMSFilter(const size_t order, const float mu) : order(order), mu(mu) {
    input_buffer = new double[order];
    delay_buffer = new double[order];

    for (size_t i = 0; i < order; ++i) {
        input_buffer[i] = 0.0f;
        delay_buffer[i] = 0.0f;
    }

    weights.resize(order, 0.0f);
}

void LMSFilter::updateBuffer(float new_sample) {
    input_buffer[index] = new_sample;
    delay_buffer[index] = new_sample;
    index = (index + 1) % order;
}

// Get the oldest sample from the delay buffer
double LMSFilter::getDelayedReference() const {
    return delay_buffer[(index - 1)%order];
}

// Compute the filter output
double LMSFilter::computeFilterOutput() const {
    double output = 0.0;
    for (std::size_t i = 0; i < order; ++i) {
        output += weights[i] * input_buffer[i];
    }
    return output;
}

// Update the filter weights
void LMSFilter::updateWeights(const double desired) {
    const double output = computeFilterOutput();
    const double error = desired - output;

    for (std::size_t i = 0; i < weights.size(); ++i) {
        weights[i] += 2 * mu * error * input_buffer[i];
    }
}

// TODO : check compute filter output
// Treatment of one input sample by passing the LMS filter
double LMSFilter::process(const float input) {
    updateBuffer(input);
    const auto delayed_signal = getDelayedReference();
    updateWeights(delayed_signal);
    return computeFilterOutput();
}