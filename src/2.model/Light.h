// ============================================================
//  Light.h
//  Posizione: src/model/Light.h
//
//  Model del dispositivo luce (relay SSR + LED grow light).
//  Eredita da ControllableDevice — può essere AUTO/ON/OFF.
//
//  Logica mode:
//    AUTO → LightController segue fotoperiodo NTP
//    ON   → relay chiuso indipendentemente dall'ora
//    OFF  → relay aperto indipendentemente dall'ora
//
//  Dipendenze: model/ControllableDevice.h, core/ActuatorTypes.h
// ============================================================
#pragma once

#include <stdint.h>
#include <ControllableDevice.h>
#include <1.core/ActuatorTypes.h>

// ─────────────────────────────────────────────────────────────
//  Light
//
//  SCRITTURA:
//    LightController → state, onHour, offHour, timestamp
//    RTDBCommands    → mode
// ─────────────────────────────────────────────────────────────
struct Light : public ControllableDevice {

    // --- Stato fisico relay (scritto da LightController) ---
    LightState  state     = LightState::OFF;
    uint32_t    timestamp = 0;

    // --- Fotoperiodo ---
    // Inizializzato da LightConfig, aggiornabile da RTDBCommands.
    uint8_t     onHour    = 0;
    uint8_t     offHour   = 0;
};