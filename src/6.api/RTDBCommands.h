// ============================================================
//  RTDBCommands.h
//  Posizione: src/6.api/RTDBCommands.h
//
//  Legge da RTDB in streaming:
//    devices/{deviceId}/shelves/0/
//      mode, cycle/, safety/
// ============================================================
#pragma once
#include <Firebase_ESP_Client.h>
#include "3.system/SystemState.h"

class RTDBCommands {
public:
    RTDBCommands(SystemState& state);
    void begin(const String& deviceId);
    void update();

private:
    SystemState& _state;
    FirebaseData _stream;
    String       _deviceId;
    bool         _streamStarted = false;
    bool         _initialLoad   = false;

    String _basePath() {
        return "/devices/" + _deviceId + "/shelves/0";
    }

    void _startStream();
    void _loadInitialData();
    void _applyStream();

    // Parser specifici
    void _parseMode(const String& path, const String& value);
    void _parseCycle(FirebaseJson& json);
    void _parseSafety(FirebaseJson& json);
    void _parseClimateParams(FirebaseJson& json, const String& prefix,
                              ClimateParams& params);
    void _parseIrrigations(FirebaseJson& json, IrrigationParams& params);

    DeviceMode _stringToMode(const String& str);
};
