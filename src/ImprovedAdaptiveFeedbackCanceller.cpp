#include "ImprovedAdaptiveFeedbackCanceller.h"

#define MULT_16 32767
constexpr unsigned char audioOutputs{1};
constexpr unsigned int channel{0};

ImprovedAdaptiveFeedbackCanceller::ImprovedAdaptiveFeedbackCanceller()
	: AudioStream(audioOutputs, new audio_block_t*[audioOutputs]) {}

ImprovedAdaptiveFeedbackCanceller::~ImprovedAdaptiveFeedbackCanceller() = default;

void ImprovedAdaptiveFeedbackCanceller::setGain(const double gain) {
	this->gain = gain;
}

void ImprovedAdaptiveFeedbackCanceller::changeMode() {
	mode = !mode;
}

void ImprovedAdaptiveFeedbackCanceller::reset() {
	improvedFilter.reset();
}

void ImprovedAdaptiveFeedbackCanceller::setLMS(const bool enabled) {
	improvedFilter.enableLMS(enabled);
}

void ImprovedAdaptiveFeedbackCanceller::setNotch(const bool enabled) {
	improvedFilter.enableNotch(enabled);
}

void ImprovedAdaptiveFeedbackCanceller::setStrategy(const int strategyIndex) {
	if (strategyIndex >= 0 && strategyIndex <= 3) {
		improvedFilter.setStrategy(static_cast<ImprovedNotchLMSFilter::Strategy>(strategyIndex));
	}
}

void ImprovedAdaptiveFeedbackCanceller::setMute(const bool muted) {
	this->muted = muted;
}

void ImprovedAdaptiveFeedbackCanceller::update() {
	audio_block_t* inBlock{receiveReadOnly(0)};
	if (!inBlock) return;

	audio_block_t* outBlock{allocate()};
	if (!outBlock) {
		release(inBlock);
		return;
	}

	for (int i = 0; i < AUDIO_BLOCK_SAMPLES; i++) {
		auto currentSample{static_cast<double>(inBlock->data[i]) / static_cast<double>(MULT_16)};

		if (!mode) {
			currentSample = improvedFilter.process(currentSample);
			currentSample *= gain;
		}

		if (muted) {
			currentSample = 0.0;
		}

		currentSample = max(-1.0, min(1.0, currentSample));
		outBlock->data[i] = static_cast<int16_t>(currentSample * MULT_16);
	}

	transmit(outBlock, channel);
	release(outBlock);
	release(inBlock);
}