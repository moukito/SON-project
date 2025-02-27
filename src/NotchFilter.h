#ifndef NOTCH_FILTER_H
#define NOTCH_FILTER_H

#include <Audio.h>
#include <cmath>

/**
 * @brief The NotchFilter class implements a notch filter for audio processing.
 *
 * This class provides methods to apply a notch filter to an input signal,
 * allowing for the attenuation of a specific frequency band.
 */
class NotchFilter {
public:
    /**
     * @brief Constructs a NotchFilter object with the specified frequency and bandwidth.
     *
     * @param frequency The center frequency of the notch filter.
     * @param bandwidth The bandwidth of the notch filter.
     */
    NotchFilter(double frequency, double bandwidth);

    /**
     * @brief Processes an input sample and returns the filtered output.
     *
     * @param x0 The input sample to be filtered.
     * @return The filtered output sample.
     */
    double tick(double x0);

    /**
     * @brief Computes the filter coefficients based on the current frequency and bandwidth.
     */
    void computeCoefficient();

    /**
     * @brief Computes the radius (r) based on the given bandwidth.
     *
     * @param bandwidth The bandwidth of the notch filter.
     * @return The computed radius (r).
     */
    static double computeR(double bandwidth);

    /**
     * @brief Sets the center frequency of the notch filter.
     *
     * @param frequency The new center frequency.
     */
    void setFrequency(const double frequency) { this->frequency = frequency; }

    /**
     * @brief Gets the current center frequency of the notch filter.
     *
     * @return The current center frequency.
     */
    [[nodiscard]] double getCenterFrequency() const { return frequency; }

    /**
     * @brief Sets the bandwidth of the notch filter.
     *
     * @param bandwidth The new bandwidth.
     */
    void setBandwidth(const double bandwidth) { r = computeR(bandwidth); }

    /**
     * @brief Gets the current bandwidth of the notch filter.
     *
     * @return The current bandwidth.
     */
    [[nodiscard]] double getBandwidth() const { return -log(r) * AUDIO_SAMPLE_RATE_EXACT / M_PI; }

private:
    double frequency; ///< The center frequency of the notch filter.
    double r; ///< The radius (r) of the filter.

    double x1{0}, x2{0}; ///< Previous input samples.
    double y1{0}, y2{0}; ///< Previous output samples.

    double w0{}; ///< Angular frequency.

    double a1{}, a2{}; ///< Filter coefficients.
    double b0{1.0f}, b1{}, b2{1.0f}; ///< Filter coefficients.
};

#endif