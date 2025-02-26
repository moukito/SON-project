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


class LMSFilter final {
public:
	explicit LMSFilter(std::size_t order, double mu = 0.0001);
	~LMSFilter();
	double tick(double micSample);
	void reset();

	void setMu(double new_mu);
	[[nodiscard]] double getMu() const { return mu; }

#ifdef LEAKAGE
	void setLeakage(const double newLeakage) { leakage = newLeakage; }
	[[nodiscard]] double getLeakage() const { return leakage; }
#endif

private:
	std::size_t order;
	double mu;
	double* reference_buffer;
	double* weights;
	std::size_t index{0};

#ifdef NLMS
	double power{0.0};
#endif

#ifdef LEAKAGE
	double leakage{0.001};
#else
	double leakage{1.0};
#endif

#ifdef ADAPTIVE_GAMMA
	double signalVarianceEstimate{0.0};
	double errorVarianceEstimate{0.0};

	double muMin{0.00001};
	double muMax{0.01};
	double gammaMin{0.990};
	double gammaMax{0.9999};

#ifdef KALMAN
	double signalVarianceError{1.0};
	double signalProcessNoise{0.01};
	double signalMeasurementNoise{0.1};

	double errorVarianceError{1.0};
	double errorProcessNoise{0.01};
	double errorMeasurementNoise{0.1};
#else
	double alpha{0.95};
#endif

#ifdef DYNAMIC_NOISE
	static constexpr int ESTIMATION_WINDOW = 50;
	double signalValues[ESTIMATION_WINDOW]{};
	double errorValues[ESTIMATION_WINDOW]{};
	int windowIndex = 0;
	bool windowFilled = false;

	void updateNoiseParameters(double error);
#endif
#endif

	bool noiseReduction{false};
};

#endif