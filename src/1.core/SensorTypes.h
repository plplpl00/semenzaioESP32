// ============================================================
//  SensorTypes.h
//  Posizione: src/1.core/SensorTypes.h
// ============================================================
#pragma once
#include <stdint.h>

enum class SensorStatus : uint8_t {
    OK           = 0,
    ERROR        = 1,
    DISCONNECTED = 2,
    OUT_OF_RANGE = 3,
};

enum class AlertLevel : uint8_t {
    OK       = 0,
    WARNING  = 1,
    CRITICAL = 2,
};
