#ifndef ADAPTIVE_FEEDBACK_CANCELLER_H
#define ADAPTIVE_FEEDBACK_CANCELLER_H

#include "Audio.h"
#include "NotchLMSFilter.h"

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
    [[nodiscard]] bool isLMSEnabled() const { return notchLMSFilter.isLMSEnabled(); }
    [[nodiscard]] bool isNotchEnabled() const { return notchLMSFilter.isNotchEnabled(); }
    void setMute(bool muted);
    [[nodiscard]] bool isMuted() const { return muted; }

private:
    NotchLMSFilter notchLMSFilter{64, 2750, 100};
    double gain{1.0};
    bool mode{false};

    bool muted{false};
};

#endif