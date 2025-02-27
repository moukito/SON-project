#include "AdaptiveFeedbackCanceller.h"

#define MULT_16 32767
constexpr unsigned char audioOutputs{1};
constexpr unsigned int channel{0};

/**
 * @brief Constructs an AdaptiveFeedbackCanceller object.
 */
AdaptiveFeedbackCanceller::AdaptiveFeedbackCanceller()
    : AudioStream(audioOutputs, new audio_block_t*[audioOutputs]) {}

/**
 * @brief Destroys the AdaptiveFeedbackCanceller object.
 */
AdaptiveFeedbackCanceller::~AdaptiveFeedbackCanceller() = default;

/**
 * @brief Sets the gain for the feedback canceller.
 *
 * @param gain The new gain value.
 */
void AdaptiveFeedbackCanceller::setGain(const double gain) {
    this->gain = gain;
}

/**
 * @brief Changes the mode of the feedback canceller.
 */
void AdaptiveFeedbackCanceller::changeMode() {
    mode = !mode;
}

/**
 * @brief Resets the LMS filter.
 */
void AdaptiveFeedbackCanceller::resetLMS() {
    notchLMSFilter.LMSReset();
}

/**
 * @brief Enables or disables the LMS filter.
 *
 * @param enabled True to enable the LMS filter, false to disable it.
 */
void AdaptiveFeedbackCanceller::setLMS(const bool enabled) {
    notchLMSFilter.enableLMS(enabled);
}

/**
 * @brief Enables or disables the notch filter.
 *
 * @param enabled True to enable the notch filter, false to disable it.
 */
void AdaptiveFeedbackCanceller::setNotch(const bool enabled) {
    notchLMSFilter.enableNotch(enabled);
}

/**
 * @brief Mutes or unmutes the feedback canceller.
 *
 * @param muted True to mute the feedback canceller, false to unmute it.
 */
void AdaptiveFeedbackCanceller::setMute(const bool muted) {
    this->muted = muted;
}

/**
 * @brief Updates the audio stream with the processed output.
 */
void AdaptiveFeedbackCanceller::update() {
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
            currentSample = notchLMSFilter.tick(currentSample);
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