#ifndef ADAPTIVE_FEEDBACK_CANCELLER_H
#define ADAPTIVE_FEEDBACK_CANCELLER_H

#include "Arduino.h"
#include "AudioStream.h"
#include "Audio.h"

class AdaptiveFeedbackCanceller final : public AudioStream {
	public:
		AdaptiveFeedbackCanceller();

		virtual ~AdaptiveFeedbackCanceller();

		void update() override;
};

#endif //ADAPTIVE_FEEDBACK_CANCELLER_H