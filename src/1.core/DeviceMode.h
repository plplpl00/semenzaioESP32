// ============================================================
//  DeviceMode.h
//  Posizione: src/1.core/DeviceMode.h
//
//  Modalità operativa per ogni ripiano.
//  AUTO → segue la ricetta attiva
//  ON   → forzato attivo (comando manuale)
//  OFF  → forzato spento (comando manuale)
// ============================================================
#pragma once
#include <stdint.h>

enum class DeviceMode : uint8_t {
    AUTO = 0,
    ON   = 1,
    OFF  = 2,
};
