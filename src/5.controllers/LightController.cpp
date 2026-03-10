// ============================================================
//  LightController.cpp
//  Posizione: src/5.controllers/LightController.cpp
// ============================================================
#include <Arduino.h>
#include "LightController.h"

void LightController::begin() {
    pinMode(PIN_LIGHT_RELAY, OUTPUT);
    _setRelay(false);
    LOG_SUCCESS("LightCtrl", "Inizializzato — relay spento");
}

void LightController::update(const RecipeParams& recipe,
                              uint8_t currentHour,
                              bool ntpSynced,
                              Light& out) {
    out.onHour  = recipe.lightOnHour;
    out.offHour = recipe.lightOffHour;

    // ── OFF manuale ──────────────────────────────────────
    if (out.mode == DeviceMode::OFF) {
        _setRelay(false);
        out.state     = LightState::OFF_MANUAL;
        out.timestamp = millis();
        return;
    }

    // ── ON manuale ───────────────────────────────────────
    if (out.mode == DeviceMode::ON) {
        _setRelay(true);
        out.state     = LightState::ON_MANUAL;
        out.timestamp = millis();
        return;
    }

    // ── AUTO — nessuna ricetta attiva ────────────────────
    if (!recipe.active) {
        _setRelay(recipe.safety.offlineLightOff ? false : true);
        out.state     = LightState::OFF_NO_CYCLE;
        out.timestamp = millis();
        return;
    }

    // ── AUTO — NTP non sincronizzato ─────────────────────
    if (!ntpSynced) {
        _setRelay(false);
        out.state     = LightState::OFF_NTP_MISSING;
        out.timestamp = millis();
        return;
    }

    // ── AUTO — segue fotoperiodo dalla ricetta ───────────
    bool shouldOn = recipe.isDaytime(currentHour);
    _setRelay(shouldOn);
    out.state     = shouldOn ? LightState::ON : LightState::OFF;
    out.timestamp = millis();
}

void LightController::_setRelay(bool on) {
    digitalWrite(PIN_LIGHT_RELAY, on ? HIGH : LOW);
}
