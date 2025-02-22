#ifndef LMS_H
#define LMS_H

#include <cstddef>

class LMSFilter {
public:
	LMSFilter(const std::size_t order, const double mu, const double epsilon);
	~LMSFilter();
	double tick(const double micSample);

private:
	std::size_t order;
	double mu;
	double epsilon;          // Ajout de epsilon pour la normalisation
	double* reference_buffer; // Renommé de input_buffer à reference_buffer
	double* weights;
	std::size_t index{0};
};

#endif