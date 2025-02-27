#include "AdaptiveNotchFilter.h"

AdaptiveNotchFilter::AdaptiveNotchFilter(const double initialFreq, const double initialQ)
	: centerFreq(initialFreq), qFactor(initialQ) {
	updateCoefficients();
}

double AdaptiveNotchFilter::process(const double input) {
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

void AdaptiveNotchFilter::setFrequency(const double freq) {
	centerFreq = freq;
	updateCoefficients();
}

void AdaptiveNotchFilter::setQ(const double q) {
	qFactor = q;
	updateCoefficients();
}

void AdaptiveNotchFilter::updateCoefficients() {
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