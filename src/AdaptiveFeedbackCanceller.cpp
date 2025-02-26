#include "AdaptiveFeedbackCanceller.h"

#define MULT_16 32767
constexpr unsigned char audioOutputs{1};
constexpr unsigned int channel{0};

AdaptiveFeedbackCanceller::AdaptiveFeedbackCanceller()
    : AudioStream(audioOutputs, new audio_block_t*[audioOutputs]) {}

AdaptiveFeedbackCanceller::~AdaptiveFeedbackCanceller() = default;

void AdaptiveFeedbackCanceller::setGain(const double gain) {
    this->gain = gain;
}

void AdaptiveFeedbackCanceller::changeMode() {
    mode = !mode;
}

void AdaptiveFeedbackCanceller::resetLMS() {
    lmsFilter.reset();
}

void AdaptiveFeedbackCanceller::setLMSEnabled(bool enabled) {
    lmsEnabled = enabled;
}

void AdaptiveFeedbackCanceller::setNotchEnabled(bool enabled) {
    notchEnabled = enabled;
}

void AdaptiveFeedbackCanceller::setMute(bool muted) {
    this->muted = muted;
}

void AdaptiveFeedbackCanceller::update() {
    audio_block_t* inBlock{receiveReadOnly(0)};
    if (!inBlock) return;

    audio_block_t* outBlock{allocate()};
    if (!outBlock) {
        release(inBlock);
        return;
    }

    double samples[AUDIO_BLOCK_SAMPLES];
    for (int i = 0; i < AUDIO_BLOCK_SAMPLES; i++) {
        samples[i] = {static_cast<double>(inBlock->data[i]) / static_cast<double>(MULT_16)};
    }

    for (int i = 0; i < AUDIO_BLOCK_SAMPLES; i++) {
        auto& currentSample = samples[i];

        if (!mode) {
            if (notchEnabled) {
                currentSample = notchFilter.tick(currentSample);
            }

            if (lmsEnabled) {
                currentSample = lmsFilter.tick(currentSample);
            }

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