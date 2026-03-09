// ============================================================
//  LightController.cpp
//  Posizione: src/controllers/LightController.cpp
// ============================================================

#include <time.h>
#include "LightController.h"
#include "utility.h"

LightController::LightController(const LightConfig& config)
    : _cfg(config)
{}

void LightController::begin() {
    pinMode(PIN_LIGHT_RELAY, OUTPUT);
    _setRelay(false);
    LOG_SUCCESS("LightCtrl", "Inizializzato — relay spento");
}

void LightController::update(Light& out, bool ntpSynced) {

    // Sincronizza fotoperiodo da config (aggiornabile da RTDB)
    out.onHour  = _cfg.onHour;
    out.offHour = _cfg.offHour;

    // OFF — spento manuale
    if (out.mode == DeviceMode::OFF) {
        _setRelay(false);
        out.state     = LightState::OFF_MANUAL;
        out.timestamp = millis();
        return;
    }

    // ON — acceso manuale
    if (out.mode == DeviceMode::ON) {
        _setRelay(true);
        out.state     = LightState::ON_MANUAL;
        out.timestamp = millis();
        return;
    }

    // AUTO — segue fotoperiodo NTP
    if (!ntpSynced) {
        _setRelay(false);
        out.state     = LightState::OFF_NTP_MISSING;
        out.timestamp = millis();
        return;
    }

    bool shouldOn = _shouldBeOn(ntpSynced);
    _setRelay(shouldOn);
    out.state     = shouldOn ? LightState::ON : LightState::OFF;
    out.timestamp = millis();
}

bool LightController::_shouldBeOn(bool ntpSynced) const {
    if (!ntpSynced) return false;

    struct tm timeInfo;
    if (!getLocalTime(&timeInfo)) return false;

    uint8_t hour = (uint8_t)timeInfo.tm_hour;

    if (_cfg.onHour < _cfg.offHour) {
        // Es. 06:00 → 22:00
        return hour >= _cfg.onHour && hour < _cfg.offHour;
    } else {
        // Es. 22:00 → 06:00 (attraversa mezzanotte)
        return hour >= _cfg.onHour || hour < _cfg.offHour;
    }
}

void LightController::_setRelay(bool on) {
    digitalWrite(PIN_LIGHT_RELAY, on ? HIGH : LOW);
}