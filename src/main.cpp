#include <Arduino.h>
#include <Audio.h>
#include <AdaptiveFeedbackCanceller.h>

AdaptiveFeedbackCanceller adaptiveFeedbackCanceller;
AudioInputI2S in;
AudioOutputI2S out;
AudioControlSGTL5000 audioShield;

AudioConnection patchCord0(in,0,adaptiveFeedbackCanceller,0);
AudioConnection patchCord1(adaptiveFeedbackCanceller,0,out,0);
AudioConnection patchCord2(adaptiveFeedbackCanceller,0,out,1);

void setup() {
    Serial.begin(9600);
    AudioMemory(6);
    audioShield.enable();
    audioShield.inputSelect(AUDIO_INPUT_MIC);
    audioShield.micGain(10);
    audioShield.volume(0.5);
    Serial.println("Microphone ready...");
}

void loop() {
    delay(100);
}