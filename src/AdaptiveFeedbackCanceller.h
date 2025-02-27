#ifndef ADAPTIVE_FEEDBACK_CANCELLER_H
#define ADAPTIVE_FEEDBACK_CANCELLER_H

#include "Audio.h"
#include "NotchLMSFilter.h"

/**
 * @brief The AdaptiveFeedbackCanceller class implements an adaptive feedback canceller for audio processing.
 *
 * This class provides methods to apply adaptive feedback cancellation using a combination of notch and LMS filters.
 */
class AdaptiveFeedbackCanceller final : public AudioStream {
public:
    /**
     * @brief Constructs an AdaptiveFeedbackCanceller object.
     */
    AdaptiveFeedbackCanceller();

    /**
     * @brief Destroys the AdaptiveFeedbackCanceller object.
     */
    virtual ~AdaptiveFeedbackCanceller();

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
     * @brief Resets the LMS filter.
     */
    void resetLMS();

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
     * @brief Checks if the LMS filter is enabled.
     *
     * @return True if the LMS filter is enabled, false otherwise.
     */
    [[nodiscard]] bool isLMSEnabled() const { return notchLMSFilter.isLMSEnabled(); }

    /**
     * @brief Checks if the notch filter is enabled.
     *
     * @return True if the notch filter is enabled, false otherwise.
     */
    [[nodiscard]] bool isNotchEnabled() const { return notchLMSFilter.isNotchEnabled(); }

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
    NotchLMSFilter notchLMSFilter{64, 2750, 100}; ///< The notch and LMS filter used for feedback cancellation.
    double gain{1.0}; ///< The gain of the feedback canceller.
    bool mode{false}; ///< The mode of the feedback canceller.

    bool muted{false}; ///< Indicates if the feedback canceller is muted.
};

#endif