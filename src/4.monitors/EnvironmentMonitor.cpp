// ============================================================
//  EnvironmentMonitor.cpp
//  Posizione: src/4.monitors/EnvironmentMonitor.cpp
// ============================================================
#include <Arduino.h>
#include "utility.h"
#include "EnvironmentMonitor.h"

EnvironmentMonitor::EnvironmentMonitor()
    : _ds18b20Int(PIN_ONE_WIRE, SONDA_INTERNA),
      _ds18b20Ext(PIN_ONE_WIRE, SONDA_ESTERNA),
      _sht31(ADDR_SHT31)
{}

bool EnvironmentMonitor::begin() {
    bool intOk  = _ds18b20Int.begin();
    bool extOk  = _ds18b20Ext.begin();
    bool shtOk  = _sht31.begin();

    if (!intOk) LOG_WARNING("EnvMon", "DS18B20 interno non trovato");
    if (!extOk) LOG_WARNING("EnvMon", "DS18B20 esterno non trovato (opzionale)");
    if (!shtOk) LOG_WARNING("EnvMon", "SHT31 non trovato su 0x%02X", ADDR_SHT31);

    if (intOk && shtOk) {
        LOG_SUCCESS("EnvMon", "Sensori principali OK%s",
                    extOk ? " + sonda esterna" : "");
    }

    return intOk || shtOk;
}

void EnvironmentMonitor::update(Environment& out) {
    uint32_t now = millis();
    if (now - _lastReadMs < ENV_READ_INTERVAL) return;
    _lastReadMs = now;

    _readSensors(out);
    _updateDelta(out);
    out.timestamp = now;
}

void EnvironmentMonitor::_readSensors(Environment& out) {
    // ── DS18B20 Interno ──────────────────────────────────
    TempReading dsInt = _ds18b20Int.read();
    if (dsInt.isValid()) {
        out.temperature = _movingAverage(_tempSamples, dsInt.value);
        out.tempStatus  = SensorStatus::OK;
    } else {
        out.tempStatus = SensorStatus::ERROR;
    }

    // ── DS18B20 Esterno ──────────────────────────────────
    TempReading dsExt = _ds18b20Ext.read();
    if (dsExt.isValid()) {
        out.tempExternal  = _movingAverage(_tempExtSamples, dsExt.value);
        out.tempExtStatus = SensorStatus::OK;
    } else {
        out.tempExtStatus = SensorStatus::ERROR;
    }

    // ── SHT31 ────────────────────────────────────────────
    SHT31Reading sht = _sht31.read();
    if (sht.isValid()) {
        out.humidity       = _movingAverage(_humSamples, sht.humidity);
        out.tempSHT31      = sht.temperature;
        out.humidityStatus = SensorStatus::OK;
    } else {
        out.humidityStatus = SensorStatus::ERROR;
    }

    // ── Cross-check ──────────────────────────────────────
    if (out.tempStatus == SensorStatus::OK &&
        out.humidityStatus == SensorStatus::OK) {
        if (!_crossCheck(out.temperature, out.tempSHT31)) {
            LOG_WARNING("EnvMon", "Cross-check: DS18B20=%.1f SHT31=%.1f",
                        out.temperature, out.tempSHT31);
        }
    }
}

void EnvironmentMonitor::_updateDelta(Environment& out) {
    if (out.tempStatus == SensorStatus::OK &&
        out.tempExtStatus == SensorStatus::OK) {
        out.tempDelta  = out.temperature - out.tempExternal;
        out.deltaValid = true;
    } else {
        out.deltaValid = false;
    }
}

float EnvironmentMonitor::_movingAverage(float* samples, float newVal) {
    samples[_sampleIndex] = newVal;
    uint8_t count = min((uint8_t)(_sampleCount + 1), AVG_SAMPLES);
    if (_sampleCount < AVG_SAMPLES) _sampleCount++;

    float sum = 0;
    for (uint8_t i = 0; i < count; i++) sum += samples[i];

    _sampleIndex = (_sampleIndex + 1) % AVG_SAMPLES;
    return sum / count;
}

bool EnvironmentMonitor::_crossCheck(float tempDS, float tempSHT) const {
    return abs(tempDS - tempSHT) <= 3.0f;
}
