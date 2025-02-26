#ifndef NOTCH_LMS_FILTER_H
#define NOTCH_LMS_FILTER_H

#include "NotchFilter.h"
#include "LMSFilter.h"
#include <cstddef>

class NotchLMSFilter final {
public:
    NotchLMSFilter(std::size_t order, double initialCenterFreq, double initialBandwidth);
    ~NotchLMSFilter();
    
    double tick(double inputSample);
    
    void setMu(const double newMu) { lmsFilter.setMu(newMu); }
    [[nodiscard]] double getMu() const { return lmsFilter.getMu(); }

#ifdef LEAKAGE
    void setLeakage(double newLeakage) { lmsFilter.setLeakage(newLeakage); }
    [[nodiscard]] double getLeakage() const { return lmsFilter.getLeakage(); }
#endif
    
    void setNotchFrequency(double frequency);
    [[nodiscard]] double getNotchFrequency() const { return notchFilter.getCenterFrequency(); }
    void setNotchBandwidth(double bandwidth);
    [[nodiscard]] double getNotchBandwidth() const { return notchFilter.getBandwidth(); }

    void enableNotch(const bool enable) { notchEnabled = enable; }
    [[nodiscard]] bool isNotchEnabled() const { return notchEnabled; }

    void enableLMS(const bool enable) { lmsEnabled = enable; }
    [[nodiscard]] bool isLMSEnabled() const { return lmsEnabled; }

    void enableAdaptiveNotch(const bool enable) { adaptiveNotchEnabled = enable; }
    [[nodiscard]] bool isAdaptiveNotchEnabled() const { return adaptiveNotchEnabled; }
    
    void setFrequencyLimits(const double minFreq, const double maxFreq) {
        minFrequency = minFreq;
        maxFrequency = maxFreq;
    }

    void LMSReset() { lmsFilter.reset(); }

private:
    NotchFilter notchFilter;
    LMSFilter lmsFilter;

    bool notchEnabled{true};
    bool lmsEnabled{true};
    bool adaptiveNotchEnabled{true};
    
    double minFrequency{100.0};
    double maxFrequency{8000.0};
    
    double freqUpdateRate{0.01};
    
    void updateNotchFrequency(double error, double output);
    
    static constexpr size_t SPECTRAL_BUFFER_SIZE = 128;
    double spectralBuffer[SPECTRAL_BUFFER_SIZE]{};
    size_t spectralBufferIndex{0};
    
    [[nodiscard]] double estimateDominantFrequency() const;
};

#endif
