// ============================================================
//  Environment.h
//  Posizione: src/2.model/Environment.h
//
//  Model sensori ambientali: DS18B20 interno, DS18B20 esterno, SHT31.
// ============================================================
#pragma once
#include <stdint.h>
#include "1.core/SensorTypes.h"

struct Environment {
    // --- Temperatura interna DS18B20 ---
    float        temperature     = -999.0f;
    SensorStatus tempStatus      = SensorStatus::DISCONNECTED;

    // --- Temperatura esterna DS18B20 ---
    float        tempExternal    = -999.0f;
    SensorStatus tempExtStatus   = SensorStatus::DISCONNECTED;

    // --- Delta termico (Tinterna - Testerna) ---
    float        tempDelta       = 0.0f;
    bool         deltaValid      = false;

    // --- Umidità + temperatura SHT31 ---
    float        humidity        = -1.0f;
    float        tempSHT31       = -999.0f;
    SensorStatus humidityStatus  = SensorStatus::DISCONNECTED;

    // --- Alert ---
    AlertLevel   tempAlert       = AlertLevel::OK;
    AlertLevel   humidityAlert   = AlertLevel::OK;
    AlertLevel   tempExtAlert    = AlertLevel::OK;

    // --- Timestamp ---
    uint32_t     timestamp       = 0;

    bool isValid() const {
        return tempStatus     == SensorStatus::OK &&
               humidityStatus == SensorStatus::OK;
    }

    bool hasExternalTemp() const {
        return tempExtStatus == SensorStatus::OK;
    }

    bool hasAnyAlert() const {
        return tempAlert     != AlertLevel::OK ||
               humidityAlert != AlertLevel::OK ||
               tempExtAlert  != AlertLevel::OK;
    }
};
