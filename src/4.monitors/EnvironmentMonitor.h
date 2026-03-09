// ============================================================
//  EnvironmentMonitor.h
//  Posizione: src/monitors/EnvironmentMonitor.h
//
//  Legge DS18B20 (temperatura) e SHT31 (umidità + temperatura).
//  Applica media mobile e cross-check tra i due sensori.
//  Scrive su Environment — non conosce altri moduli.
//
//  Dipendenze: config/SystemConfig.h, model/Environment.h,
//              lib/DS18B20Sensor, lib/SHT31Sensor
// ============================================================
#pragma once

#include <DS18B20Sensor.h>
#include <SHT31Sensor.h>
#include "utility.h"
#include <3.system/SystemConfig.h>
#include <Environment.h>

class EnvironmentMonitor {
public:
    explicit EnvironmentMonitor(const EnvironmentConfig& config);

    // Inizializza i sensori. Ritorna true se almeno uno è trovato.
    bool begin();

    // Chiamata nel loop() — non bloccante.
    void update(Environment& out);

private:
    const EnvironmentConfig& _cfg;

    // Librerie custom
    DS18B20Sensor _ds18b20;   // usa SONDA_INTERNA da utility.h
    SHT31Sensor   _sht31;     // usa ADDR_SHT31 da utility.h

    // Media mobile
    float    _tempSamples[8] = {};
    float    _humSamples[8]  = {};
    uint8_t  _sampleIndex    = 0;
    uint8_t  _sampleCount    = 0;

    // Timer non bloccante
    uint32_t _lastReadMs         = 0;
    bool     _conversionStarted  = false;

    void  _readSensors   (Environment& out);
    float _movingAverage (float* samples, float newVal);
    void  _updateAlerts  (Environment& out);
    bool  _crossCheck    (float tempDS, float tempSHT) const;
};