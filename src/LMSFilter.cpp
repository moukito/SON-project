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

    double gamma{1.0};

#ifdef ADAPTATIVE_GAMMA
    signalVariance = alpha * signalVariance + (1.0 - alpha) * micSample * micSample;
    errorVariance = alpha * errorVariance + (1.0 - alpha) * error * error;

    double snr = (signalVariance > 1e-10) ? (signalVariance / (errorVariance + 1e-10)) : 1.0;

    if (snr > 10.0) {
        mu = muMax;
    } else if (snr < 2.0) {
        mu = muMin;
    } else {
        mu = muMin + (muMax - muMin) * (snr - 2.0) / 8.0;
    }

    double gamma;
    if (errorVariance > 0.1) {
        gamma = gammaMin;
    } else if (errorVariance < 0.01) {
        gamma = gammaMax;
    } else {
        gamma = gammaMin + (gammaMax - gammaMin) * (0.1 - errorVariance) / 0.09;
    }
#endif

#ifdef NLMS
    constexpr double epsilon{1e-6};
    power += reference_buffer[index] * reference_buffer[index];

    const double mu_eff = mu / (power + epsilon);
#else
    const double mu_eff = mu;
#endif

    for (std::size_t i = 0; i < order; ++i) {
        weights[i] = weights[i] * gamma + mu_eff * error * reference_buffer[(index - i + order) % order];
    }

    index = (index + 1) % order;

    return error;
}