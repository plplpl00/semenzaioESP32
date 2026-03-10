// ============================================================
//  SystemState.h
//  Posizione: src/3.system/SystemState.h
//
//  Snapshot runtime del semenzaio — unica istanza in main.cpp.
//
//  REGOLE DI SCRITTURA:
//    EnvironmentMonitor    → environment
//    VentilationController → ventilation
//    LightController       → light
//    RTDBCommands          → recipe, ventilation.mode, light.mode
//    main.cpp              → wifiConnected, ntpSynced, uptimeMs
// ============================================================
#pragma once
#include "2.model/Environment.h"
#include "2.model/Ventilation.h"
#include "2.model/Light.h"
#include "2.model/RecipeParams.h"

struct SystemState {
    // ── Sensori ──────────────────────────────────────────
    Environment  environment;

    // ── Attuatori ────────────────────────────────────────
    Ventilation  ventilation;
    Light        light;

    // ── Ricetta attiva (dal RTDB via Cloud Function) ─────
    RecipeParams recipe;

    // ── Connettività ─────────────────────────────────────
    bool wifiConnected  = false;
    bool ntpSynced      = false;
    bool firebaseOnline = false;

    // ── Uptime ───────────────────────────────────────────
    uint32_t uptimeMs = 0;

    // ── Ora corrente (aggiornata nel loop) ───────────────
    uint8_t currentHour   = 0;
    uint8_t currentMinute = 0;

    // ── Metodi ───────────────────────────────────────────
    bool isReady() const {
        return environment.isValid() && recipe.active;
    }

    bool hasAlert() const {
        return environment.hasAnyAlert() || ventilation.stalled;
    }

    bool isDaytime() const {
        return recipe.isDaytime(currentHour);
    }

    const ClimateParams& currentClimate() const {
        return recipe.currentClimate(currentHour);
    }
};
