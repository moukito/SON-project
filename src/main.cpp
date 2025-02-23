#include <Arduino.h>
#include <Audio.h>
#include "AdaptiveFeedbackCanceller.h"
#include <math.h>

AdaptiveFeedbackCanceller adaptiveFeedbackCanceller;
AudioInputI2S in;
AudioOutputI2S out;
AudioControlSGTL5000 audioShield;
AudioAnalyzeFFT1024 fft1024;

AudioConnection patchCord0(in,0,adaptiveFeedbackCanceller,0);
AudioConnection patchCord1(adaptiveFeedbackCanceller,0,out,0);
AudioConnection patchCord2(adaptiveFeedbackCanceller,0,out,1);
AudioConnection patchCord3(adaptiveFeedbackCanceller,0,fft1024,0);

constexpr uint8_t buttonPin{0};
int buttonState = HIGH;
int lastButtonState = LOW;
unsigned long lastDebounceTime = 0;
constexpr unsigned long debounceDelay = 50;
bool changedState = false;

void setup() {
	Serial.begin(9600);
	pinMode(buttonPin, INPUT);
	AudioMemory(20);
	audioShield.enable();
	audioShield.inputSelect(AUDIO_INPUT_MIC);
	audioShield.micGain(20);
	audioShield.volume(0.8);
	Serial.println("Microphone ready...");
}

void loop() {
	const auto reading = digitalRead(buttonPin);

	if (reading != lastButtonState) {
		if (changedState && ((millis() - lastDebounceTime) > debounceDelay)) {
			changedState = false;
		}
		lastDebounceTime = millis();
	}

	if ((reading == buttonState) && ((millis() - lastDebounceTime) > debounceDelay)) {
		if (!changedState) {
			changedState = true;
			adaptiveFeedbackCanceller.changeMode();
		}
	}

	lastButtonState = reading;
	//const auto potentiometerValue{analogRead(0) / 256};
	//adaptiveFeedbackCanceller.setGain(potentiometerValue);

	if (fft1024.available()) {
		float maxVal = 0.0f;
		int maxBin = 0;
		// La FFT1024 fournit 512 bins utiles (le reste est symétrique)
		for (int i = 0; i < 512; i++) {
			float binValue = fft1024.read(i);
			if (binValue > maxVal) {
				maxVal = binValue;
				maxBin = i;
			}
		}
		// Calcul de la résolution en fréquence
		float Fs = AUDIO_SAMPLE_RATE_EXACT; // par exemple, 44100 ou 48000 Hz
		float freqResolution = Fs / 1024.0f;
		float dominantFreq = maxBin * freqResolution;

		Serial.print("Fréquence dominante (howling?) : ");
		Serial.print(dominantFreq);
		Serial.println(" Hz");
	}

	delay(100);
}