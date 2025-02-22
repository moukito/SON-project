#ifndef LMS_FILTER_H
#define LMS_FILTER_H

#include <cstddef>

class LMSFilter final {
public:
	LMSFilter(std::size_t order, double mu, double epsilon);
	~LMSFilter();
	double tick(double micSample);

private:
	std::size_t order;
	double mu;
	double epsilon;          // Ajout de epsilon pour la normalisation
	double* reference_buffer; // Renommé de input_buffer à reference_buffer
	double* weights;
	std::size_t index{0};
};

#endif