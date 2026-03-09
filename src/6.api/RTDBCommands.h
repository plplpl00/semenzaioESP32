// ============================================================
//  RTDBCommands.h
//  Posizione: src/api/RTDBCommands.h
//
//  Riceve comandi in streaming da Realtime Database.
//  Aggiorna SystemConfig e SystemState — i controller
//  reagiscono al ciclo successivo.
//
//  Struttura RTDB:
//    /semenzaio/esp32/commands/
//      ventilation/
//        mode: "AUTO" | "ON" | "OFF"
//        manualSpeed: 0-100
//      light/
//        mode: "AUTO" | "ON" | "OFF"
//        onHour: 0-23
//        offHour: 0-23
//      thresholds/
//        tempIdealMax, tempWarnMax, tempCritical
//        humidityIdealMax, humidityCritical
//
//  REGOLA: solo lettura, mai scrittura su RTDB.
// ============================================================
#pragma once

#include <Firebase_ESP_Client.h>
#include <3.system/SystemConfig.h>
#include <3.system/SystemState.h>

class RTDBCommands {
public:
    RTDBCommands(SystemConfig& config, SystemState& state);

    void begin(const String& deviceId);

    // Chiamata nel loop() — controlla se ci sono nuovi dati
    // dallo stream e li applica a SystemConfig/SystemState.
    void update();

private:
    SystemConfig& _cfg;
    SystemState&  _state;

    FirebaseData _stream;
    String       _deviceId;
    bool         _streamStarted = false;

    String _pathCommands() {
        String p = "/devices/semenzaio/commands"; return p;
    }

    void _startStream();
    void _applyStream();

    void _applyVentilation(FirebaseJson& json);
    void _applyLight      (FirebaseJson& json);
    void _applyThresholds (FirebaseJson& json);

    DeviceMode _parseMode(const String& modeStr);
};