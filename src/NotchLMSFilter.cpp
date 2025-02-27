#include "NotchLMSFilter.h"
#include <cmath>

/**
 * @brief Constructs a NotchLMSFilter object.
 *
 * @param order The order of the filter.
 * @param initialCenterFreq The initial center frequency of the notch filter.
 * @param initialBandwidth The initial bandwidth of the notch filter.
 */
NotchLMSFilter::NotchLMSFilter(const std::size_t order, const double initialCenterFreq, const double initialBandwidth)
    : notchFilter(initialCenterFreq, initialBandwidth), lmsFilter(order) {
    for (double & i : spectralBuffer) {
        i = 0.0;
    }
}

/**
 * @brief Destroys the NotchLMSFilter object.
 */
NotchLMSFilter::~NotchLMSFilter() = default;

/**
 * @brief Processes an input sample and returns the filtered output.
 *
 * @param inputSample The input sample to be filtered.
 * @return The filtered output sample.
 */
double NotchLMSFilter::tick(const double inputSample) {
    double notchOutput{};
    if (notchEnabled) {
        notchOutput = notchFilter.tick(inputSample);
    }

    double lmsOutput{inputSample};
    if (lmsEnabled) {
        lmsOutput = lmsFilter.tick(notchEnabled ? notchOutput : inputSample);
    }

    spectralBuffer[spectralBufferIndex] = inputSample;
    spectralBufferIndex = (spectralBufferIndex + 1) % SPECTRAL_BUFFER_SIZE;

    if (adaptiveNotchEnabled && notchEnabled && lmsEnabled && spectralBufferIndex == 0) {
        updateNotchFrequency(notchOutput - lmsOutput, lmsOutput);
    }

    return lmsOutput;
}

/**
 * @brief Sets the center frequency of the notch filter.
 *
 * @param frequency The new center frequency.
 */
void NotchLMSFilter::setNotchFrequency(double frequency) {
    frequency = std::max(minFrequency, std::min(maxFrequency, frequency));
    notchFilter.setFrequency(frequency);
}

/**
 * @brief Sets the bandwidth of the notch filter.
 *
 * @param bandwidth The new bandwidth.
 */
void NotchLMSFilter::setNotchBandwidth(const double bandwidth) {
    notchFilter.setBandwidth(bandwidth);
}

/**
 * @brief Updates the notch filter frequency based on the error and output.
 *
 * @param error The error signal.
 * @param output The output signal.
 */
void NotchLMSFilter::updateNotchFrequency(const double error, const double output) {
    if (std::abs(error) > 0.05 || std::abs(output) > 0.7) {
        if (const double dominantFreq = estimateDominantFrequency(); dominantFreq > 0) {
            const double currentFreq = notchFilter.getCenterFrequency();
            double newFreq = currentFreq * (1.0 - freqUpdateRate) + dominantFreq * freqUpdateRate;

            newFreq = std::max(minFrequency, std::min(maxFrequency, newFreq));

            notchFilter.setFrequency(newFreq);

            const double bandwidth = std::max(50.0, newFreq * 0.1);
            notchFilter.setBandwidth(bandwidth);
        }
    }
}

/**
 * @brief Estimates the dominant frequency in the spectral buffer.
 *
 * @return The estimated dominant frequency.
 */
double NotchLMSFilter::estimateDominantFrequency() const {
    constexpr size_t MAX_LAG = SPECTRAL_BUFFER_SIZE / 2;
    double autocorr[MAX_LAG];

    for (size_t lag = 0; lag < MAX_LAG; ++lag) {
        autocorr[lag] = 0;
        for (size_t i = 0; i < SPECTRAL_BUFFER_SIZE - lag; ++i) {
            autocorr[lag] += spectralBuffer[i] * spectralBuffer[i + lag];
        }
    }

    size_t peakLag = 0;
    double peakValue = 0;

    for (size_t lag = MAX_LAG / 5; lag < MAX_LAG - 1; ++lag) {
        if (autocorr[lag] > autocorr[lag-1] && autocorr[lag] > autocorr[lag+1] && autocorr[lag] > peakValue) {
            peakLag = lag;
            peakValue = autocorr[lag];
        }
    }

    if (peakLag > 0) {
        constexpr double SAMPLE_RATE = AUDIO_SAMPLE_RATE_EXACT;
        return SAMPLE_RATE / peakLag;
    }

    return 0.0;
}