// ============================================================
//  VentilationController.h
//  Posizione: src/controllers/VentilationController.h
//
//  Gestisce la ventola PWM in base a Environment e al mode
//  del model Ventilation.
//
//  Logica mode:
//    AUTO → calcola velocità da temperatura e umidità
//    ON   → usa ventilation.manualSpeed
//    OFF  → ferma ventola
//
//  Dipendenze: config/SystemConfig.h,
//              model/Ventilation.h, model/Environment.h
// ============================================================
#pragma once

#include <PWMFan.h>
#include <3.system/SystemConfig.h>
#include <2.model/Environment.h>
#include <Ventilation.h>

class VentilationController {
public:
    explicit VentilationController(const VentilationConfig& config);

    bool begin();

    // Chiamata nel loop() — rispetta updateInterval in AUTO.
    // In ON e OFF agisce immediatamente.
    void update(const Environment& env, Ventilation& out);

    // Forza ricalcolo immediato — solo in AUTO.
    void forceUpdate(const Environment& env, Ventilation& out);

private:
    const VentilationConfig& _cfg;
    PWMFan   _fan;
    uint32_t _lastUpdateMs = 0;

    uint8_t          _calcSpeedFromTemp    (float temp)     const;
    uint8_t          _calcSpeedFromHumidity(float humidity) const;
    VentilationCause _determineCause       (uint8_t speedTemp,
                                            uint8_t speedHum) const;
    void             _updateAlerts         (const Environment& env,
                                            Ventilation& out)  const;
    void             _applySpeed           (uint8_t speedPercent,
                                            VentilationCause cause,
                                            Ventilation& out);
};