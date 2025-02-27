#include <Arduino.h>
#include <Audio.h>
#include "AdaptiveFeedbackCanceller.h"
#include <cmath>

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

/**
 * @brief Processes a serial command and performs the corresponding action.
 *
 * @param command The serial command to process.
 */
void processSerialCommand(const String &command) {
    if (command.startsWith("SET:GAIN:")) {
        const double gain = command.substring(9).toFloat();
        adaptiveFeedbackCanceller.setGain(gain);
        Serial.print("DATA:GAIN:");
        Serial.println(gain);
    }
    else if (command == "SET:LMS:ON") {
        adaptiveFeedbackCanceller.setLMS(true);
        Serial.println("DATA:LMS:ON");
    }
    else if (command == "SET:LMS:OFF") {
        adaptiveFeedbackCanceller.setLMS(false);
        Serial.println("DATA:LMS:OFF");
    }
    else if (command == "SET:NOTCH:ON") {
        adaptiveFeedbackCanceller.setNotch(true);
        Serial.println("DATA:NOTCH:ON");
    }
    else if (command == "SET:NOTCH:OFF") {
        adaptiveFeedbackCanceller.setNotch(false);
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

/**
 * @brief Initializes the audio system and serial communication.
 */
void setup() {
    Serial.begin(115200);
#ifdef BUTTON
    pinMode(buttonPin, INPUT);
#endif
    AudioMemory(20);
    audioShield.enable();
    audioShield.inputSelect(AUDIO_INPUT_MIC);
    audioShield.micGain(10);
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

/**
 * @brief Main loop that processes serial commands and updates the audio system.
 */
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
            if (const float binValue = fft1024.read(i); binValue > maxVal) {
                maxVal = binValue;
                maxBin = i;
            }
        }

        const auto dominantFreq = static_cast<float>(maxBin) * AUDIO_SAMPLE_RATE_EXACT / 1024.0f;

        Serial.print("DATA:FREQ:");
        Serial.print(dominantFreq);
        Serial.print(",");
        Serial.println(maxVal);
    }

    delay(100);
}