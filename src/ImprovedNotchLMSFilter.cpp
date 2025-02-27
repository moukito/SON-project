#include "ImprovedNotchLMSFilter.h"

ImprovedNotchLMSFilter::ImprovedNotchLMSFilter(const size_t lmsOrder, const double initialFreq)
    : lmsFilter(lmsOrder, 0.001),
      feedbackDetector(512) {
    // Initialize notch bank with one filter at the initial frequency
    notchBank.emplace_back(initialFreq, 10.0);
}

double ImprovedNotchLMSFilter::process(double input) {
    // Add sample to feedback detector for analysis
    feedbackDetector.addSample(input);
    
    // Update notch bank based on detected feedback frequencies
    updateNotchBank();
    
    double notchOutput = input;
    double lmsOutput = input;
    double output = input;
    
    // Process the input based on the current strategy and enabled filters
    if (currentStrategy == Strategy::ADAPTIVE) {
        // Calculate input energy for this sample
        const double inputEnergy = input * input;
        avgInputEnergy = alpha * avgInputEnergy + (1.0 - alpha) * inputEnergy;
        
        // Select the best strategy based on signal characteristics
        selectStrategy(avgInputEnergy, avgErrorEnergy);
    }
    
    switch (currentStrategy) {
        case Strategy::NOTCH_FIRST:
            if (notchEnabled) {
                // Apply all notch filters in series
                for (auto& notch : notchBank) {
                    notchOutput = notch.process(notchOutput);
                }
                output = notchOutput;
            }
            
            if (lmsEnabled) {
                // Apply LMS filter after notch filtering
                output = lmsFilter.tick(output);
            }
            break;
            
        case Strategy::LMS_FIRST:
            if (lmsEnabled) {
                // Apply LMS filter first
                lmsOutput = lmsFilter.tick(input);
                output = lmsOutput;
            }
            
            if (notchEnabled) {
                // Apply all notch filters in series after LMS filtering
                for (auto& notch : notchBank) {
                    output = notch.process(output);
                }
            }
            break;
            
        case Strategy::PARALLEL:
            if (notchEnabled) {
                // Apply all notch filters in series
                for (auto& notch : notchBank) {
                    notchOutput = notch.process(notchOutput);
                }
            } else {
                notchOutput = input;
            }
            
            if (lmsEnabled) {
                // Apply LMS filter separately
                lmsOutput = lmsFilter.tick(input);
            } else {
                lmsOutput = input;
            }
            
            // Mix the outputs of both filters (50/50)
            output = 0.5 * notchOutput + 0.5 * lmsOutput;
            break;
            
        case Strategy::ADAPTIVE:
            // This case should be handled before the switch, but include as fallback
            if (notchEnabled) {
                for (auto& notch : notchBank) {
                    notchOutput = notch.process(notchOutput);
                }
                output = notchOutput;
            }
            
            if (lmsEnabled) {
                output = lmsFilter.tick(output);
            }
            break;
    }
    
    // Calculate error energy for adaptive strategy
    const double error = output - input;
    const double errorEnergy = error * error;
    avgErrorEnergy = alpha * avgErrorEnergy + (1.0 - alpha) * errorEnergy;
    
    // Store for next iteration
    lastInput = input;
    lastError = error;
    
    return output;
}

void ImprovedNotchLMSFilter::reset() {
    // Reset LMS filter
    lmsFilter.reset();
    
    // Reset all notch filters
    for (auto& notch : notchBank) {
        notch.setFrequency(notch.getFrequency());  // This triggers a coefficient update
    }
    
    // Reset state variables
    lastInput = 0.0;
    lastError = 0.0;
    avgInputEnergy = 0.0;
    avgErrorEnergy = 0.0;
}

void ImprovedNotchLMSFilter::selectStrategy(const double inputEnergy, const double errorEnergy) {
    // Simplified adaptive strategy selection based on signal characteristics
    
    // Calculate signal-to-error ratio (SER)
    const double ser = (inputEnergy > 1e-10) ? (inputEnergy / (errorEnergy + 1e-10)) : 1.0;
    
    // Select strategy based on SER
    if (ser > 10.0) {
        // High SER indicates good performance, use parallel for stability
        currentStrategy = Strategy::PARALLEL;
    } else if (ser < 2.0) {
        // Low SER indicates poor performance, try notch first
        currentStrategy = Strategy::NOTCH_FIRST;
    } else {
        // Moderate SER, use LMS first
        currentStrategy = Strategy::LMS_FIRST;
    }
}

void ImprovedNotchLMSFilter::updateNotchBank() {
    // Get detected feedback frequencies
    const auto& frequencies = feedbackDetector.getDetectedFrequencies();
    
    // No detected frequencies, keep current bank
    if (frequencies.empty()) {
        return;
    }
    
    // Limit to top MAX_NOTCH_FILTERS frequencies
    const size_t numFrequencies = std::min(frequencies.size(), MAX_NOTCH_FILTERS);
    
    // Resize the notch bank if needed
    if (notchBank.size() < numFrequencies) {
        // Add new notch filters
        for (size_t i = notchBank.size(); i < numFrequencies; ++i) {
            notchBank.emplace_back(frequencies[i], 10.0);
        }
    } else if (notchBank.size() > numFrequencies) {
        // Remove excess notch filters
        notchBank.resize(numFrequencies);
    }
    
    // Update each notch filter's frequency and Q factor
    for (size_t i = 0; i < numFrequencies; ++i) {
        // Get the detected frequency and ensure it's within bounds
        double frequency = std::max(MIN_FREQUENCY, std::min(MAX_FREQUENCY, frequencies[i]));
        
        // Calculate appropriate Q factor based on frequency
        // Higher frequencies get higher Q values for narrower notches
        double qFactor = MIN_Q_FACTOR + (MAX_Q_FACTOR - MIN_Q_FACTOR) * 
                        (frequency - MIN_FREQUENCY) / (MAX_FREQUENCY - MIN_FREQUENCY);
        
        // Update the notch filter
        notchBank[i].setFrequency(frequency);
        notchBank[i].setQ(qFactor);
    }
}