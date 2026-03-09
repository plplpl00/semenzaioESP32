// ============================================================
//  ControllableDevice.h
//  Posizione: src/model/ControllableDevice.h
//
//  Struct base per tutti i dispositivi controllabili.
//  Contiene solo DeviceMode — nessun altro campo.
//
//  Dipendenze: core/DeviceMode.h
// ============================================================
#pragma once

#include <ControllableDevice.h>
#include "1.core/DeviceMode.h"

// ─────────────────────────────────────────────────────────────
//  ControllableDevice
//  Tutti i dispositivi che possono essere AUTO/ON/OFF
//  ereditano da questa struct.
// ─────────────────────────────────────────────────────────────
struct ControllableDevice {
    DeviceMode mode = DeviceMode::AUTO;
};