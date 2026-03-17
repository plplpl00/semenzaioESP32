// ============================================================
//  HeaterController.cpp
//  Posizione: src/5.controllers/HeaterController.cpp
//
//  Logica termostato con isteresi:
//    - Sotto heaterTempOn → accendi
//    - Sopra heaterTempOn + heaterHysteresis → spegni
//    - heaterTempOn = 0 → riscaldamento disabilitato
// ============================================================
#include <Arduino.h>
#include "HeaterController.h"

void HeaterController::begin() {
    pinMode(PIN_HEATER_RELAY, OUTPUT);
    _setRelay(false);
    LOG_SUCCESS("HeaterCtrl", "Inizializzato — relay spento");
}

void HeaterController::update(const RecipeParams& recipe,
                               uint8_t currentHour,
                               float temperature,
                               bool tempValid,
                               Heater& out) {

    // ── OFF manuale ──────────────────────────────────────
    if (out.mode == DeviceMode::OFF) {
        _setRelay(false);
        out.state     = HeaterState::OFF_MANUAL;
        out.relayOn   = false;
        out.timestamp = millis();
        return;
    }

    // ── ON manuale ───────────────────────────────────────
    if (out.mode == DeviceMode::ON) {
        _setRelay(true);
        out.state     = HeaterState::ON_MANUAL;
        out.relayOn   = true;
        out.timestamp = millis();
        return;
    }

    // ── AUTO — nessuna ricetta attiva ────────────────────
    if (!recipe.active) {
        _setRelay(false);
        out.state     = HeaterState::OFF_NO_CYCLE;
        out.relayOn   = false;
        out.timestamp = millis();
        return;
    }

    // ── AUTO — parametri dalla fase corrente ─────────────
    const ClimateParams& climate = recipe.currentClimate(currentHour);
    out.tempOn  = climate.heaterTempOn;
    out.tempOff = climate.heaterTempOn + climate.heaterHysteresis;

    // ── Riscaldamento disabilitato (heaterTempOn = 0) ────
    if (climate.heaterTempOn <= 0.0f) {
        _setRelay(false);
        out.state     = HeaterState::OFF_DISABLED;
        out.relayOn   = false;
        out.timestamp = millis();
        return;
    }

    // ── Sensore temperatura non valido → spegni per sicurezza
    if (!tempValid) {
        _setRelay(false);
        out.state     = HeaterState::OFF;
        out.relayOn   = false;
        out.timestamp = millis();
        LOG_WARNING("HeaterCtrl", "Sensore temp non valido — spento");
        return;
    }

    // ── Logica termostato con isteresi ───────────────────
    // Se il riscaldatore è acceso, spegni solo sopra tempOff
    // Se è spento, accendi solo sotto tempOn
    if (out.relayOn) {
        // Attualmente acceso — spegni sopra soglia alta
        if (temperature >= out.tempOff) {
            _setRelay(false);
            out.relayOn = false;
            out.state   = HeaterState::OFF;
            LOG_INFO("HeaterCtrl", "OFF: %.1f°C >= %.1f°C (soglia+isteresi)",
                     temperature, out.tempOff);
        } else {
            out.state = HeaterState::ON;
        }
    } else {
        // Attualmente spento — accendi sotto soglia bassa
        if (temperature < out.tempOn) {
            _setRelay(true);
            out.relayOn = true;
            out.state   = HeaterState::ON;
            LOG_INFO("HeaterCtrl", "ON: %.1f°C < %.1f°C (soglia)",
                     temperature, out.tempOn);
        } else {
            out.state = HeaterState::OFF;
        }
    }

    out.timestamp = millis();
}

void HeaterController::_setRelay(bool on) {
    digitalWrite(PIN_HEATER_RELAY, on ? HIGH : LOW);
}
