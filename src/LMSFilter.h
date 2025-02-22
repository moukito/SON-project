#ifndef LMS_H
#define LMS_H

#include <vector>

class LMSFilter {
public:
	LMSFilter(std::size_t order, double mu);
	virtual ~LMSFilter();
	[[nodiscard]] double computeFilterOutput() const;
	void updateWeights(double desired, double output) const;
	double tick(double input);

private:
	std::size_t index{0};
	std::size_t order;
    double* input_buffer;
    double* weights;
	double mu;
};

#endif