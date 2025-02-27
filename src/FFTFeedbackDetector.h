#ifndef FFT_FEEDBACK_DETECTOR_H
#define FFT_FEEDBACK_DETECTOR_H

#include <vector>
#include <algorithm>
#include <Audio.h>

/**
 * @brief The FFTFeedbackDetector class detects feedback using FFT analysis.
 *
 * This class provides functionality to detect feedback in an audio signal
 * by analyzing its frequency spectrum.
 */
class FFTFeedbackDetector {
public:
	/**
	 * @brief Constructs an FFTFeedbackDetector with specified buffer size.
	 *
	 * @param bufferSize The size of the buffer for FFT analysis.
	 */
	explicit FFTFeedbackDetector(size_t bufferSize = 256);

	/**
	 * @brief Adds a sample to the buffer and performs FFT analysis when the buffer is full.
	 *
	 * @param sample The sample to add to the buffer.
	 */
	void addSample(double sample);

	/**
	 * @brief Gets the list of detected feedback frequencies.
	 *
	 * @return A vector of detected feedback frequencies.
	 */
	[[nodiscard]] const std::vector<double>& getDetectedFrequencies() const { return detectedFrequencies; }

	/**
	 * @brief Gets the energy of the signal in the buffer.
	 *
	 * @return The energy of the signal.
	 */
	[[nodiscard]] double getSignalEnergy() const { return signalEnergy; }

private:
	/**
	 * @brief Performs FFT analysis on the buffer data.
	 */
	void performFFTAnalysis();

	std::vector<double> buffer;               ///< Buffer for storing samples.
	size_t bufferIndex{0};                    ///< Current index in the buffer.
	std::vector<double> detectedFrequencies;  ///< List of detected feedback frequencies.
	double signalEnergy{0.0};                 ///< Energy of the signal in the buffer.
};

#endif