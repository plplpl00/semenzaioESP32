// ============================================================
//  HeaterController.h
//  Posizione: src/5.controllers/HeaterController.h
//
//  Termostato con isteresi per tappetino riscaldante.
//  Parametri heaterTempOn e heaterHysteresis per fase day/night.
// ============================================================
#pragma once
#include "utility.h"
#include "2.model/RecipeParams.h"
#include "2.model/Heater.h"

class HeaterController {
public:
    void begin();
    void update(const RecipeParams& recipe,
                uint8_t currentHour,
                float temperature,
                bool tempValid,
                Heater& out);

private:
    void _setRelay(bool on);
};
