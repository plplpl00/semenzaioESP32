// ============================================================
//  FirestorePublisher.h
//  Posizione: src/6.api/FirestorePublisher.h
// ============================================================
#pragma once
#include <Firebase_ESP_Client.h>
#include "3.system/SystemState.h"

class FirestorePublisher {
public:
    FirestorePublisher(SystemState& state);
    void begin(const String& deviceId);
    void update();

private:
    SystemState& _state;
    FirebaseData _fbData;
    String       _deviceId;
    uint32_t     _lastPushMs = 0;

    String _pathState()    { return "devices/" + _deviceId + "/telemetry/current"; }
    String _pathReadings() { return "devices/" + _deviceId + "/readings"; }

    void   _pushState();
    void   _pushReading();
    void   _buildStateDoc(FirebaseJson& doc);
    void   _buildReadingDoc(FirebaseJson& doc);
    String _getTimestampId();
};
