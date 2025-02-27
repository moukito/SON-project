#ifndef IMPROVED_NOTCH_LMS_FILTER_H
#define IMPROVED_NOTCH_LMS_FILTER_H

#include <vector>
#include <cmath>
#include <Audio.h>
#include <algorithm>
#include "LMSFilter.h"

/**
 * @brief The ImprovedNotchLMSFilter class combines multiple adaptive notch filters with an LMS filter.
 *
 * This class provides an advanced approach to feedback cancellation by using a bank of
 * adaptive notch filters combined with an LMS filter. It uses different strategies to
 * process the signal based on its characteristics.
 */
class ImprovedNotchLMSFilter {
public:
    /**
     * @brief Strategy enumeration for combining notch and LMS filters.
     */
    enum class Strategy {
        NOTCH_FIRST,    ///< Notch filter followed by LMS filter.
        LMS_FIRST,      ///< LMS filter followed by notch filter.
        PARALLEL,       ///< Both filters in parallel, then mix the results.
        ADAPTIVE        ///< Strategy selected based on signal characteristics.
    };

    /**
     * @brief Constructs an ImprovedNotchLMSFilter with specified LMS order and initial frequency.
     *
     * @param lmsOrder The order of the LMS filter.
     * @param initialFreq The initial center frequency for the notch filters.
     */
    ImprovedNotchLMSFilter(size_t lmsOrder, double initialFreq = 1000.0);
    
    /**
     * @brief Processes an input sample and returns the filtered output.
     *
     * @param input The input sample to be filtered.
     * @return The filtered output sample.
     */
    double process(double input);
    
    /**
     * @brief Enables or disables the notch filter bank.
     *
     * @param enable True to enable, false to disable.
     */
    void enableNotch(bool enable) { notchEnabled = enable; }
    
    /**
     * @brief Checks if the notch filter bank is enabled.
     *
     * @return True if enabled, false otherwise.
     */
    [[nodiscard]] bool isNotchEnabled() const { return notchEnabled; }
    
    /**
     * @brief Enables or disables the LMS filter.
     *
     * @param enable True to enable, false to disable.
     */
    void enableLMS(bool enable) { lmsEnabled = enable; }
    
    /**
     * @brief Checks if the LMS filter is enabled.
     *
     * @return True if enabled, false otherwise.
     */
    [[nodiscard]] bool isLMSEnabled() const { return lmsEnabled; }
    
    /**
     * @brief Sets the strategy for combining notch and LMS filters.
     *
     * @param strategy The new strategy to use.
     */
    void setStrategy(Strategy strategy) { currentStrategy = strategy; }
    
    /**
     * @brief Gets the current strategy.
     *
     * @return The current strategy.
     */
    [[nodiscard]] Strategy getStrategy() const { return currentStrategy; }
    
    /**
     * @brief Resets the LMS filter.
     */
    void reset();
    
private:
    /**
     * @brief The AdaptiveNotchFilter class implements an adaptive biquad notch filter.
     *
     * This class provides a notch filter with variable frequency and Q factor.
     */
    class AdaptiveNotchFilter {
    public:
        /**
         * @brief Constructs an AdaptiveNotchFilter with specified initial frequency and Q factor.
         *
         * @param initialFreq The initial center frequency.
         * @param initialQ The initial Q factor.
         */
        AdaptiveNotchFilter(double initialFreq, double initialQ);
        
        /**
         * @brief Processes an input sample and returns the filtered output.
         *
         * @param input The input sample to be filtered.
         * @return The filtered output sample.
         */
        double process(double input);
        
        /**
         * @brief Sets the center frequency of the notch filter.
         *
         * @param freq The new center frequency.
         */
        void setFrequency(double freq);
        
        /**
         * @brief Gets the current center frequency of the notch filter.
         *
         * @return The current center frequency.
         */
        [[nodiscard]] double getFrequency() const { return centerFreq; }
        
        /**
         * @brief Sets the Q factor of the notch filter.
         *
         * @param q The new Q factor.
         */
        void setQ(double q);
        
        /**
         * @brief Gets the current Q factor of the notch filter.
         *
         * @return The current Q factor.
         */
        [[nodiscard]] double getQ() const { return qFactor; }
        
    private:
        /**
         * @brief Updates the filter coefficients based on the current frequency and Q factor.
         */
        void updateCoefficients();
        
        double b0{1.0}, b1{0.0}, b2{1.0}; ///< Feed-forward coefficients.
        double a1{0.0}, a2{0.0};          ///< Feed-back coefficients.
        double x1{0.0}, x2{0.0};          ///< Previous input samples.
        double y1{0.0}, y2{0.0};          ///< Previous output samples.
        
