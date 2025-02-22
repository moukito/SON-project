#include "AdaptiveFeedbackCanceller.h"

#define AUDIO_OUTPUTS 1
#define MULT_16 32767

AdaptiveFeedbackCanceller::AdaptiveFeedbackCanceller()
	: AudioStream(AUDIO_OUTPUTS, new audio_block_t*[AUDIO_OUTPUTS]) {}

AdaptiveFeedbackCanceller::~AdaptiveFeedbackCanceller() = default;

void AdaptiveFeedbackCanceller::setGain(const double gain) {
	this->gain = gain;
}

void AdaptiveFeedbackCanceller::changeMode() {
	mode = !mode;
}

void AdaptiveFeedbackCanceller::update() {
	audio_block_t* inBlock = receiveReadOnly(0);
	if (!inBlock) return;

	audio_block_t* outBlock[AUDIO_OUTPUTS];
	for (int channel = 0; channel < AUDIO_OUTPUTS; channel++) {
		outBlock[channel] = allocate();
		if (!outBlock[channel]) {
			release(inBlock);
			return;
		}

		for (int i = 0; i < AUDIO_BLOCK_SAMPLES; i++) {
			double currentSample{static_cast<double>(inBlock->data[i]) / static_cast<double>(MULT_16)};

			//currentSample          =  notchFilter.tick(currentSample);
			currentSample          =  lmsFilter.tick(currentSample);
			//currentSample *= gain;  // Correction: *= gain au lieu de *= gain * gain
			currentSample = max(-1.0, min(1.0, currentSample));
			outBlock[channel]->data[i] = static_cast<int16_t>(currentSample * MULT_16);
		}

		transmit(outBlock[channel], channel);
		release(outBlock[channel]);
	}
	release(inBlock);
}