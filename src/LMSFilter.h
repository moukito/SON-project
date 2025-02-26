#ifndef LMS_FILTER_H
#define LMS_FILTER_H

#include <cstddef>

#define NLMS
#define ADAPTATIVE_GAMMA

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

	double signalVariance{0.0};
	double errorVariance{0.0};
	double alpha{0.95};         // Facteur de lissage pour l'estimation de la variance

	// Paramètres pour l'adaptation
	double muMin{0.00001};      // Valeur minimale de mu
	double muMax{0.01};         // Valeur maximale de mu
	double gammaMin{0.990};     // Valeur minimale de gamma
	double gammaMax{0.9999};    // Valeur maximale de gamma
};

#endif