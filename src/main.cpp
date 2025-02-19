#include <Arduino.h>
#include <Audio.h>
#include <MyDsp.h>

MyDsp mydsp;
AudioInputI2S in;
AudioOutputI2S out;
AudioControlSGTL5000 audioShield;

AudioConnection patchCord0(mydsp,0,out,0);
AudioConnection patchCord1(mydsp,0,out,1);
AudioConnection patchCord2(in,0,mydsp,0);

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