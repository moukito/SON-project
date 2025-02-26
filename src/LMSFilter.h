#ifndef LMS_FILTER_H
#define LMS_FILTER_H

#include <cstddef>

#define NLMS

class LMSFilter final {
public:
	LMSFilter(std::size_t order, double mu);
	~LMSFilter();
	double tick(double micSample);
	void reset();

private:
	std::size_t order;
	double mu;
	double leakage{0.001};
#ifdef NLMS
	double power{0.0};
#endif
	double* reference_buffer;
	double* weights;
	std::size_t index{0};
};

#endif