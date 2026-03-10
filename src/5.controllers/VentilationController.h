// ============================================================
//  VentilationController.h
//  Posizione: src/5.controllers/VentilationController.h
//
//  Nuova logica: rampa lineare da threshold a maxAlarm.
//  Contributo temperatura condizionato dal delta esterno.
//  Contributo umidità sempre attivo.
//  Combinazione: MAX(speedTemp, speedHum).
// ============================================================
#pragma once
#include <PWMFan.h>
#include "utility.h"
#include "2.model/Environment.h"
#include "2.model/Ventilation.h"
#include "2.model/RecipeParams.h"

class VentilationController {
public:
    VentilationController();
    bool begin();
    void update(const Environment& env, const RecipeParams& recipe,
                uint8_t currentHour, Ventilation& out);

private:
    PWMFan   _fan;
    uint32_t _lastUpdateMs = 0;
    static const uint16_t STALL_RPM_THRESHOLD = 100;

    // Calcola velocità con rampa lineare (Opzione B)
    // Raggiunge speedMax esattamente a maxAlarm
    uint8_t _calcRampSpeed(float value, float threshold,
                           float maxAlarm, uint8_t speedMin,
                           uint8_t speedMax) const;

    void _applySpeed(uint8_t speed, VentilationCause cause,
                     Ventilation& out);
};
