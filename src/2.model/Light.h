// ============================================================
//  Light.h
//  Posizione: src/2.model/Light.h
// ============================================================
#pragma once
#include <stdint.h>
#include "1.core/DeviceMode.h"
#include "1.core/ActuatorTypes.h"

struct Light {
    DeviceMode mode      = DeviceMode::AUTO;
    LightState state     = LightState::OFF;
    uint8_t    onHour    = 0;
    uint8_t    offHour   = 0;
    uint32_t   timestamp = 0;
};
