#ifndef LMS_FILTER_H
#define LMS_FILTER_H

#include <cstddef>

#define NLMS

class LMSFilter final {
public:
	LMSFilter(std::size_t order, double mu);
	~LMSFilter();
	double tick(double micSample);

private:
	std::size_t order;
	double mu;
	double* reference_buffer;
	double* weights;
	std::size_t index{0};
};

#endif