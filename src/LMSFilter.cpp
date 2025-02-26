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

#ifdef KALMAN
    double updateKalmanVariance(const double currentEstimate, double& estimationError, const double measurement, const double processNoise, const double measurementNoise) {
        const double prediction = currentEstimate;
        const double predictionError = estimationError + processNoise;

        const double kalmanGain = predictionError / (predictionError + measurementNoise);
        const double newEstimate = prediction + kalmanGain * (measurement - prediction);
        const double newEstimationError = (1.0 - kalmanGain) * predictionError;

        estimationError = newEstimationError;
        return newEstimate;
    }
#endif

#ifdef DYNAMIC_NOISE
    void LMSFilter::updateNoiseParameters(const double error) {
        signalValues[windowIndex] = reference_buffer[index] * reference_buffer[index];
        errorValues[windowIndex] = error * error;

        windowIndex = (windowIndex + 1) % ESTIMATION_WINDOW;
        if (windowIndex == 0) windowFilled = true;

        if (!windowFilled) return;

        double signalMean = 0.0, errorMean = 0.0;
        double signalVar = 0.0, errorVar = 0.0;

        for (int i = 0; i < ESTIMATION_WINDOW; i++) {
            signalMean += signalValues[i];
            errorMean += errorValues[i];
        }
        signalMean /= ESTIMATION_WINDOW;
        errorMean /= ESTIMATION_WINDOW;

        for (int i = 0; i < ESTIMATION_WINDOW; i++) {
            signalVar += (signalValues[i] - signalMean) * (signalValues[i] - signalMean);
            errorVar += (errorValues[i] - errorMean) * (errorValues[i] - errorMean);
        }
        signalVar /= ESTIMATION_WINDOW;
        errorVar /= ESTIMATION_WINDOW;

        signalMeasurementNoise = std::max(0.01, std::min(1.0, signalVar * 0.1));
        errorMeasurementNoise = std::max(0.01, std::min(1.0, errorVar * 0.1));

        double signalMeanFirst = 0.0, signalMeanLast = 0.0;
        double errorMeanFirst = 0.0, errorMeanLast = 0.0;
        constexpr int subWindowSize = ESTIMATION_WINDOW / 5;

        for (int i = 0; i < subWindowSize; i++) {
            signalMeanFirst += signalValues[i];
            errorMeanFirst += errorValues[i];
            signalMeanLast += signalValues[ESTIMATION_WINDOW - 1 - i];
            errorMeanLast += errorValues[ESTIMATION_WINDOW - 1 - i];
        }

        signalMeanFirst /= subWindowSize;
        errorMeanFirst /= subWindowSize;
        signalMeanLast /= subWindowSize;
        errorMeanLast /= subWindowSize;

        const double signalChange = std::abs(signalMeanLast - signalMeanFirst) / signalMean;
        const double errorChange = std::abs(errorMeanLast - errorMeanFirst) / errorMean;

        signalProcessNoise = std::max(0.001, std::min(0.1, signalChange * 0.05));
        errorProcessNoise = std::max(0.001, std::min(0.1, errorChange * 0.05));
    }
#endif

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

#ifdef DYNAMIC_NOISE
    updateNoiseParameters(error);
#endif

    double gamma{leakage};

#ifdef ADAPTIVE_GAMMA
    const double signalMeasurement = micSample * micSample;
    const double errorMeasurement = error * error;

#ifdef KALMAN
    signalVarianceEstimate = updateKalmanVariance(signalVarianceEstimate,signalVarianceError,signalMeasurement, signalProcessNoise, signalMeasurementNoise);
    errorVarianceEstimate = updateKalmanVariance(errorVarianceEstimate,errorVarianceError,errorMeasurement, errorProcessNoise, errorMeasurementNoise);
#else
    signalVarianceEstimate = alpha * signalVarianceEstimate + (1.0 - alpha) * micSample * micSample;
    errorVarianceEstimate = alpha * errorVarianceEstimate + (1.0 - alpha) * error * error;
#endif

    const double snr = (signalVarianceEstimate > 1e-10) ? (signalVarianceEstimate / (errorVarianceEstimate + 1e-10)) : 1.0;

    if (snr > 10.0) {
        mu = muMax;
    } else if (snr < 2.0) {
        mu = muMin;
    } else {
        mu = muMin + (muMax - muMin) * (snr - 2.0) / 8.0;
    }

    if (errorVarianceEstimate > 0.1) {
        gamma = gammaMin;
    } else if (errorVarianceEstimate < 0.01) {
        gamma = gammaMax;
    } else {
        gamma = gammaMin + (gammaMax - gammaMin) * (0.1 - errorVarianceEstimate) / 0.09;
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