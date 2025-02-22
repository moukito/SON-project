#include "LMSFilter.h"

LMSFilter::LMSFilter(const std::size_t order, const double mu, const double epsilon)
    : order(order), mu(mu), epsilon(epsilon) {
    reference_buffer = new double[order];
    weights = new double[order];

    for (std::size_t i = 0; i < order; ++i) {
        reference_buffer[i] = 0.0;
        weights[i] = 0.0;
    }
}

LMSFilter::~LMSFilter() {
    delete[] reference_buffer;
    delete[] weights;
}

double LMSFilter::tick(const double micSample) {
    // Calculer l'estimation du feedback
    double estimation = 0.0;
    for (std::size_t i = 0; i < order; ++i) {
        estimation += weights[i] * reference_buffer[(index + i) % order];
    }

    // Calculer l'erreur
    const double error = micSample - estimation;

    // Calculer la puissance du signal de référence
    double power = 0.0;
    for (std::size_t i = 0; i < order; ++i) {
        const double sample = reference_buffer[(index + i) % order];
        power += sample * sample;
    }

    // Normaliser le pas d'adaptation
    const double mu_eff = (power > 0.0) ? mu / (power + epsilon) : 0.0;

    // Mettre à jour les poids
    for (std::size_t i = 0; i < order; ++i) {
        weights[i] += mu_eff * error * reference_buffer[(index + i) % order];
    }

    // Mettre à jour le buffer de référence avec l'erreur (signal de sortie)
    reference_buffer[index] = error;
    index = (index + 1) % order;

    return error;
}