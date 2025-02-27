#include "FFTFeedbackDetector.h"

FFTFeedbackDetector::FFTFeedbackDetector(const size_t bufferSize)
    : buffer(bufferSize, 0.0) {
}

void FFTFeedbackDetector::addSample(const double sample) {
    // Add sample to buffer
    buffer[bufferIndex] = sample;
    bufferIndex = (bufferIndex + 1) % buffer.size();
    
    // If buffer is full, perform FFT analysis
    if (bufferIndex == 0) {
        performFFTAnalysis();
    }
}

void FFTFeedbackDetector::performFFTAnalysis() {
    // Simplified FFT analysis using autocorrelation method
    
    const size_t bufferSize = buffer.size();
    const size_t maxLag = bufferSize / 2;
    std::vector<double> autocorr(maxLag, 0.0);
    
    // Calculate signal energy
    signalEnergy = 0.0;
    for (const double sample : buffer) {
        signalEnergy += sample * sample;
    }
    signalEnergy /= bufferSize;
    
    // Calculate autocorrelation
    for (size_t lag = 0; lag < maxLag; ++lag) {
        for (size_t i = 0; i < bufferSize - lag; ++i) {
            autocorr[lag] += buffer[i] * buffer[i + lag];
        }
        autocorr[lag] /= (bufferSize - lag);
    }
    
    // Find peaks in autocorrelation (potential feedback frequencies)
    std::vector<std::pair<double, double>> peaks;  // (frequency, magnitude)
    
    for (size_t lag = maxLag / 8; lag < maxLag - 1; ++lag) {
        if (autocorr[lag] > autocorr[lag-1] && autocorr[lag] > autocorr[lag+1] && 
            autocorr[lag] > 0.1 * autocorr[0]) {  // 10% threshold
            
            // Calculate frequency from lag
            const double frequency = AUDIO_SAMPLE_RATE_EXACT / lag;
            
            // Only consider frequencies in the audible range
            if (frequency >= 100.0 && frequency <= 8000.0) {
                peaks.emplace_back(frequency, autocorr[lag]);
            }
        }
    }
    
    // Sort peaks by magnitude (descending)
    std::sort(peaks.begin(), peaks.end(), 
              [](const auto& a, const auto& b) { return a.second > b.second; });
    
    // Extract top frequencies
    detectedFrequencies.clear();
    for (const auto& peak : peaks) {
        detectedFrequencies.push_back(peak.first);
        
        // Limit to 5 strongest frequencies
        if (detectedFrequencies.size() >= 5) {
            break;
        }
    }
}