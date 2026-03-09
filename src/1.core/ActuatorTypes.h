// ============================================================
//  ActuatorTypes.h
//  Posizione: src/core/ActuatorTypes.h
//
//  Enumerazioni relative agli attuatori fisici.
//  ZERO dipendenze — questo file non include nulla.
// ============================================================
#pragma once

#include <stdint.h>

// ─────────────────────────────────────────────────────────────
//  VentilationCause
//  Perché la ventola sta girando a quella velocità.
//  Utile per trasparenza nell'app Flutter.
// ─────────────────────────────────────────────────────────────
enum class VentilationCause : uint8_t {
    MANUAL      = 0,  // velocità impostata a mano dall'utente
    TEMPERATURE = 1,  // temperatura troppo alta
    HUMIDITY    = 2,  // umidità troppo alta
    COMBINED    = 3,  // entrambe fuori soglia
    MINIMUM     = 4,  // ventilazione minima (CO₂ + rinforzo)
};

// ─────────────────────────────────────────────────────────────
//  LightState
//  Stato fisico del relay SSR della lampada.
// ─────────────────────────────────────────────────────────────
enum class LightState : uint8_t {
    OFF             = 0,
    ON              = 1,
    OFF_NTP_MISSING = 2,  // spento: NTP non sincronizzato
    OFF_MANUAL      = 3,  // spento: comando manuale (OFF)
    ON_MANUAL       = 4,  // acceso: comando manuale (ON)
};