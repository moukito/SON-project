#include "NotchLMSFilter.h"
#include <cmath>

NotchLMSFilter::NotchLMSFilter(const std::size_t order, const double initialCenterFreq, const double initialBandwidth) : notchFilter(initialCenterFreq, initialBandwidth), lmsFilter(order) {
    for (double & i : spectralBuffer) {
        i = 0.0;
    }
}

NotchLMSFilter::~NotchLMSFilter() = default;

double NotchLMSFilter::tick(const double inputSample) {
    const double notchOutput = notchFilter.tick(inputSample);
    
    const double lmsOutput = lmsFilter.tick(notchOutput);
    
    spectralBuffer[spectralBufferIndex] = inputSample;
    spectralBufferIndex = (spectralBufferIndex + 1) % SPECTRAL_BUFFER_SIZE;
    
    if (adaptiveNotchEnabled && spectralBufferIndex == 0) {
        updateNotchFrequency(notchOutput - lmsOutput, lmsOutput);
    }
    
    return lmsOutput;
}

void NotchLMSFilter::setNotchFrequency(double frequency) {
    frequency = std::max(minFrequency, std::min(maxFrequency, frequency));
    notchFilter.setFrequency(frequency);
}

void NotchLMSFilter::setNotchBandwidth(const double bandwidth) {
    notchFilter.setBandwidth(bandwidth);
}

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
