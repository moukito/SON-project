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

#ifdef BUTTON
constexpr uint8_t buttonPin{0};
int buttonState = HIGH;
int lastButtonState = LOW;
unsigned long lastDebounceTime = 0;
constexpr unsigned long debounceDelay = 50;
bool changedState = false;
#endif

void processSerialCommand(const String &command) {
    if (command.startsWith("SET:GAIN:")) {
        double gain = command.substring(9).toFloat();
        adaptiveFeedbackCanceller.setGain(gain);
        Serial.print("DATA:GAIN:");
        Serial.println(gain);
    }
    else if (command == "SET:LMS:ON") {
        adaptiveFeedbackCanceller.setLMSEnabled(true);
        Serial.println("DATA:LMS:ON");
    }
    else if (command == "SET:LMS:OFF") {
        adaptiveFeedbackCanceller.setLMSEnabled(false);
        Serial.println("DATA:LMS:OFF");
    }
    else if (command == "SET:NOTCH:ON") {
        adaptiveFeedbackCanceller.setNotchEnabled(true);
        Serial.println("DATA:NOTCH:ON");
    }
    else if (command == "SET:NOTCH:OFF") {
        adaptiveFeedbackCanceller.setNotchEnabled(false);
        Serial.println("DATA:NOTCH:OFF");
    }
    else if (command == "SET:MUTE:ON") {
        adaptiveFeedbackCanceller.setMute(true);
        Serial.println("DATA:MUTE:ON");
    }
    else if (command == "SET:MUTE:OFF") {
        adaptiveFeedbackCanceller.setMute(false);
        Serial.println("DATA:MUTE:OFF");
    }
    else if (command == "RESET:LMS") {
        adaptiveFeedbackCanceller.resetLMS();
        Serial.println("DATA:LMS:RESET");
    }
    else if (command == "GET:STATUS") {
        Serial.print("DATA:STATUS:");
        Serial.print(adaptiveFeedbackCanceller.isLMSEnabled() ? "LMS:ON," : "LMS:OFF,");
        Serial.print(adaptiveFeedbackCanceller.isNotchEnabled() ? "NOTCH:ON," : "NOTCH:OFF,");
        Serial.print(adaptiveFeedbackCanceller.isMuted() ? "MUTE:ON" : "MUTE:OFF");
        Serial.println();
    }
}

void setup() {
    Serial.begin(115200);
#ifdef BUTTON
    pinMode(buttonPin, INPUT);
#endif
    AudioMemory(20);
    audioShield.enable();
    audioShield.inputSelect(AUDIO_INPUT_MIC);
    audioShield.micGain(20);
    audioShield.volume(0.8);

    Serial.println("DATA:INIT:Système initialisé");
    Serial.print("DATA:STATUS:");
    Serial.print(adaptiveFeedbackCanceller.isLMSEnabled() ? "LMS:ON," : "LMS:OFF,");
    Serial.print(adaptiveFeedbackCanceller.isNotchEnabled() ? "NOTCH:ON," : "NOTCH:OFF,");
    Serial.print(adaptiveFeedbackCanceller.isMuted() ? "MUTE:ON" : "MUTE:OFF");
    Serial.println();

    Serial.print("DATA:MODE:");
    Serial.println("INACTIF");
}

void loop() {
    if (Serial.available() > 0) {
        String command = Serial.readStringUntil('\n');
        command.trim();
        processSerialCommand(command);
    }

#ifdef BUTTON
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
            Serial.print("DATA:MODE:");
            Serial.println(reading == LOW ? "ACTIF" : "INACTIF");
        }
    }

    lastButtonState = reading;
#endif

#ifdef POTENTIOMETER
    const auto potentiometerValue{analogRead(0) / 256};
    adaptiveFeedbackCanceller.setGain(potentiometerValue);
#endif

    if (fft1024.available()) {
        float maxVal = 0.0f;
        int maxBin = 0;
        for (int i = 0; i < 512; i++) {
            float binValue = fft1024.read(i);
            if (binValue > maxVal) {
                maxVal = binValue;
                maxBin = i;
            }
        }

        float Fs = AUDIO_SAMPLE_RATE_EXACT;
        float freqResolution = Fs / 1024.0f;
        float dominantFreq = maxBin * freqResolution;

        Serial.print("DATA:FREQ:");
        Serial.print(dominantFreq);
        Serial.print(",");
        Serial.println(maxVal);
    }

    delay(100);
}