#include "LMSFilter.h"

LMSFilter::LMSFilter(const std::size_t order, const double mu, const double epsilon)
    : order(order), mu(mu), epsilon(epsilon) {
    reference_buffer = new double[order];
    weights = new double[order];

    for (std::size_t i = 0; i < order; ++i) {
        reference_buffer[i] = 0.0;
        weights[i] = 0.0;
    }
}

LMSFilter::~LMSFilter() {
    delete[] reference_buffer;
    delete[] weights;
}

double LMSFilter::tick(const double micSample) {
    reference_buffer[index] = micSample;

    double estimation = 0.0;
    for (std::size_t i = 0; i < order; ++i) {
        estimation += weights[i] * reference_buffer[(index - i) % order];
    }

    const double error = reference_buffer[(index-1)%order] - estimation;

    double power = 0.0;
    for (std::size_t i = 0; i < order; ++i) {
        const double sample = reference_buffer[i];
        power += sample * sample;
    }

    const double mu_eff = mu / (power + epsilon);

    for (std::size_t i = 0; i < order; ++i) {
        weights[i] += mu_eff * error * reference_buffer[(index - i) % order];
    }

    index = (index + 1) % order;

    return estimation;
}