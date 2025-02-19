#include "MyDsp.h"

#define AUDIO_OUTPUTS 1

#define MULT_16 32767

MyDsp::MyDsp() :
AudioStream(AUDIO_OUTPUTS, new audio_block_t*[AUDIO_OUTPUTS]){}

MyDsp::~MyDsp() = default;

void MyDsp::update() {
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
			float currentSample = static_cast<float>(inBlock->data[i]) / static_cast<float>(MULT_16);
			currentSample = max(-1,min(1,currentSample));
			outBlock[channel]->data[i] = static_cast<int16_t>(currentSample * MULT_16);
		}

		transmit(outBlock[channel], channel);
		release(outBlock[channel]);
		release(inBlock);
	}
}