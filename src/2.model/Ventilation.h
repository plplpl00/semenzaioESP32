// ============================================================
//  Ventilation.h
//  Posizione: src/2.model/Ventilation.h
// ============================================================
#pragma once
#include <stdint.h>
#include "1.core/DeviceMode.h"
#include "1.core/ActuatorTypes.h"

struct Ventilation {
    DeviceMode       mode         = DeviceMode::AUTO;
    uint8_t          speedPercent = 0;
    uint16_t         rpm          = 0;
    bool             stalled      = false;
    VentilationCause cause        = VentilationCause::MINIMUM;
    uint32_t         timestamp    = 0;
    uint8_t          manualSpeed  = 50;

    // Contributi individuali (per debug/telemetria)
    uint8_t          speedFromTemp = 0;
    uint8_t          speedFromHum  = 0;
    bool             tempRampActive = false;  // false se delta sfavorevole
};
