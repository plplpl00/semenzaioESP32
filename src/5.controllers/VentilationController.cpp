// ============================================================
//  VentilationController.cpp
//  Posizione: src/5.controllers/VentilationController.cpp
//
//  Logica ventilazione:
//    AUTO → rampa lineare basata su ricetta + delta esterno
//    ON   → manualSpeed
//    OFF  → ventola ferma
// ============================================================
#include <Arduino.h>
#include "utility.h"
#include "VentilationController.h"

VentilationController::VentilationController()
    : _fan(PIN_FAN_PWM, PIN_FAN_TACH)
{}

bool VentilationController::begin() {
    pinMode(PIN_VENTILATION_RELAY, OUTPUT);
    digitalWrite(PIN_VENTILATION_RELAY, HIGH);
    _fan.begin();
    _fan.setSpeed(20);  // speedMin di default all'avvio
    LOG_SUCCESS("VentCtrl", "Inizializzato");
    return true;
}

void VentilationController::update(const Environment& env,
                                    const RecipeParams& recipe,
                                    uint8_t currentHour,
                                    Ventilation& out) {
    _fan.update();

    // ── OFF manuale ──────────────────────────────────────
    if (out.mode == DeviceMode::OFF) {
        _applySpeed(0, VentilationCause::MANUAL, out);
        return;
    }

    // ── ON manuale ───────────────────────────────────────
    if (out.mode == DeviceMode::ON) {
        uint8_t speed = constrain(out.manualSpeed, (uint8_t)0, (uint8_t)100);
        _applySpeed(speed, VentilationCause::MANUAL, out);
        return;
    }

    // ── AUTO — rispetta il timer ─────────────────────────
    uint32_t now = millis();
    if (now - _lastUpdateMs < VENT_UPDATE_INTERVAL) return;
    _lastUpdateMs = now;

    // ── Nessuna ricetta attiva → sicurezza ───────────────
    if (!recipe.active) {
        _applySpeed(recipe.safety.offlineSpeedMin,
                    VentilationCause::SAFETY, out);
        return;
    }

    // ── Sensori non validi → sicurezza ───────────────────
    if (!env.isValid()) {
        LOG_WARNING("VentCtrl", "Sensori non validi → safety speed");
        _applySpeed(recipe.safety.offlineSpeedMin,
                    VentilationCause::SAFETY, out);
        return;
    }

    // ── Seleziona parametri giorno/notte ─────────────────
    const ClimateParams& params = recipe.currentClimate(currentHour);

    // ── Contributo TEMPERATURA ───────────────────────────
    // Attivo SOLO se il delta interno-esterno è favorevole
    uint8_t speedTemp = 0;
    bool tempRampActive = false;

    if (env.temperature >= params.tempThreshold) {
        // Verifica delta con sonda esterna
        bool deltaFavorable = true;  // default: ventila se no sonda esterna

        if (env.hasExternalTemp()) {
            deltaFavorable = env.tempDelta > recipe.safety.deltaMinForVent;

            // Alert temperatura esterna eccessiva
            if (env.tempExternal >= recipe.safety.externalTempMax) {
                LOG_ERROR("VentCtrl", "CRITICO: Temp esterna=%.1f >= %.1f",
                          env.tempExternal, recipe.safety.externalTempMax);
            }
        }

        if (deltaFavorable) {
            speedTemp = _calcRampSpeed(
                env.temperature,
                params.tempThreshold,
                params.tempMaxAlarm,
                params.speedMin,
                params.speedMax
            );
            tempRampActive = true;
        } else {
            LOG_INFO("VentCtrl", "Delta sfavorevole (%.1f) → rampa temp OFF",
                     env.tempDelta);
        }
    }

    // ── Contributo UMIDITÀ (SEMPRE attivo) ───────────────
    uint8_t speedHum = 0;

    if (env.humidity >= params.humThreshold) {
        speedHum = _calcRampSpeed(
            env.humidity,
            params.humThreshold,
            params.humMaxAlarm,
            params.speedMin,
            params.speedMax
        );
    }

    // ── Combinazione: MAX ────────────────────────────────
    uint8_t target = max(speedTemp, speedHum);
    target = max(target, params.speedMin);  // mai sotto speedMin
    target = constrain(target, params.speedMin, params.speedMax);

    // ── Determina causa ──────────────────────────────────
    VentilationCause cause;
    if (speedTemp > 0 && speedHum > 0)     cause = VentilationCause::COMBINED;
    else if (speedTemp > 0)                 cause = VentilationCause::TEMPERATURE;
    else if (speedHum > 0)                  cause = VentilationCause::HUMIDITY;
    else                                    cause = VentilationCause::MINIMUM;

    // ── Salva contributi per telemetria ──────────────────
    out.speedFromTemp  = speedTemp;
    out.speedFromHum   = speedHum;
    out.tempRampActive = tempRampActive;

    _applySpeed(target, cause, out);

    LOG_INFO("VentCtrl", "%s T=%.1f H=%.1f → Temp:%d%% Hum:%d%% → %d%% [%s]",
             tempRampActive ? "RAMP" : "NORP",
             env.temperature, env.humidity,
             speedTemp, speedHum, target,
             recipe.isDaytime(currentHour) ? "DAY" : "NIGHT");
}

// ─────────────────────────────────────────────────────────────
//  _calcRampSpeed() — Opzione B
//  Rampa lineare: speedMin a threshold → speedMax a maxAlarm
// ─────────────────────────────────────────────────────────────
uint8_t VentilationController::_calcRampSpeed(float value,
                                               float threshold,
                                               float maxAlarm,
                                               uint8_t speedMin,
                                               uint8_t speedMax) const {
    if (value < threshold)  return 0;
    if (value >= maxAlarm)  return speedMax;

    float range = maxAlarm - threshold;
    if (range <= 0) return speedMax;

    float ratio = (value - threshold) / range;
    return speedMin + (uint8_t)(ratio * (float)(speedMax - speedMin));
}

// ─────────────────────────────────────────────────────────────
//  _applySpeed()
// ─────────────────────────────────────────────────────────────
void VentilationController::_applySpeed(uint8_t speedPercent,
                                         VentilationCause cause,
                                         Ventilation& out) {
    if (speedPercent == 0) {
        digitalWrite(PIN_VENTILATION_RELAY, LOW);
        _fan.setSpeed(0);
    } else {
        digitalWrite(PIN_VENTILATION_RELAY, HIGH);
        _fan.setSpeed(speedPercent);
    }

    out.speedPercent = speedPercent;
    out.cause        = cause;
    out.rpm          = _fan.getRPM();
    out.stalled      = (out.rpm < STALL_RPM_THRESHOLD && speedPercent > 0);
    out.timestamp    = millis();

    if (out.stalled) {
        LOG_WARNING("VentCtrl", "STALL: RPM=%d Speed=%d%%", out.rpm, speedPercent);
    }
}
