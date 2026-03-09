// ============================================================
//  EnvironmentMonitor.cpp
//  Posizione: src/monitors/EnvironmentMonitor.cpp
// ============================================================

#include "utility.h"
#include <Arduino.h>
#include "EnvironmentMonitor.h"

// ─────────────────────────────────────────────────────────────
//  Costruttore
//  DS18B20Sensor riceve pin + indirizzo da utility.h
//  SHT31Sensor   riceve indirizzo I2C da utility.h
// ─────────────────────────────────────────────────────────────
EnvironmentMonitor::EnvironmentMonitor(const EnvironmentConfig& config)
    : _cfg(config),
      _ds18b20(PIN_ONE_WIRE, SONDA_INTERNA),
      _sht31(ADDR_SHT31)
{}

// ─────────────────────────────────────────────────────────────
//  begin()
// ─────────────────────────────────────────────────────────────
bool EnvironmentMonitor::begin() {
    bool ds18b20Ok = _ds18b20.begin();
    bool sht31Ok   = _sht31.begin();

    if (!ds18b20Ok) LOG_WARNING("EnvMonitor", "DS18B20 non trovato");
    if (!sht31Ok)   LOG_WARNING("EnvMonitor", "SHT31 non trovato su indirizzo 0x%02X", ADDR_SHT31);

    if (ds18b20Ok && sht31Ok) {
        LOG_SUCCESS("EnvMonitor", "Entrambi i sensori inizializzati");
    } else if (ds18b20Ok || sht31Ok) {
        LOG_WARNING("EnvMonitor", "Un sensore non trovato — funzionamento degradato");
    } else {
        LOG_ERROR("EnvMonitor", "Nessun sensore trovato");
    }

    return ds18b20Ok || sht31Ok;
}

// ─────────────────────────────────────────────────────────────
//  update() — non bloccante, rispetta readInterval
// ─────────────────────────────────────────────────────────────
void EnvironmentMonitor::update(Environment& out) {
    uint32_t now = millis();
    if (now - _lastReadMs < _cfg.readInterval) return;
    _lastReadMs = now;

    _readSensors(out);
    _updateAlerts(out);
    out.timestamp = now;
}

// ─────────────────────────────────────────────────────────────
//  _readSensors()
// ─────────────────────────────────────────────────────────────
void EnvironmentMonitor::_readSensors(Environment& out) {

    // ── DS18B20 ──────────────────────────────────────────────
    TempReading ds = _ds18b20.read();

    if (ds.isValid()) {
        out.temperature  = _movingAverage(_tempSamples, ds.value);
        out.tempStatus   = SensorStatus::OK;
    } else {
        out.tempStatus   = SensorStatus::ERROR;
        LOG_WARNING("EnvMonitor", "DS18B20: %s", DS18B20Sensor::errorToString(ds.error));
    }

    // ── SHT31 ────────────────────────────────────────────────
    SHT31Reading sht = _sht31.read();

    if (sht.isValid()) {
        out.humidity     = _movingAverage(_humSamples, sht.humidity);
        out.tempSHT31    = sht.temperature;
        out.humidityStatus = SensorStatus::OK;
    } else {
        out.humidityStatus = SensorStatus::ERROR;
        LOG_WARNING("EnvMonitor", "SHT31: %s", SHT31Sensor::errorToString(sht.error));
    }

    // ── Cross-check ──────────────────────────────────────────
    if (out.tempStatus == SensorStatus::OK &&
        out.humidityStatus == SensorStatus::OK) {
        if (!_crossCheck(out.temperature, out.tempSHT31)) {
            LOG_WARNING("EnvMonitor", "Cross-check fallito: DS18B20=%.1f°C SHT31=%.1f°C",
                        out.temperature, out.tempSHT31);
        }
    }
}

// ─────────────────────────────────────────────────────────────
//  _movingAverage()
// ─────────────────────────────────────────────────────────────
float EnvironmentMonitor::_movingAverage(float* samples, float newVal) {
    samples[_sampleIndex] = newVal;

    uint8_t count = min((uint8_t)(_sampleCount + 1), _cfg.movingAvgSamples);
    if (_sampleCount < _cfg.movingAvgSamples) _sampleCount++;

    float sum = 0;
    for (uint8_t i = 0; i < count; i++) sum += samples[i];

    _sampleIndex = (_sampleIndex + 1) % _cfg.movingAvgSamples;
    return sum / count;
}

// ─────────────────────────────────────────────────────────────
//  _updateAlerts()
// ─────────────────────────────────────────────────────────────
void EnvironmentMonitor::_updateAlerts(Environment& out) {
    // Reset
    out.tempAlert     = AlertLevel::OK;
    out.humidityAlert = AlertLevel::OK;

    if (out.tempStatus != SensorStatus::OK) return;

    // Temperatura
    if (out.temperature >= 24.0f) {
        out.tempAlert = AlertLevel::CRITICAL;
        LOG_ERROR("EnvMonitor", "CRITICO: Temp=%.1f°C", out.temperature);
    } else if (out.temperature >= 21.0f) {
        out.tempAlert = AlertLevel::WARNING;
        LOG_WARNING("EnvMonitor", "WARNING: Temp=%.1f°C", out.temperature);
    }

    // Umidità
    if (out.humidityStatus != SensorStatus::OK) return;

    if (out.humidity >= 88.0f) {
        out.humidityAlert = AlertLevel::CRITICAL;
        LOG_ERROR("EnvMonitor", "CRITICO: Hum=%.1f%%", out.humidity);
    } else if (out.humidity >= 75.0f) {
        out.humidityAlert = AlertLevel::WARNING;
        LOG_WARNING("EnvMonitor", "WARNING: Hum=%.1f%%", out.humidity);
    }
}

// ─────────────────────────────────────────────────────────────
//  _crossCheck()
// ─────────────────────────────────────────────────────────────
bool EnvironmentMonitor::_crossCheck(float tempDS, float tempSHT) const {
    return abs(tempDS - tempSHT) <= _cfg.maxTempDelta;
}