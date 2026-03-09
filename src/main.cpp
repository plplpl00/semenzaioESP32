// ============================================================
//  main.cpp
//  Posizione: src/main.cpp
//
//  MODALITÀ:
//    #define DEBUG_MODE    → abilita DebugLogger WebSocket
//    #define FIREBASE_MODE → abilita Firestore + RTDB
// ============================================================

#include <Arduino.h>
#include "utility.h"
#include "secrets.h"
#include "3.system/SystemConfig.h"
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
//  ORDINE: config e state prima di tutto il resto
// ─────────────────────────────────────────────────────────────
SystemConfig config;
SystemState  state;

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
EnvironmentMonitor    envMonitor(config.environment);
VentilationController ventCtrl(config.ventilation);
LightController       lightCtrl(config.light);

#ifdef FIREBASE_MODE
    FirebaseManager firebase(config, state);
#endif


// ─────────────────────────────────────────────────────────────
//  setup()
// ─────────────────────────────────────────────────────────────
void setup() {
    Serial.begin(115200);
    delay(500);
    Serial.println("\n=== Semenzaio — avvio ===");

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
    // ── Connettività ─────────────────────────────────────────
    wifi.handle();
    state.wifiConnected = wifi.isConnected();
    state.ntpSynced     = wifi.isNTPSynced();
    state.uptimeMs      = millis();



    // ── Monitor ──────────────────────────────────────────────
    envMonitor.update(state.environment);

    // ── Controller ───────────────────────────────────────────
    ventCtrl.update(state.environment, state.ventilation);
    lightCtrl.update(state.light, state.ntpSynced);

    // ── API ──────────────────────────────────────────────────
    #ifdef FIREBASE_MODE
        firebase.update();
    #endif

    // ── Log periodico ogni 6s ─────────────────────────────────
    #ifdef DEBUG_MODE
        static uint32_t lastPrint = 0;
        if (millis() - lastPrint >= 6000) {
            lastPrint = millis();

            auto& e = state.environment;
            auto& v = state.ventilation;
            auto& l = state.light;

            LOG_INFO("ENV",
                "Temp=%.2f  Hum=%.1f  TempSHT31=%.2f  Valid=%s",
                e.temperature, e.humidity, e.tempSHT31,
                e.isValid() ? "YES" : "NO");

            LOG_INFO("VENT",
                "Speed=%d  RPM=%d  Causa=%d  Mode=%d  Stalled=%s",
                v.speedPercent, v.rpm, (int)v.cause, (int)v.mode,
                v.stalled ? "YES" : "NO");

            LOG_INFO("LIGHT",
                "State=%d  On=%02d:00  Off=%02d:00",
                (int)l.state, l.onHour, l.offHour);
        }
    #endif
}