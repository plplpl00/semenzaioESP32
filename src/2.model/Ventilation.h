// ============================================================
//  Ventilation.h
//  Posizione: src/model/Ventilation.h
//
//  Model del dispositivo di ventilazione (ventola PWM).
//  Eredita da ControllableDevice — può essere AUTO/ON/OFF.
//
//  Logica mode:
//    AUTO → VentilationController calcola da temperatura/umidità
//    ON   → usa manualSpeed (0-100%)
//    OFF  → ventola ferma (spegnimento manuale)
//
//  Dipendenze: model/ControllableDevice.h, core/ActuatorTypes.h
// ============================================================
#pragma once

#include <stdint.h>
#include <1.core/ActuatorTypes.h>
#include <ControllableDevice.h>
// ─────────────────────────────────────────────────────────────
//  Ventilation
//
//  SCRITTURA:
//    VentilationController → speedPercent, rpm, stalled,
//                            cause, timestamp
//    RTDBCommands          → mode, manualSpeed
// ─────────────────────────────────────────────────────────────
struct Ventilation : public ControllableDevice {

    // --- Stato runtime (scritto da VentilationController) ---
    uint8_t          speedPercent = 0;
    uint16_t         rpm          = 0;
    bool             stalled      = false;
    VentilationCause cause        = VentilationCause::MINIMUM;
    uint32_t         timestamp    = 0;

    // --- Controllo manuale (scritto da RTDBCommands) ---
    // Usato solo quando mode == ON.
    uint8_t          manualSpeed  = 50;
};