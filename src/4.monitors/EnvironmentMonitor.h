// ============================================================
//  EnvironmentMonitor.h
//  Posizione: src/4.monitors/EnvironmentMonitor.h
//
//  Legge DS18B20 interno, DS18B20 esterno, SHT31.
//  Calcola delta termico interno-esterno.
// ============================================================
#pragma once
#include <DS18B20Sensor.h>
#include <SHT31Sensor.h>
#include "utility.h"
#include "2.model/Environment.h"

class EnvironmentMonitor {
public:
    EnvironmentMonitor();
    bool begin();
    void update(Environment& out);

private:
    DS18B20Sensor _ds18b20Int;    // sonda interna
    DS18B20Sensor _ds18b20Ext;    // sonda esterna
    SHT31Sensor   _sht31;

    // Media mobile
    static const uint8_t AVG_SAMPLES = 5;
    float    _tempSamples[AVG_SAMPLES]    = {};
    float    _tempExtSamples[AVG_SAMPLES] = {};
    float    _humSamples[AVG_SAMPLES]     = {};
    uint8_t  _sampleIndex = 0;
    uint8_t  _sampleCount = 0;

    // Timer non bloccante
    uint32_t _lastReadMs = 0;

    void  _readSensors(Environment& out);
    float _movingAverage(float* samples, float newVal);
    void  _updateDelta(Environment& out);
    bool  _crossCheck(float tempDS, float tempSHT) const;
};
