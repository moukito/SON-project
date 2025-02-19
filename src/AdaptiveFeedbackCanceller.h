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

private:
	NotchFilter notchFilter{1000, 0.9};
};

#endif //ADAPTIVE_FEEDBACK_CANCELLER_H