        double centerFreq;               ///< Center frequency of the notch filter.
        double qFactor;                  ///< Q factor of the notch filter.
    };
    
    /**
     * @brief The FFTFeedbackDetector class detects feedback using FFT analysis.
     *
     * This class provides functionality to detect feedback in an audio signal
     * by analyzing its frequency spectrum.
     */
    class FFTFeedbackDetector {
    public:
        /**
         * @brief Constructs an FFTFeedbackDetector with specified buffer size.
         *
         * @param bufferSize The size of the buffer for FFT analysis.
         */
        explicit FFTFeedbackDetector(size_t bufferSize = 256);
        
        /**
         * @brief Adds a sample to the buffer and performs FFT analysis when the buffer is full.
         *
         * @param sample The sample to add to the buffer.
         */
        void addSample(double sample);
        
        /**
         * @brief Gets the list of detected feedback frequencies.
         *
         * @return A vector of detected feedback frequencies.
         */
        [[nodiscard]] const std::vector<double>& getDetectedFrequencies() const { return detectedFrequencies; }
        
        /**
         * @brief Gets the energy of the signal in the buffer.
         *
         * @return The energy of the signal.
         */
        [[nodiscard]] double getSignalEnergy() const { return signalEnergy; }
        
    private:
        /**
         * @brief Performs FFT analysis on the buffer data.
         */
        void performFFTAnalysis();
        
        std::vector<double> buffer;          ///< Buffer for storing samples.
        size_t bufferIndex{0};               ///< Current index in the buffer.
        std::vector<double> detectedFrequencies; ///< List of detected feedback frequencies.
        double signalEnergy{0.0};            ///< Energy of the signal in the buffer.
    };
    
    /**
     * @brief Selects the best strategy based on signal characteristics.
     *
     * @param inputEnergy The energy of the input signal.
     * @param errorEnergy The energy of the error signal.
     */
    void selectStrategy(double inputEnergy, double errorEnergy);
    
    /**
     * @brief Updates the notch bank based on detected feedback frequencies.
     */
    void updateNotchBank();
    
    std::vector<AdaptiveNotchFilter> notchBank;  ///< Bank of adaptive notch filters.
    LMSFilter lmsFilter;                         ///< LMS filter instance.
    FFTFeedbackDetector feedbackDetector;        ///< Feedback detector instance.
    
    Strategy currentStrategy{Strategy::ADAPTIVE}; ///< Current strategy for combining filters.
    bool notchEnabled{true};                     ///< Flag indicating if the notch filter bank is enabled.
    bool lmsEnabled{true};                       ///< Flag indicating if the LMS filter is enabled.
    
    // Signal statistics for adaptive strategy selection
    double avgInputEnergy{0.0};                  ///< Average input signal energy.
    double avgErrorEnergy{0.0};                  ///< Average error signal energy.
    double alpha{0.95};                          ///< Smoothing factor for averages.
    
    // Parameters for notch filter management
    static constexpr size_t MAX_NOTCH_FILTERS{5}; ///< Maximum number of notch filters in the bank.
    static constexpr double MIN_FREQUENCY{100.0}; ///< Minimum frequency for notch filters.
    static constexpr double MAX_FREQUENCY{8000.0}; ///< Maximum frequency for notch filters.
    static constexpr double MIN_Q_FACTOR{1.0};    ///< Minimum Q factor for notch filters.
    static constexpr double MAX_Q_FACTOR{30.0};   ///< Maximum Q factor for notch filters.
    
    // Helper buffer for signal processing
    double lastInput{0.0};                       ///< Last input sample.
    double lastError{0.0};                       ///< Last error sample.
};

// Implementation of ImprovedNotchLMSFilter

ImprovedNotchLMSFilter::ImprovedNotchLMSFilter(const size_t lmsOrder, const double initialFreq)
    : lmsFilter(lmsOrder, 0.001),
      feedbackDetector(512) {
    // Initialize notch bank with one filter at the initial frequency
    notchBank.emplace_back(initialFreq, 10.0);
}

