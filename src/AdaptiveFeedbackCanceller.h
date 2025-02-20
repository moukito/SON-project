#ifndef ADAPTIVE_FEEDBACK_CANCELLER_H
#define ADAPTIVE_FEEDBACK_CANCELLER_H

#include "Arduino.h"
#include "AudioStream.h"
#include "Audio.h"
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
	double gain{1.0f};
	bool mode{false};
};

#endif //ADAPTIVE_FEEDBACK_CANCELLER_H