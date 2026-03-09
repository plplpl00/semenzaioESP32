// ============================================================
//  SystemState.h
//  Posizione: src/config/SystemState.h
//
//  Snapshot runtime del semenzaio — unica istanza in main.cpp.
//  Passata per riferimento a tutti i moduli.
//
//  REGOLE DI SCRITTURA:
//    EnvironmentMonitor    → environment
//    VentilationController → ventilation (speedPercent, rpm,
//                            stalled, cause, timestamp)
//    LightController       → light (state, timestamp)
//    RTDBCommands          → ventilation.mode, ventilation.manualSpeed
//    main.cpp              → wifiConnected, ntpSynced, uptimeMs
//    FirestorePublisher    → firebaseOnline
//
//  REGOLA DI LETTURA: tutti i moduli possono leggere tutto.
//
//  Dipendenze: model/Environment.h, model/Ventilation.h,
//              model/Light.h
// ============================================================
#pragma once
#include <2.model/Ventilation.h>
#include <Environment.h>
#include <Light.h>


// ─────────────────────────────────────────────────────────────
//  SystemState
// ─────────────────────────────────────────────────────────────
struct SystemState {

    // ── Model dispositivi ─────────────────────────────────────
    Environment  environment;
    Ventilation  ventilation;
    Light        light;

    // ── Connettività ─────────────────────────────────────────
    bool wifiConnected  = false;
    bool ntpSynced      = false;
    bool firebaseOnline = false;

    // ── Uptime ───────────────────────────────────────────────
    uint32_t uptimeMs = 0;

    // ── Metodi di comodo ─────────────────────────────────────
    bool isReady() const {
        return environment.isValid();
    }

    bool hasAlert() const {
        return environment.hasAnyAlert() || ventilation.stalled;
    }
};