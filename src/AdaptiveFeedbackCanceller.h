#ifndef ADAPTIVE_FEEDBACK_CANCELLER_H
#define ADAPTIVE_FEEDBACK_CANCELLER_H

// todo : see if we can erase arduino and audiostream to just let audio
#include "Arduino.h"
#include "Audio.h"
#include "AudioStream.h"
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
    void setLMSEnabled(bool enabled);
    void setNotchEnabled(bool enabled);
    bool isLMSEnabled() const { return lmsEnabled; }
    bool isNotchEnabled() const { return notchEnabled; }
    void setMute(bool muted);
    bool isMuted() const { return muted; }

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