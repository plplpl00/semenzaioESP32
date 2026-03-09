// ============================================================
//  Environment.h
//  Posizione: src/model/Environment.h
//
//  Model dei sensori ambientali (DS18B20 + SHT31).
//  Non eredita da ControllableDevice — i sensori non si
//  controllano, acquisiscono dati e basta.
//
//  Dipendenze: core/SensorTypes.h
// ============================================================
#pragma once

#include <stdint.h>
#include "1.core/SensorTypes.h"

// ─────────────────────────────────────────────────────────────
//  Environment
//  Scritto da: EnvironmentMonitor
//  Letto da:   VentilationController, FirestorePublisher
// ─────────────────────────────────────────────────────────────
struct Environment {

    // --- Temperatura DS18B20 ---
    float        temperature    = -999.0f;  // °C, -999 = non valido
    SensorStatus tempStatus     = SensorStatus::DISCONNECTED;

    // --- Umidità + temperatura SHT31 ---
    float        humidity       = -1.0f;    // %, -1 = non valido
    float        tempSHT31      = -999.0f;  // °C per cross-check
    SensorStatus humidityStatus = SensorStatus::DISCONNECTED;

    // --- Alert aggregati ---
    AlertLevel   tempAlert      = AlertLevel::OK;
    AlertLevel   humidityAlert  = AlertLevel::OK;

    // --- Timestamp ultima lettura riuscita ---
    uint32_t     timestamp      = 0;

    // --- Metodi di comodo ---
    bool isValid() const {
        return tempStatus     == SensorStatus::OK &&
               humidityStatus == SensorStatus::OK;
    }

    bool hasAnyAlert() const {
        return tempAlert     != AlertLevel::OK ||
               humidityAlert != AlertLevel::OK;
    }
};