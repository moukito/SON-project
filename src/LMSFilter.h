#ifndef LMS_H
#define LMS_H

#include <vector>

class LMSFilter {
public:
	LMSFilter(size_t order, float mu);
	~LMSFilter() = default;

	void updateBuffer(float new_sample);

	// Compute the filter output
	double computeFilterOutput() const;

	// Get the oldest sample from the delay buffer
	double getDelayedReference() const;

	// Update the filter weights
	void updateWeights(double desired);

	// One iteration of the LMS filter
	double process(float input);

private:
	size_t index{0};
	size_t order;
    double* input_buffer;
    double* delay_buffer;
    std::vector<float> weights; // Filter weights
	float mu; // Step size
};

#endif