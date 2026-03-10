// ============================================================
//  LightController.h
//  Posizione: src/5.controllers/LightController.h
// ============================================================
#pragma once
#include "utility.h"
#include "2.model/Light.h"
#include "2.model/RecipeParams.h"

class LightController {
public:
    void begin();
    void update(const RecipeParams& recipe, uint8_t currentHour,
                bool ntpSynced, Light& out);

private:
    void _setRelay(bool on);
};
