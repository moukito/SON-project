#ifndef NOTCH_LMS_FILTER_H
#define NOTCH_LMS_FILTER_H

#include "NotchFilter.h"
#include "LMSFilter.h"
#include <cstddef>

/**
 * @brief The NotchLMSFilter class combines a notch filter and an LMS filter.
 *
 * This class provides functionality to apply a notch filter and an LMS filter
 * to an input signal. It allows for adaptive filtering and provides methods
 * to configure the filters.
 */
class NotchLMSFilter final {
public:
    /**
     * @brief Constructs a NotchLMSFilter object.
     *
     * @param order The order of the filter.
     * @param initialCenterFreq The initial center frequency of the notch filter.
     * @param initialBandwidth The initial bandwidth of the notch filter.
     */
    NotchLMSFilter(std::size_t order, double initialCenterFreq, double initialBandwidth);

    /**
     * @brief Destroys the NotchLMSFilter object.
     */
    ~NotchLMSFilter();
    
    /**
     * @brief Processes an input sample and returns the filtered output.
     *
     * @param inputSample The input sample to be filtered.
     * @return The filtered output sample.
     */
    double tick(double inputSample);

    /**
     * @brief Sets the adaptation rate (mu) for the LMS filter.
     *
     * @param newMu The new adaptation rate.
     */
    void setMu(const double newMu) { lmsFilter.setMu(newMu); }

    /**
     * @brief Gets the current adaptation rate (mu) of the LMS filter.
     *
     * @return The current adaptation rate.
     */
    [[nodiscard]] double getMu() const { return lmsFilter.getMu(); }

#ifdef LEAKAGE
    /**
     * @brief Sets the leakage factor for the LMS filter.
     *
     * @param newLeakage The new leakage factor.
     */
    void setLeakage(double newLeakage) { lmsFilter.setLeakage(newLeakage); }

    /**
     * @brief Gets the current leakage factor of the LMS filter.
     *
     * @return The current leakage factor.
     */
    [[nodiscard]] double getLeakage() const { return lmsFilter.getLeakage(); }
#endif

    /**
     * @brief Sets the center frequency of the notch filter.
     *
     * @param frequency The new center frequency.
     */
    void setNotchFrequency(double frequency);

    /**
     * @brief Gets the current center frequency of the notch filter.
     *
     * @return The current center frequency.
     */
    [[nodiscard]] double getNotchFrequency() const { return notchFilter.getCenterFrequency(); }

    /**
     * @brief Sets the bandwidth of the notch filter.
     *
     * @param bandwidth The new bandwidth.
     */
    void setNotchBandwidth(double bandwidth);

    /**
     * @brief Gets the current bandwidth of the notch filter.
     *
     * @return The current bandwidth.
     */
    [[nodiscard]] double getNotchBandwidth() const { return notchFilter.getBandwidth(); }

    /**
     * @brief Enables or disables the notch filter.
     *
     * @param enable True to enable, false to disable.
     */
    void enableNotch(const bool enable) { notchEnabled = enable; }

    /**
     * @brief Checks if the notch filter is enabled.
     *
     * @return True if enabled, false otherwise.
     */
    [[nodiscard]] bool isNotchEnabled() const { return notchEnabled; }

    /**
     * @brief Enables or disables the LMS filter.
     *
     * @param enable True to enable, false to disable.
     */
    void enableLMS(const bool enable) { lmsEnabled = enable; }

    /**
     * @brief Checks if the LMS filter is enabled.
     *
     * @return True if enabled, false otherwise.
     */
    [[nodiscard]] bool isLMSEnabled() const { return lmsEnabled; }

    /**
     * @brief Enables or disables the adaptive notch filter.
     *
     * @param enable True to enable, false to disable.
     */
    void enableAdaptiveNotch(const bool enable) { adaptiveNotchEnabled = enable; }

    /**
     * @brief Checks if the adaptive notch filter is enabled.
     *
     * @return True if enabled, false otherwise.
     */
    [[nodiscard]] bool isAdaptiveNotchEnabled() const { return adaptiveNotchEnabled; }

    /**
     * @brief Sets the frequency limits for the adaptive notch filter.
     *
     * @param minFreq The minimum frequency limit.
     * @param maxFreq The maximum frequency limit.
     */
    void setFrequencyLimits(const double minFreq, const double maxFreq) {
        minFrequency = minFreq;
        maxFrequency = maxFreq;
    }

    /**
     * @brief Resets the LMS filter.
     */
    void LMSReset() { lmsFilter.reset(); }

private:
    NotchFilter notchFilter; ///< The notch filter instance.
    LMSFilter lmsFilter; ///< The LMS filter instance.

    bool notchEnabled{true}; ///< Flag indicating if the notch filter is enabled.
    bool lmsEnabled{true}; ///< Flag indicating if the LMS filter is enabled.
    bool adaptiveNotchEnabled{true}; ///< Flag indicating if the adaptive notch filter is enabled.

    double minFrequency{100.0}; ///< Minimum frequency limit for the adaptive notch filter.
    double maxFrequency{8000.0}; ///< Maximum frequency limit for the adaptive notch filter.

    double freqUpdateRate{0.01}; ///< Frequency update rate for the adaptive notch filter.

    static constexpr size_t SPECTRAL_BUFFER_SIZE = 128; ///< Size of the spectral buffer.
    double spectralBuffer[SPECTRAL_BUFFER_SIZE]{}; ///< Buffer for storing spectral data.
    size_t spectralBufferIndex{0}; ///< Current index in the spectral buffer.

    /**
     * @brief Updates the notch filter frequency based on the error and output.
     *
     * @param error The error signal.
     * @param output The output signal.
     */
    void updateNotchFrequency(double error, double output);

    /**
     * @brief Estimates the dominant frequency in the spectral buffer.
     *
     * @return The estimated dominant frequency.
     */
    [[nodiscard]] double estimateDominantFrequency() const;
};

#endif
