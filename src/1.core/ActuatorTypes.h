// ============================================================
//  ActuatorTypes.h
//  Posizione: src/1.core/ActuatorTypes.h
// ============================================================
#pragma once
#include <stdint.h>

enum class VentilationCause : uint8_t {
    MANUAL      = 0,
    TEMPERATURE = 1,
    HUMIDITY    = 2,
    COMBINED    = 3,
    MINIMUM     = 4,
    SAFETY      = 5,  // temp esterna troppo alta, delta sfavorevole
};

enum class LightState : uint8_t {
    OFF             = 0,
    ON              = 1,
    OFF_NTP_MISSING = 2,
    OFF_MANUAL      = 3,
    ON_MANUAL       = 4,
    OFF_NO_CYCLE    = 5,  // nessuna ricetta attiva
};
