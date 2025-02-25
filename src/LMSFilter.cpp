#include "LMSFilter.h"

LMSFilter::LMSFilter(const std::size_t order, const double mu)
    : order(order), mu(mu) {
    reference_buffer = new double[order];
    weights = new double[order];

    reset();
}

LMSFilter::~LMSFilter() {
    delete[] reference_buffer;
    delete[] weights;
}

void LMSFilter::reset() {
    for (std::size_t i = 0; i < order; ++i) {
        reference_buffer[i] = 0.0;
        weights[i] = 0.0;
    }
    index = 0;
#ifdef NLMS
    power = 0.0;
#endif
}

double LMSFilter::tick(const double micSample) {
#ifdef NLMS
    power -= reference_buffer[index] * reference_buffer[index];
#endif
    reference_buffer[index] = micSample;

    double estimation = 0.0;
    for (std::size_t i = 0; i < order; ++i) {
        estimation += weights[i] * reference_buffer[(index - i + order) % order];
    }

    const double error = reference_buffer[index] - estimation;

#ifdef NLMS
    constexpr double epsilon{1e-6};
    power += reference_buffer[index] * reference_buffer[index];

    const double mu_eff = mu / (power + epsilon);
#else
    const double mu_eff = mu;
#endif

    for (std::size_t i = 0; i < order; ++i) {
        weights[i] += mu_eff * error * reference_buffer[(index - i + order) % order];
    }

    index = (index + 1) % order;

    return error;
}