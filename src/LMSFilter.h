#ifndef LMS_FILTER_H
#define LMS_FILTER_H

#include <cstddef>

#define NLMS

class LMSFilter final {
public:
	LMSFilter(std::size_t order, double mu);
	~LMSFilter();
	double tick(double micSample);
	void reset(); // Nouvelle fonction

private:
	std::size_t order;
	double mu;
#ifdef NLMS
	double power{0.0};
#endif
	double* reference_buffer;
	double* weights;
	std::size_t index{0};
};

#endif