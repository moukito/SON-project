#include "NotchFilter.h"

/**
 * @brief Constructs a NotchFilter object with the specified frequency and bandwidth.
 *
 * @param frequency The center frequency of the notch filter.
 * @param bandwidth The bandwidth of the notch filter.
 */
NotchFilter::NotchFilter(const double frequency, const double bandwidth) : frequency(frequency), r(computeR(bandwidth)) {
	computeCoefficient();
}

/**
 * @brief Computes the filter coefficients based on the current frequency and bandwidth.
 */
void NotchFilter::computeCoefficient() {
	w0 = 2.0f * M_PI * frequency / AUDIO_SAMPLE_RATE_EXACT;
	b1 = -2.0f * cos(w0);
	a1 = -2.0f * r * cos(w0);
	a2 = r * r;
}

/**
 * @brief Computes the radius (r) based on the given bandwidth.
 *
 * @param bandwidth The bandwidth of the notch filter.
 * @return The computed radius (r).
 */
double NotchFilter::computeR(const double bandwidth) {
	return exp(-(M_PI * bandwidth) / AUDIO_SAMPLE_RATE_EXACT);
}

/**
 * @brief Processes an input sample and returns the filtered output.
 *
 * @param x0 The input sample to be filtered.
 * @return The filtered output sample.
 */
double NotchFilter::tick(const double x0) {
	const double y0 = b0 * x0 + b1 * x1 + b2 * x2 - a1 * y1 - a2 * y2;

	x2 = x1;
	x1 = x0;

	y2 = y1;
	y1 = y0;

	return y0;
}