// ============================================================
//  main.cpp
//  Posizione: src/main.cpp
//
//  Semenzaio ESP32 — v2.0
//  Sistema guidato da ricette via Firebase.
//
//  Build flags:
//    -D DEBUG_MODE    → DebugLogger WebSocket
//    -D FIREBASE_MODE → Firestore + RTDB
// ============================================================

#include <Arduino.h>
#include "utility.h"
#include "secrets.h"
#include "3.system/SystemState.h"
#include <WiFiManager.h>

#ifdef DEBUG_MODE
    #include <DebugLogger.h>
#endif

#ifdef FIREBASE_MODE
    #include <Firebase_ESP_Client.h>
    #include "6.api/FirebaseManager.h"
#endif

#include "4.monitors/EnvironmentMonitor.h"
#include "5.controllers/VentilationController.h"
#include "5.controllers/LightController.h"

// ─────────────────────────────────────────────────────────────
//  Istanze globali
// ─────────────────────────────────────────────────────────────
SystemState state;

WiFiConfig wifiCfg = {
    .ssid              = WIFI_SSID,
    .password          = WIFI_PASSWORD,
    .hostname          = "semenzaio",
    .otaPassword       = OTA_PASSWORD,
    .enableOTA         = true,
    .enableNTP         = true,
    .gmtOffset         = 3600,
    .daylightOffset    = 3600,
    .maxRetries        = 20,
    .retryDelay        = 500,
    .reconnectInterval = 30000,
    .ntpResyncInterval = 3600000
};

WiFiManager           wifi(wifiCfg);
EnvironmentMonitor    envMonitor;
VentilationController ventCtrl;
LightController       lightCtrl;

#ifdef FIREBASE_MODE
    FirebaseManager firebase(state);
#endif

// ─────────────────────────────────────────────────────────────
//  Funzione di aggiornamento ora corrente
// ─────────────────────────────────────────────────────────────
void updateCurrentTime() {
    if (!state.ntpSynced) return;
    struct tm timeInfo;
    if (getLocalTime(&timeInfo)) {
        state.currentHour   = (uint8_t)timeInfo.tm_hour;
        state.currentMinute = (uint8_t)timeInfo.tm_min;
    }
}

// ─────────────────────────────────────────────────────────────
//  Modalità sicurezza (no WiFi / no Firebase)
// ─────────────────────────────────────────────────────────────
void applySafetyMode() {
    // Se non connesso e nessuna ricetta caricata → safety
    if (!state.wifiConnected && !state.recipe.active) {
        // Luci spente
        if (state.light.mode == DeviceMode::AUTO) {
            digitalWrite(PIN_LIGHT_RELAY, LOW);
            state.light.state = LightState::OFF_NO_CYCLE;
        }
        // Ventilazione minima di sicurezza
        if (state.ventilation.mode == DeviceMode::AUTO) {
            state.ventilation.speedPercent = state.recipe.safety.offlineSpeedMin;
        }
    }
}

// ─────────────────────────────────────────────────────────────
//  setup()
// ─────────────────────────────────────────────────────────────
void setup() {
    Serial.begin(115200);
    delay(500);
    Serial.println("\n=== Semenzaio v2.0 — avvio ===");

    wifi.begin();

    #ifdef DEBUG_MODE
        Debug.begin();
        LOG_INFO("MAIN", "DEBUG_MODE attivo");
    #endif

    bool envOk  = envMonitor.begin();
    bool ventOk = ventCtrl.begin();
    lightCtrl.begin();

    #ifdef FIREBASE_MODE
        firebase.begin();
        LOG_INFO("MAIN", "FIREBASE_MODE attivo");
    #endif

    LOG_INFO("MAIN", "Boot OK — ENV:%s VENT:%s",
             envOk  ? "OK" : "ERR",
             ventOk ? "OK" : "ERR");
}

// ─────────────────────────────────────────────────────────────
//  loop()
// ─────────────────────────────────────────────────────────────
void loop() {
    // ── Connettività ─────────────────────────────────────
    wifi.handle();
    state.wifiConnected = wifi.isConnected();
    state.ntpSynced     = wifi.isNTPSynced();
    state.uptimeMs      = millis();

    // ── Ora corrente ─────────────────────────────────────
    updateCurrentTime();

    // ── Monitor sensori ──────────────────────────────────
    envMonitor.update(state.environment);

    // ── Controller (usano parametri dalla ricetta) ───────
    ventCtrl.update(state.environment, state.recipe,
                    state.currentHour, state.ventilation);

    lightCtrl.update(state.recipe, state.currentHour,
                     state.ntpSynced, state.light);

    // ── API Firebase ─────────────────────────────────────
    #ifdef FIREBASE_MODE
        firebase.update();
    #endif

    // ── Sicurezza offline ────────────────────────────────
    applySafetyMode();

    // ── Log periodico ────────────────────────────────────
    #ifdef DEBUG_MODE
        static uint32_t lastPrint = 0;
        if (millis() - lastPrint >= 6000) {
            lastPrint = millis();

            auto& e = state.environment;
            auto& v = state.ventilation;
            auto& l = state.light;
            auto& r = state.recipe;

            LOG_INFO("ENV",
                "Tint=%.1f Text=%.1f Delta=%.1f Hum=%.1f%%",
                e.temperature, e.tempExternal, e.tempDelta, e.humidity);

            LOG_INFO("VENT",
                "Speed=%d%% RPM=%d Causa=%d TempRamp=%s [%s]",
                v.speedPercent, v.rpm, (int)v.cause,
                v.tempRampActive ? "ON" : "OFF",
                state.isDaytime() ? "DAY" : "NIGHT");

            LOG_INFO("LIGHT",
                "State=%d On=%02d:00 Off=%02d:00",
                (int)l.state, l.onHour, l.offHour);

            LOG_INFO("CYCLE",
                "Active=%s Recipe=%s Day=%d",
                r.active ? "YES" : "NO",
                r.recipeName, r.dayOfCycle);
        }
    #endif
}
