#ifndef ADAPTIVE_FEEDBACK_CANCELLER_H
#define ADAPTIVE_FEEDBACK_CANCELLER_H

#include "Audio.h"
#include "LMSFilter.h"
#include "NotchFilter.h"

class AdaptiveFeedbackCanceller final : public AudioStream {
public:
    AdaptiveFeedbackCanceller();
    virtual ~AdaptiveFeedbackCanceller();
    void update() override;
    void setGain(double gain);
    void changeMode();

    void resetLMS();
    void setLMS(bool enabled);
    void setNotch(bool enabled);
    [[nodiscard]] bool isLMSEnabled() const { return lmsEnabled; }
    [[nodiscard]] bool isNotchEnabled() const { return notchEnabled; }
    void setMute(bool muted);
    [[nodiscard]] bool isMuted() const { return muted; }

private:
    NotchFilter notchFilter{2750.0, 500.0};
    LMSFilter lmsFilter{64, 0.0001};
    double gain{1.0};
    bool mode{false};

    bool lmsEnabled{true};
    bool notchEnabled{false};
    bool muted{false};
};

#endif