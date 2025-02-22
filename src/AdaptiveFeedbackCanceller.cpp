#include "AdaptiveFeedbackCanceller.h"
#include <arm_math.h>

// Constantes pour la conversion int16_t vers float32_t
#define MULT_16 32767

// Définition des paramètres du filtre NLMS
#define BLOCK_SIZE AUDIO_BLOCK_SAMPLES  // Taille du bloc audio (défini par la Teensy Audio Library)
#define NUM_TAPS   32                   // Nombre de coefficients du filtre

// Buffers statiques pour le filtre NLMS
static float32_t filterState[NUM_TAPS + BLOCK_SIZE - 1]; // État du filtre
static float32_t filterCoeffs[NUM_TAPS];                 // Coefficients adaptatifs
static float32_t previousOutput[BLOCK_SIZE];             // Signal de sortie précédent (référence)
static bool isInitialized = false;                       // Indicateur d'initialisation
static arm_lms_norm_instance_f32 nlmsInstance;           // Instance du filtre NLMS

// Constructeur
AdaptiveFeedbackCanceller::AdaptiveFeedbackCanceller()
    : AudioStream(1, new audio_block_t*[1])
{
    // Initialisation de previousOutput à zéro
    for (int i = 0; i < BLOCK_SIZE; i++) {
        previousOutput[i] = 0.0f;
    }
}

// Destructeur
AdaptiveFeedbackCanceller::~AdaptiveFeedbackCanceller() = default;

// Méthode principale de traitement audio
void AdaptiveFeedbackCanceller::update() {
    // Récupérer le bloc d'entrée (signal du microphone)
    audio_block_t* inBlock = receiveReadOnly(0);
    if (!inBlock) return;

    // Allouer un bloc de sortie
    audio_block_t* outBlock = allocate();
    if (!outBlock) {
        release(inBlock);
        return;
    }

    // Buffers temporaires pour le traitement
    float32_t micSignal[BLOCK_SIZE];    // Signal du microphone
    float32_t refSignal[BLOCK_SIZE];    // Signal de référence (sortie précédente)
    float32_t feedbackEst[BLOCK_SIZE];  // Estimation du feedback
    float32_t errorSignal[BLOCK_SIZE];  // Signal désiré (erreur)

    // Convertir le signal d'entrée (int16_t -> float32_t, normalisé en [-1, 1])
    for (int i = 0; i < BLOCK_SIZE; i++) {
        micSignal[i] = inBlock->data[i] / (float32_t)MULT_16;
    }

    // Copier le signal de sortie précédent dans refSignal
    for (int i = 0; i < BLOCK_SIZE; i++) {
        refSignal[i] = previousOutput[i];
    }

    // Initialiser le filtre NLMS une seule fois
    if (!isInitialized) {
        // Réinitialiser les coefficients
        for (int i = 0; i < NUM_TAPS; i++) {
            filterCoeffs[i] = 0.0f;
        }
        // Initialiser l'instance NLMS avec un pas d'adaptation de 0.1
        arm_lms_norm_init_f32(&nlmsInstance, NUM_TAPS, filterCoeffs, filterState, 0.1f, BLOCK_SIZE);
        isInitialized = true;
    }

    // Appliquer le filtre NLMS
    // - refSignal (pSrc) : signal de référence (sortie précédente)
    // - micSignal (pRef) : signal du microphone
    // - feedbackEst (pOut) : estimation du feedback
    // - errorSignal (pErr) : signal désiré
    arm_lms_norm_f32(&nlmsInstance, refSignal, micSignal, feedbackEst, errorSignal, BLOCK_SIZE);

    // Utiliser errorSignal comme signal de sortie
    for (int i = 0; i < BLOCK_SIZE; i++) {
        float32_t sample = errorSignal[i];
        // Limiter les valeurs pour éviter le clipping
        if (sample > 1.0f) sample = 1.0f;
        if (sample < -1.0f) sample = -1.0f;
        outBlock->data[i] = (int16_t)(sample * MULT_16);
        // Mettre à jour previousOutput avec le signal envoyé au haut-parleur
        previousOutput[i] = errorSignal[i];
    }

    // Debug : afficher les premières valeurs toutes les 100 itérations
    static int counter = 0;
    if (counter % 100 == 0) {
        Serial.print("Mic[0]: ");
        Serial.print(micSignal[0], 6);
        Serial.print(" | Ref[0]: ");
        Serial.print(refSignal[0], 6);
        Serial.print(" | Feedback[0]: ");
        Serial.print(feedbackEst[0], 6);
        Serial.print(" | Error[0]: ");
        Serial.println(errorSignal[0], 6);
    }
    counter++;

    // Transmettre le bloc de sortie et libérer la mémoire
    transmit(outBlock, 0);
    release(outBlock);
    release(inBlock);
}