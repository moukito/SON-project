#ifndef LMS_FILTER_H
#define LMS_FILTER_H

#include <cstddef>
#include <algorithm>

#define NLMS
#define ADAPTIVE_GAMMA
#define KALMAN
#define DYNAMIC_NOISE

#if defined(KALMAN) || defined(DYNAMIC_NOISE)
#ifndef ADAPTIVE_GAMMA
#define ADAPTIVE_GAMMA
#endif
#endif

#ifdef DYNAMIC_NOISE
#ifndef KALMAN
#define KALMAN
#endif
#endif

/**
 * @brief The LMSFilter class implements an adaptive LMS filter.
 *
 * This class provides methods to apply an adaptive LMS filter to an input signal,
 * allowing for noise reduction and adaptive filtering.
 */
class LMSFilter final {
public:
    /**
     * @brief Constructs an LMSFilter object with the specified order and adaptation rate.
     *
     * @param order The order of the filter.
     * @param mu The adaptation rate (default is 0.0001).
     */
    explicit LMSFilter(std::size_t order, double mu = 0.0001);

    /**
     * @brief Destroys the LMSFilter object.
     */
    ~LMSFilter();

    /**
     * @brief Processes an input sample and returns the filtered output.
     *
     * @param micSample The input sample to be filtered.
     * @return The filtered output sample.
     */
    double tick(double micSample);

    /**
     * @brief Resets the LMS filter.
     */
    void reset();

    /**
     * @brief Sets the adaptation rate (mu) for the LMS filter.
     *
     * @param new_mu The new adaptation rate.
     */
    void setMu(double new_mu);

    /**
     * @brief Gets the current adaptation rate (mu) of the LMS filter.
     *
     * @return The current adaptation rate.
     */
    [[nodiscard]] double getMu() const { return mu; }

#ifdef LEAKAGE
    /**
     * @brief Sets the leakage factor for the LMS filter.
     *
     * @param newLeakage The new leakage factor.
     */
    void setLeakage(const double newLeakage) { leakage = newLeakage; }

    /**
     * @brief Gets the current leakage factor of the LMS filter.
     *
     * @return The current leakage factor.
     */
    [[nodiscard]] double getLeakage() const { return leakage; }
#endif

private:
    std::size_t order; ///< The order of the filter.
    double mu; ///< The adaptation rate.
    double* reference_buffer; ///< Buffer for reference signal.
    double* weights; ///< Weights of the filter.
    std::size_t index{0}; ///< Current index in the buffer.

#ifdef NLMS
    double power{0.0}; ///< Power of the input signal.
#endif

#ifdef LEAKAGE
    double leakage{0.001}; ///< Leakage factor.
#else
    double leakage{1.0}; ///< Default leakage factor.
#endif

#ifdef ADAPTIVE_GAMMA
    double signalVarianceEstimate{0.0}; ///< Estimate of the signal variance.
    double errorVarianceEstimate{0.0}; ///< Estimate of the error variance.

    double muMin{0.00001}; ///< Minimum adaptation rate.
    double muMax{0.01}; ///< Maximum adaptation rate.
    double gammaMin{0.990}; ///< Minimum gamma value.
    double gammaMax{0.9999}; ///< Maximum gamma value.

#ifdef KALMAN
    double signalVarianceError{1.0}; ///< Error in signal variance estimate.
    double signalProcessNoise{0.01}; ///< Process noise for signal variance.
    double signalMeasurementNoise{0.1}; ///< Measurement noise for signal variance.

    double errorVarianceError{1.0}; ///< Error in error variance estimate.
    double errorProcessNoise{0.01}; ///< Process noise for error variance.
    double errorMeasurementNoise{0.1}; ///< Measurement noise for error variance.
#else
    double alpha{0.95}; ///< Alpha value for non-Kalman adaptive gamma.
#endif

#ifdef DYNAMIC_NOISE
    static constexpr int ESTIMATION_WINDOW = 50; ///< Window size for noise estimation.
    double signalValues[ESTIMATION_WINDOW]{}; ///< Buffer for signal values.
    double errorValues[ESTIMATION_WINDOW]{}; ///< Buffer for error values.
    int windowIndex = 0; ///< Current index in the estimation window.
    bool windowFilled = false; ///< Flag indicating if the window is filled.

    /**
     * @brief Updates the noise parameters based on the error signal.
     *
     * @param error The error signal.
     */
    void updateNoiseParameters(double error);
#endif
#endif

    bool noiseReduction{false}; ///< Flag indicating if noise reduction is enabled.
};

#endif