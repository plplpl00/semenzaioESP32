// ============================================================
//  SensorTypes.h
//  Posizione: src/core/SensorTypes.h
//
//  Enumerazioni relative ai sensori fisici.
//  ZERO dipendenze — questo file non include nulla.
// ============================================================
#pragma once

#include <stdint.h>

// ─────────────────────────────────────────────────────────────
//  SensorStatus
//  Stato di salute di un singolo sensore.
// ─────────────────────────────────────────────────────────────
enum class SensorStatus : uint8_t {
    OK           = 0,
    ERROR        = 1,
    DISCONNECTED = 2,
    OUT_OF_RANGE = 3,
};

// ─────────────────────────────────────────────────────────────
//  AlertLevel
//  Gravità di una condizione anomala rilevata da un sensore.
// ─────────────────────────────────────────────────────────────
enum class AlertLevel : uint8_t {
    OK       = 0,
    WARNING  = 1,
    CRITICAL = 2,
};