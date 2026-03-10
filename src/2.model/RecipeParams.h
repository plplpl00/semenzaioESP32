// ============================================================
//  RecipeParams.h
//  Posizione: src/2.model/RecipeParams.h
//
//  Parametri della ricetta letti da RTDB.
//  Compilati dalla Cloud Function per il giorno corrente.
// ============================================================
#pragma once
#include <stdint.h>

// ─────────────────────────────────────────────────────────────
//  ClimateParams — soglie per un periodo (giorno o notte)
// ─────────────────────────────────────────────────────────────
struct ClimateParams {
    float   tempThreshold  = 20.0f;
    float   tempMaxAlarm   = 26.0f;
    float   humThreshold   = 70.0f;
    float   humMaxAlarm    = 88.0f;
    uint8_t speedMin       = 20;
    uint8_t speedMax       = 80;
};

// ─────────────────────────────────────────────────────────────
//  IrrigationParams — parametri irrigazione del giorno
// ─────────────────────────────────────────────────────────────
struct IrrigationParams {
    uint8_t  count          = 0;
    uint16_t durationSec    = 0;
    // Orari irrigazione (max 4 irrigazioni al giorno)
    uint8_t  hours[4]       = {};
    uint8_t  minutes[4]     = {};
};

// ─────────────────────────────────────────────────────────────
//  SafetyParams — parametri di sicurezza globali
// ─────────────────────────────────────────────────────────────
struct SafetyParams {
    float   externalTempMax  = 33.0f;
    float   deltaMinForVent  = 1.0f;
    uint8_t offlineSpeedMin  = 15;
    bool    offlineLightOff  = true;
};

// ─────────────────────────────────────────────────────────────
//  RecipeParams — tutti i parametri del giorno corrente
// ─────────────────────────────────────────────────────────────
struct RecipeParams {
    bool            active       = false;
    char            recipeName[32] = "";
    uint8_t         dayOfCycle   = 0;
    uint8_t         lightOnHour  = 6;
    uint8_t         lightOffHour = 22;

    ClimateParams   day;
    ClimateParams   night;
    IrrigationParams irrigations;
    SafetyParams    safety;

    // Determina se siamo nel fotoperiodo in base all'ora
    bool isDaytime(uint8_t currentHour) const {
        if (lightOnHour < lightOffHour) {
            return currentHour >= lightOnHour && currentHour < lightOffHour;
        } else {
            // Crossing mezzanotte (es. 22:00 → 06:00)
            return currentHour >= lightOnHour || currentHour < lightOffHour;
        }
    }

    // Ritorna i parametri attivi in base all'ora
    const ClimateParams& currentClimate(uint8_t currentHour) const {
        return isDaytime(currentHour) ? day : night;
    }
};
