#include <Arduino.h>
#include <Audio.h>
#include "AdaptiveFeedbackCanceller.h"

// Instanciation de la classe AdaptiveFeedbackCanceller
// Dans cette version, le filtre Notch et le filtre LMS (CMSIS‑DSP NLMS) sont testés.
// La classe récupère le signal du microphone, applique le traitement NLMS, puis renvoie le résultat.
AdaptiveFeedbackCanceller adaptiveFeedbackCanceller;

// Création des objets pour l'entrée (micro) et la sortie (casque)
AudioInputI2S  micInput;
AudioOutputI2S audioOutput;
AudioControlSGTL5000 audioShield;

// Connexions audio :
// Le signal du micro est envoyé à notre module de traitement (adaptiveFeedbackCanceller)
// La sortie de ce module est ensuite routée vers les deux canaux de l'interface I2S (casque/stéréo)
AudioConnection patchCord0(micInput, 0, adaptiveFeedbackCanceller, 0);
AudioConnection patchCord1(adaptiveFeedbackCanceller, 0, audioOutput, 0);
AudioConnection patchCord2(adaptiveFeedbackCanceller, 0, audioOutput, 1);

void setup() {
	Serial.begin(115200);
	while (!Serial) {}  // Attente pour la connexion série, si nécessaire

	// Allocation de mémoire audio
	AudioMemory(12);

	// Initialisation de l'audioShield (SGTL5000)
	audioShield.enable();
	audioShield.inputSelect(AUDIO_INPUT_MIC);
	audioShield.micGain(10);
	audioShield.volume(0.5);

	Serial.println("Adaptive LMS Test using CMSIS-DSP NLMS filter is running...");
}

void loop() {
	// Le traitement s'effectue en temps réel dans la méthode update() de AdaptiveFeedbackCanceller.
	// Ici, nous n'avons rien à faire dans loop().
	delay(100);
}
