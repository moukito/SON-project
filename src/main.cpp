#include <Arduino.h>
#include <Audio.h>
#include "AdaptiveFeedbackCanceller.h"

AdaptiveFeedbackCanceller adaptiveFeedbackCanceller;
AudioInputI2S in;
AudioOutputI2S out;
AudioControlSGTL5000 audioShield;

AudioConnection patchCord0(in,0,adaptiveFeedbackCanceller,0);
AudioConnection patchCord1(adaptiveFeedbackCanceller,0,out,0);
AudioConnection patchCord2(adaptiveFeedbackCanceller,0,out,1);

constexpr uint8_t buttonPin{0};
int buttonState = HIGH;
int lastButtonState = LOW;
unsigned long lastDebounceTime = 0;
constexpr unsigned long debounceDelay = 50;
bool changedState = false;

void setup() {
	Serial.begin(9600);
	pinMode(buttonPin, INPUT);
	AudioMemory(6);
	audioShield.enable();
	audioShield.inputSelect(AUDIO_INPUT_MIC);
	audioShield.micGain(10);
	audioShield.volume(0.5);
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

	const auto potentiometerValue{analogRead(0) / 256};
	adaptiveFeedbackCanceller.setGain(potentiometerValue);
	delay(100);
}