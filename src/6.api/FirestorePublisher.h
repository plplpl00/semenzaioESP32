// ============================================================
//  FirestorePublisher.h
//  Posizione: src/api/FirestorePublisher.h
//
//  Pubblica telemetria su Firestore.
//  - Ogni pushInterval: aggiorna semenzaio/state
//  - Ogni pushInterval: aggiunge documento in readings/
//
//  REGOLA: solo scrittura, mai lettura.
// ============================================================
#pragma once

#include <Firebase_ESP_Client.h>
#include <3.system/SystemConfig.h>
#include <3.system/SystemState.h>

class FirestorePublisher {
public:
    FirestorePublisher(SystemConfig& config, SystemState& state);

    void begin(const String& deviceId);
    void update();

private:
    SystemConfig& _cfg;
    SystemState&  _state;

    FirebaseData _fbData;
    String       _deviceId;
    uint32_t     _lastPushMs = 0;

    String _pathState()    { return "devices/semenzaio"; }
    String _pathReadings() { return "devices/semenzaio/readings"; }

    void   _pushState();
    void   _pushReading();
    void   _buildStateDoc  (FirebaseJson& doc);
    void   _buildReadingDoc(FirebaseJson& doc);
    String _getTimestampId();
};