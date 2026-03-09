// ============================================================
//  SystemConfig.h
//  Posizione: src/config/SystemConfig.h
//
//  Configurazione statica del semenzaio — valori di default
//  modificabili a runtime da RTDBCommands.
//
//  Dipendenze: nessuna (solo tipi primitivi)
// ============================================================
#pragma once

#include <stdint.h>

// ─────────────────────────────────────────────────────────────
//  EnvironmentConfig
// ─────────────────────────────────────────────────────────────
struct EnvironmentConfig {
    uint32_t readInterval     = 2000;   // ms tra letture sensori
    uint8_t  movingAvgSamples = 5;      // campioni media mobile
    float    maxTempDelta     = 3.0f;   // °C max differenza DS18B20/SHT31
};

// ─────────────────────────────────────────────────────────────
//  VentilationConfig
// ─────────────────────────────────────────────────────────────
struct VentilationConfig {
    // Soglie temperatura
    float   tempIdealMax    = 18.0f;  // °C — sotto: velocità minima
    float   tempWarnMax     = 21.0f;  // °C — sopra: velocità normale
    float   tempCritical    = 24.0f;  // °C — sopra: velocità massima

    // Soglie umidità
    float   humidityIdealMax  = 75.0f;  // % — sotto: velocità minima
    float   humidityCritical  = 88.0f;  // % — sopra: velocità massima

    // Velocità (0-100%)
    uint8_t speedMin          = 20;
    uint8_t speedNormal       = 35;
    uint8_t speedHigh         = 65;
    uint8_t speedMax          = 100;

    // Stallo
    uint16_t stallRpmThreshold = 100;   // RPM sotto cui è considerato stallo

    // Timer aggiornamento in AUTO
    uint32_t updateInterval    = 5000;  // ms
};

// ─────────────────────────────────────────────────────────────
//  LightConfig
// ─────────────────────────────────────────────────────────────
struct LightConfig {
    uint8_t  onHour   = 6;    // ora accensione (0-23)
    uint8_t  offHour  = 22;   // ora spegnimento (0-23)
};

// ─────────────────────────────────────────────────────────────
//  FirebaseConfig
// ─────────────────────────────────────────────────────────────
struct FirebaseSettings {
    uint32_t pushInterval = 60000;  // ms tra push su Firestore
};

// ─────────────────────────────────────────────────────────────
//  SystemConfig
//  Aggregatore — unica istanza in main.cpp
// ─────────────────────────────────────────────────────────────
struct SystemConfig {
    EnvironmentConfig environment;
    VentilationConfig ventilation;
    LightConfig       light;
    FirebaseSettings    firebase;
};