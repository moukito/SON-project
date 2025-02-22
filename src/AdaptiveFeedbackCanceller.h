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
	NotchFilter notchFilter{1000.0, 100.0};  // Exemple: fréquence 1000 Hz, largeur 100 Hz
	LMSFilter lmsFilter{32, 0.01, 1e-6};     // Exemple: order 32, mu 0.01, epsilon 1e-6
	double gain{1.0};
	bool mode{false};
};

#endif