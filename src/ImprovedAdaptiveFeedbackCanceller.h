#ifndef IMPROVED_ADAPTIVE_FEEDBACK_CANCELLER_H
#define IMPROVED_ADAPTIVE_FEEDBACK_CANCELLER_H

#include "Audio.h"
#include "ImprovedNotchLMSFilter.h"

/**
 * @brief The ImprovedAdaptiveFeedbackCanceller class implements an improved adaptive feedback canceller for audio processing.
 *
 * This class provides methods to apply adaptive feedback cancellation using the ImprovedNotchLMSFilter,
 * which combines multiple notch filters with an LMS filter using different processing strategies.
 */
class ImprovedAdaptiveFeedbackCanceller final : public AudioStream {
public:
    /**
     * @brief Constructs an ImprovedAdaptiveFeedbackCanceller object.
     */
    ImprovedAdaptiveFeedbackCanceller();

    /**
     * @brief Destroys the ImprovedAdaptiveFeedbackCanceller object.
     */
    ~ImprovedAdaptiveFeedbackCanceller();

    /**
     * @brief Updates the audio stream with the processed output.
     */
    void update() override;

    /**
     * @brief Sets the gain for the feedback canceller.
     *
     * @param gain The new gain value.
     */
    void setGain(double gain);

    /**
     * @brief Changes the mode of the feedback canceller.
     */
    void changeMode();

    /**
     * @brief Resets the filter.
     */
    void reset();

    /**
     * @brief Enables or disables the LMS filter.
     *
     * @param enabled True to enable the LMS filter, false to disable it.
     */
    void setLMS(bool enabled);

    /**
     * @brief Enables or disables the notch filter.
     *
     * @param enabled True to enable the notch filter, false to disable it.
     */
    void setNotch(bool enabled);

    /**
     * @brief Sets the strategy for combining notch and LMS filters.
     *
     * @param strategyIndex Index of the strategy to use (0-3).
     */
    void setStrategy(int strategyIndex);

    /**
     * @brief Checks if the LMS filter is enabled.
     *
     * @return True if the LMS filter is enabled, false otherwise.
     */
    [[nodiscard]] bool isLMSEnabled() const { return improvedFilter.isLMSEnabled(); }

    /**
     * @brief Checks if the notch filter is enabled.
     *
     * @return True if the notch filter is enabled, false otherwise.
     */
    [[nodiscard]] bool isNotchEnabled() const { return improvedFilter.isNotchEnabled(); }

    /**
     * @brief Gets the current strategy index.
     *
     * @return The current strategy index (0-3).
     */
    [[nodiscard]] int getStrategy() const { return static_cast<int>(improvedFilter.getStrategy()); }

    /**
     * @brief Mutes or unmutes the feedback canceller.
     *
     * @param muted True to mute the feedback canceller, false to unmute it.
     */
    void setMute(bool muted);

    /**
     * @brief Checks if the feedback canceller is muted.
     *
     * @return True if the feedback canceller is muted, false otherwise.
     */
    [[nodiscard]] bool isMuted() const { return muted; }

private:
    ImprovedNotchLMSFilter improvedFilter{64, 2750}; ///< The improved notch+LMS filter used for feedback cancellation.
    double gain{1.0}; ///< The gain of the feedback canceller.
    bool mode{false}; ///< The mode of the feedback canceller.
    bool muted{false}; ///< Indicates if the feedback canceller is muted.
};

// Implementation

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

#endif