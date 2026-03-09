// ============================================================
//  VentilationController.cpp
//  Posizione: src/controllers/VentilationController.cpp
// ============================================================

#include <Arduino.h>
#include <utility.h>
#include <VentilationController.h>

VentilationController::VentilationController(const VentilationConfig& config)
    : _cfg(config),
      _fan(PIN_FAN_PWM, PIN_FAN_TACH)
{}

bool VentilationController::begin() {
    pinMode(PIN_VENTILATION_RELAY, OUTPUT);
    digitalWrite(PIN_VENTILATION_RELAY, HIGH);
    _fan.begin();
    _fan.setSpeed(_cfg.speedMin);
    LOG_SUCCESS("VentCtrl", "Inizializzato — relay chiuso, velocità minima attiva");
    return true;
}

void VentilationController::update(const Environment& env, Ventilation& out) {
    _fan.update();

    // ON — velocità manuale dall'app
    if (out.mode == DeviceMode::ON) {
        uint8_t speed = constrain(out.manualSpeed, (uint8_t)0, (uint8_t)100);
        _applySpeed(speed, VentilationCause::MANUAL, out);
        return;
    }

    // AUTO — rispetta il timer
    uint32_t now = millis();
    if (now - _lastUpdateMs < _cfg.updateInterval) return;
    _lastUpdateMs = now;

    forceUpdate(env, out);
}

void VentilationController::forceUpdate(const Environment& env, Ventilation& out) {
    _fan.update();

    if (!env.isValid()) {
        LOG_WARNING("VentCtrl", "Sensori non validi → velocità minima");
        _applySpeed(_cfg.speedMin, VentilationCause::MINIMUM, out);
        return;
    }

    uint8_t speedTemp = _calcSpeedFromTemp(env.temperature);
    uint8_t speedHum  = _calcSpeedFromHumidity(env.humidity);
    uint8_t target    = max(speedTemp, speedHum);
    target = constrain(target, _cfg.speedMin, _cfg.speedMax);

    VentilationCause cause = _determineCause(speedTemp, speedHum);
    _applySpeed(target, cause, out);
    _updateAlerts(env, out);

    LOG_INFO("VentCtrl", "AUTO Temp=%.1f°C Hum=%.1f%% → Speed=%d%% Causa=%d", env.temperature, env.humidity, target, (int)cause);

}

uint8_t VentilationController::_calcSpeedFromTemp(float temp) const {
    if (temp < _cfg.tempIdealMax) return _cfg.speedMin;
    if (temp < _cfg.tempWarnMax)  return _cfg.speedNormal;
    if (temp < _cfg.tempCritical) return _cfg.speedHigh;
    return _cfg.speedMax;
}

uint8_t VentilationController::_calcSpeedFromHumidity(float hum) const {
    if (hum < _cfg.humidityIdealMax)  return _cfg.speedMin;
    if (hum < _cfg.humidityCritical)  return _cfg.speedHigh;
    return _cfg.speedMax;
}

VentilationCause VentilationController::_determineCause(uint8_t speedTemp,
                                                          uint8_t speedHum) const {
    bool tempActive = speedTemp > _cfg.speedMin;
    bool humActive  = speedHum  > _cfg.speedMin;
    if (tempActive && humActive) return VentilationCause::COMBINED;
    if (tempActive)              return VentilationCause::TEMPERATURE;
    if (humActive)               return VentilationCause::HUMIDITY;
    return VentilationCause::MINIMUM;
}

void VentilationController::_updateAlerts(const Environment& env,
                                           Ventilation& out) const {
    if (env.temperature >= _cfg.tempCritical)
        LOG_ERROR("VentCtrl", "CRITICO: Temp=%.1f°C", env.temperature);
    else if (env.temperature >= _cfg.tempWarnMax)
        LOG_WARNING("VentCtrl", "WARNING: Temp=%.1f°C", env.temperature);


    if (env.humidity >= _cfg.humidityCritical)
        LOG_ERROR("VentCtrl", "CRITICO: Hum=%.1f%%", env.humidity);
    else if (env.humidity >= _cfg.humidityIdealMax)
        LOG_WARNING("VentCtrl", "WARNING: Hum=%.1f%%", env.humidity);
}

void VentilationController::_applySpeed(uint8_t speedPercent,
                                         VentilationCause cause,
                                         Ventilation& out) {
    if (speedPercent == 0) {
        digitalWrite(PIN_VENTILATION_RELAY, LOW);  // apre relay
        _fan.setSpeed(0);
    } else {
        digitalWrite(PIN_VENTILATION_RELAY, HIGH);  // chiude relay
        _fan.setSpeed(speedPercent);
    }

    _fan.setSpeed(speedPercent);
    out.speedPercent = speedPercent;
    out.cause        = cause;
    out.rpm          = _fan.getRPM();
    out.stalled      = (out.rpm < _cfg.stallRpmThreshold && speedPercent > 0);
    out.timestamp    = millis();

    if (out.stalled)
        LOG_WARNING("VentCtrl", "STALL: RPM=%d Speed=%d%%", out.rpm, speedPercent);
}