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

private:
	NotchFilter notchFilter{1000.0, 100.0};
	LMSFilter lmsFilter{32, 0.01};
	double gain{1.0};
	bool mode{false};
};

#endif