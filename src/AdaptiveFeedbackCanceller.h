#ifndef ADAPTIVE_FEEDBACK_CANCELLER_H
#define ADAPTIVE_FEEDBACK_CANCELLER_H

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
	NotchFilter notchFilter{1500, 500};
	LMSFilter lmsFilter{32, 0.001};
	double gain{1.0f};
	bool mode{false};
};

#endif //ADAPTIVE_FEEDBACK_CANCELLER_H