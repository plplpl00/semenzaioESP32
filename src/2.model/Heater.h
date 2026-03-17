// ============================================================
//  Heater.h
//  Posizione: src/2.model/Heater.h
// ============================================================
#pragma once
#include <stdint.h>
#include "1.core/DeviceMode.h"
#include "1.core/ActuatorTypes.h"

struct Heater {
    DeviceMode  mode      = DeviceMode::AUTO;
    HeaterState state     = HeaterState::OFF;
    bool        relayOn   = false;
    float       tempOn    = 0;    // soglia accensione (dalla ricetta)
    float       tempOff   = 0;    // soglia spegnimento (tempOn + isteresi)
    uint32_t    timestamp = 0;
};
