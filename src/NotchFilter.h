#ifndef NOTCH_FILTER_H
#define NOTCH_FILTER_H

#include <Audio.h>
#include <cmath>

class NotchFilter {
public:
	NotchFilter(double frequency, double bandwidth);
	double tick(double x0);

	void computeCoefficient();
	static double computeR(double bandwidth);

	void setFrequency(const double frequency) { this->frequency = frequency; }
	[[nodiscard]] double getCenterFrequency() const { return frequency; }

	void setBandwidth(const double bandwidth) { r = computeR(bandwidth); }
	[[nodiscard]] double getBandwidth() const { return -log(r) * AUDIO_SAMPLE_RATE_EXACT / M_PI; }

private:
	double frequency;
	double r;

	double x1{0}, x2{0};
	double y1{0}, y2{0};

	double w0{};

	double a1{}, a2{};
	double b0{1.0f}, b1{}, b2{1.0f};
};



#endif //NOTCH_FILTER_H