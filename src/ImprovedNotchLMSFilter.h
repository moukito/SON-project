#ifndef IMPROVED_NOTCH_LMS_FILTER_H
#define IMPROVED_NOTCH_LMS_FILTER_H

#include "AdaptiveNotchFilter.h"
#include "FFTFeedbackDetector.h"
#include <Audio.h>
#include <vector>
#include <cmath>
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

#endif