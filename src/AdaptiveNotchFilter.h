#ifndef ADAPTIVE_NOTCH_FILTER_H
#define ADAPTIVE_NOTCH_FILTER_H

#include <Audio.h>
#include <cmath>
#include <vector>

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
    AdaptiveNotchFilter(double initialFreq = 1000.0, double initialQ = 10.0);
    
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
    
    double centerFreq;                ///< Center frequency of the notch filter.
    double qFactor;                   ///< Q factor of the notch filter.
};

#endif