// ============================================================
//  LightController.h
//  Posizione: src/controllers/LightController.h
//
//  Gestisce il relay SSR della lampada LED.
//
//  Logica mode:
//    AUTO → segue fotoperiodo NTP (onHour/offHour)
//    ON   → relay chiuso indipendentemente dall'ora
//    OFF  → relay aperto indipendentemente dall'ora
//
//  Dipendenze: config/SystemConfig.h, model/Light.h
// ============================================================
#pragma once
#include <3.system/SystemConfig.h>
#include <Light.h>


class LightController {
public:
    explicit LightController(const LightConfig& config);

    void begin();

    // Chiamata nel loop().
    void update(Light& out, bool ntpSynced);

private:
    const LightConfig& _cfg;

    void _setRelay(bool on);
    bool _shouldBeOn(bool ntpSynced) const;
};