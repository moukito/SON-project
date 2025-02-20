#include "LMSFilter.h"

LMSFilter::LMSFilter(const std::size_t order, const double mu) : order(order), mu(mu) {
    input_buffer = new double[order];
    weights      = new double[order];

    for (std::size_t i = 0; i < order; ++i) {
        input_buffer[i] = 0.0f;
        weights[i]      = 0.0f;
    }
}

LMSFilter::~LMSFilter() {
    delete[] input_buffer;
    delete[] weights;
}

double LMSFilter::getDelayedReference() const {
    return input_buffer[(index - 1)%order];
}

double LMSFilter::computeFilterOutput() const {
    double output{0.0};
    for (std::size_t i = 0; i < order; ++i) {
        output += weights[i] * input_buffer[i];
    }
    return output;
}

void LMSFilter::updateWeights(const double desired, const double output) const {
    const double error = desired - output;

    for (std::size_t i = 0; i < order; ++i) {
        weights[i] += /*TODO : should i put the 2 **/ mu * error * input_buffer[i];
    }
}

double LMSFilter::tick(const double input) {
    input_buffer[index] = input;
    const auto output = computeFilterOutput();

    const auto delayed_signal = getDelayedReference();
    updateWeights(delayed_signal, output);

    index = (index + 1) % order;

    return output;
}