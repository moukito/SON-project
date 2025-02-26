#include "NotchFilter.h"

NotchFilter::NotchFilter(const double frequency, const double bandwidth) : frequency(frequency), r(computeR(bandwidth)) {
	w0 = 2.0f * M_PI * frequency / AUDIO_SAMPLE_RATE_EXACT;
	b1 = -2.0f * cos(w0);
	a1 = -2.0f * r * cos(w0);
	a2 = r * r;
}

double NotchFilter::computeR(const double bandwidth) {
	return exp(-(M_PI*bandwidth)/AUDIO_SAMPLE_RATE_EXACT);
}

double NotchFilter::tick(const double x0) {
	const double y0 = b0 * x0 + b1 * x1 + b2 * x2 - a1 * y1 - a2 * y2;

	x2 = x1;
	x1 = x0;

	y2 = y1;
	y1 = y0;

	return y0;
}