#ifndef AdaptiveFeedbackCanceller_H
#define AdaptiveFeedbackCanceller_H

#include "Arduino.h"
#include "AudioStream.h"
#include "Audio.h"

class AdaptiveFeedbackCanceller final : public AudioStream {
	public:
		AdaptiveFeedbackCanceller();

		virtual ~AdaptiveFeedbackCanceller();

		void update() override;
};

#endif //AdaptiveFeedbackCanceller_H