double ImprovedNotchLMSFilter::process(double input) {
    // Add sample to feedback detector for analysis
    feedbackDetector.addSample(input);
    
    // Update notch bank based on detected feedback frequencies
    updateNotchBank();
    
    double notchOutput = input;
    double lmsOutput = input;
    double output = input;
    
    // Process the input based on the current strategy and enabled filters
    if (currentStrategy == Strategy::ADAPTIVE) {
        // Calculate input energy for this sample
        const double inputEnergy = input * input;
        avgInputEnergy = alpha * avgInputEnergy + (1.0 - alpha) * inputEnergy;
        
        // Select the best strategy based on signal characteristics
        selectStrategy(avgInputEnergy, avgErrorEnergy);
    }
    
    switch (currentStrategy) {
        case Strategy::NOTCH_FIRST:
            if (notchEnabled) {
                // Apply all notch filters in series
                for (auto& notch : notchBank) {
                    notchOutput = notch.process(notchOutput);
                }
                output = notchOutput;
            }
            
            if (lmsEnabled) {
                // Apply LMS filter after notch filtering
                output = lmsFilter.tick(output);
            }
            break;
            
        case Strategy::LMS_FIRST:
            if (lmsEnabled) {
                // Apply LMS filter first
                lmsOutput = lmsFilter.tick(input);
                output = lmsOutput;
            }
            
            if (notchEnabled) {
                // Apply all notch filters in series after LMS filtering
                for (auto& notch : notchBank) {
                    output = notch.process(output);
                }
            }
            break;
            
        case Strategy::PARALLEL:
            if (notchEnabled) {
                // Apply all notch filters in series
                for (auto& notch : notchBank) {
                    notchOutput = notch.process(notchOutput);
                }
            } else {
                notchOutput = input;
            }
            
            if (lmsEnabled) {
                // Apply LMS filter separately
                lmsOutput = lmsFilter.tick(input);
            } else {
                lmsOutput = input;
            }
            
            // Mix the outputs of both filters (50/50)
            output = 0.5 * notchOutput + 0.5 * lmsOutput;
            break;
            
        case Strategy::ADAPTIVE:
            // This case should be handled before the switch, but include as fallback
            if (notchEnabled) {
                for (auto& notch : notchBank) {
                    notchOutput = notch.process(notchOutput);
                }
                output = notchOutput;
            }
            
            if (lmsEnabled) {
                output = lmsFilter.tick(output);
            }
            break;
    }
    
    // Calculate error energy for adaptive strategy
    const double error = output - input;
    const double errorEnergy = error * error;
    avgErrorEnergy = alpha * avgErrorEnergy + (1.0 - alpha) * errorEnergy;
    
    // Store for next iteration
    lastInput = input;
    lastError = error;
    
    return output;
}

void ImprovedNotchLMSFilter::reset() {
    // Reset LMS filter
    lmsFilter.reset();
    
    // Reset all notch filters
    for (auto& notch : notchBank) {
        notch.setFrequency(notch.getFrequency());  // This triggers a coefficient update
    }
    
    // Reset state variables
    lastInput = 0.0;
    lastError = 0.0;
    avgInputEnergy = 0.0;
    avgErrorEnergy = 0.0;
}

void ImprovedNotchLMSFilter::selectStrategy(const double inputEnergy, const double errorEnergy) {
    // Simplified adaptive strategy selection based on signal characteristics
    
    // Calculate signal-to-error ratio (SER)
    const double ser = (inputEnergy > 1e-10) ? (inputEnergy / (errorEnergy + 1e-10)) : 1.0;
    
    // Select strategy based on SER
    if (ser > 10.0) {
        // High SER indicates good performance, use parallel for stability
        currentStrategy = Strategy::PARALLEL;
    } else if (ser < 2.0) {
        // Low SER indicates poor performance, try notch first
        currentStrategy = Strategy::NOTCH_FIRST;
    } else {
        // Moderate SER, use LMS first
        currentStrategy = Strategy::LMS_FIRST;
    }
}

void ImprovedNotchLMSFilter::updateNotchBank() {
    // Get detected feedback frequencies
    const auto& frequencies = feedbackDetector.getDetectedFrequencies();
    
    // No detected frequencies, keep current bank
    if (frequencies.empty()) {
        return;
    }
    
    // Limit to top MAX_NOTCH_FILTERS frequencies
    const size_t numFrequencies = std::min(frequencies.size(), MAX_NOTCH_FILTERS);
    
    // Resize the notch bank if needed
    if (notchBank.size() < numFrequencies) {
        // Add new notch filters
        for (size_t i = notchBank.size(); i < numFrequencies; ++i) {
            notchBank.emplace_back(frequencies[i], 10.0);
        }
    } else if (notchBank.size() > numFrequencies) {
        // Remove excess notch filters
        notchBank.resize(numFrequencies);
    }
    
    // Update each notch filter's frequency and Q factor
    for (size_t i = 0; i < numFrequencies; ++i) {
        // Get the detected frequency and ensure it's within bounds
        double frequency = std::max(MIN_FREQUENCY, std::min(MAX_FREQUENCY, frequencies[i]));
        
        // Calculate appropriate Q factor based on frequency
        // Higher frequencies get higher Q values for narrower notches
        double qFactor = MIN_Q_FACTOR + (MAX_Q_FACTOR - MIN_Q_FACTOR) * 
                        (frequency - MIN_FREQUENCY) / (MAX_FREQUENCY - MIN_FREQUENCY);
        
        // Update the notch filter
        notchBank[i].setFrequency(frequency);
        notchBank[i].setQ(qFactor);
    }
}

