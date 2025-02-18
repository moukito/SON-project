#include <Arduino.h>
#include <Audio.h>

AudioInputI2S in; // replace mydsp and correspond to the microphone
AudioOutputI2S out;
AudioControlSGTL5000 audioShield;
AudioConnection patchCord0(in,0,out,0);
AudioConnection patchCord1(in,0,out,1);

void setup() {
    Serial.begin(9600);
    AudioMemory(6);
    audioShield.enable();
    audioShield.inputSelect(AUDIO_INPUT_MIC);
    audioShield.micGain(10); // in dB
    audioShield.volume(0.3);
}

void loop() {
    delay(100);
}