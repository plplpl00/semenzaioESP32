// ============================================================
//  DeviceMode.h
//  Posizione: src/core/DeviceMode.h
//
//  Modalità operativa comune a tutti i dispositivi
//  controllabili del semenzaio.
//
//  ZERO dipendenze — questo file non include nulla.
// ============================================================
#pragma once

#include <stdint.h>

// ─────────────────────────────────────────────────────────────
//  DeviceMode
//
//  AUTO → il controller decide in base a sensori e config
//  ON   → forzato attivo (comando manuale dall'app)
//  OFF  → forzato spento (comando manuale)
// ─────────────────────────────────────────────────────────────
enum class DeviceMode : uint8_t {
    AUTO = 0,
    ON   = 1,
    OFF  = 2,
};