// Implementation of AdaptiveNotchFilter

ImprovedNotchLMSFilter::AdaptiveNotchFilter::AdaptiveNotchFilter(const double initialFreq, const double initialQ)
    : centerFreq(initialFreq), qFactor(initialQ) {
    updateCoefficients();
}

double ImprovedNotchLMSFilter::AdaptiveNotchFilter::process(const double input) {
    // Direct Form II implementation
    const double w = input - a1 * y1 - a2 * y2;
    const double output = b0 * w + b1 * x1 + b2 * x2;
    
    // Update state variables
    x2 = x1;
    x1 = w;
    y2 = y1;
    y1 = output;
    
    return output;
}

void ImprovedNotchLMSFilter::AdaptiveNotchFilter::setFrequency(const double freq) {
    centerFreq = freq;
    updateCoefficients();
}

void ImprovedNotchLMSFilter::AdaptiveNotchFilter::setQ(const double q) {
    qFactor = q;
    updateCoefficients();
}

void ImprovedNotchLMSFilter::AdaptiveNotchFilter::updateCoefficients() {
    // Calculate normalized frequency
    const double w0 = 2.0 * M_PI * centerFreq / AUDIO_SAMPLE_RATE_EXACT;
    const double cosw0 = cos(w0);
    const double alpha = sin(w0) / (2.0 * qFactor);
    
    // Calculate biquad notch filter coefficients
    b0 = 1.0;
    b1 = -2.0 * cosw0;
    b2 = 1.0;
    
    const double a0 = 1.0 + alpha;
    a1 = -2.0 * cosw0 / a0;
    a2 = (1.0 - alpha) / a0;
    
    // Normalize feed-forward coefficients
    b0 /= a0;
    b1 /= a0;
    b2 /= a0;
}

// Implementation of FFTFeedbackDetector

ImprovedNotchLMSFilter::FFTFeedbackDetector::FFTFeedbackDetector(const size_t bufferSize)
    : buffer(bufferSize, 0.0) {
}

void ImprovedNotchLMSFilter::FFTFeedbackDetector::addSample(const double sample) {
    // Add sample to buffer
    buffer[bufferIndex] = sample;
    bufferIndex = (bufferIndex + 1) % buffer.size();
    
    // If buffer is full, perform FFT analysis
    if (bufferIndex == 0) {
        performFFTAnalysis();
    }
}

void ImprovedNotchLMSFilter::FFTFeedbackDetector::performFFTAnalysis() {
    // Simplified FFT analysis using autocorrelation method
    // (Similar to the estimateDominantFrequency method in your NotchLMSFilter class)
    
    const size_t bufferSize = buffer.size();
    const size_t maxLag = bufferSize / 2;
    std::vector<double> autocorr(maxLag, 0.0);
    
    // Calculate signal energy
    signalEnergy = 0.0;
    for (const double sample : buffer) {
        signalEnergy += sample * sample;
    }
    signalEnergy /= bufferSize;
    
    // Calculate autocorrelation
    for (size_t lag = 0; lag < maxLag; ++lag) {
        for (size_t i = 0; i < bufferSize - lag; ++i) {
            autocorr[lag] += buffer[i] * buffer[i + lag];
        }
        autocorr[lag] /= (bufferSize - lag);
    }
    
    // Find peaks in autocorrelation (potential feedback frequencies)
    std::vector<std::pair<double, double>> peaks;  // (frequency, magnitude)
    
    for (size_t lag = maxLag / 8; lag < maxLag - 1; ++lag) {
        if (autocorr[lag] > autocorr[lag-1] && autocorr[lag] > autocorr[lag+1] && 
            autocorr[lag] > 0.1 * autocorr[0]) {  // 10% threshold
            
            // Calculate frequency from lag
            const double frequency = AUDIO_SAMPLE_RATE_EXACT / lag;
            
            // Only consider frequencies in the audible range
            if (frequency >= 100.0 && frequency <= 8000.0) {
                peaks.emplace_back(frequency, autocorr[lag]);
            }
        }
    }
    
    // Sort peaks by magnitude (descending)
    std::sort(peaks.begin(), peaks.end(), 
              [](const auto& a, const auto& b) { return a.second > b.second; });
    
    // Extract top frequencies
    detectedFrequencies.clear();
    for (const auto& peak : peaks) {
        detectedFrequencies.push_back(peak.first);
        
        // Limit to 5 strongest frequencies
        if (detectedFrequencies.size() >= 5) {
            break;
        }
    }
}

#endif