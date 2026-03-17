// ============================================================
//  FirestorePublisher.h
//  Posizione: src/6.api/FirestorePublisher.h
//
//  Solo storico: scrive readings/ su Firestore ogni 2 minuti.
//  Lo stato live è su RTDB (gestito da FirebaseManager).
// ============================================================
#pragma once
#include <Firebase_ESP_Client.h>
#include "3.system/SystemState.h"

class FirestorePublisher
{
public:
    FirestorePublisher(SystemState &state);
    void begin(const String &deviceId);
    void update();

private:
    SystemState &_state;
    FirebaseData _fbData;
    String _deviceId;
    uint32_t _lastPushMs = 0;

    String _pathReadings()
    {
        String path = "devices/";
        path += _deviceId;
        path += "/readings";
        return path;
    }

    void _pushReading();
    void _buildReadingDoc(FirebaseJson &doc);
    String _getTimestampId();
};
