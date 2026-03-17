// ============================================================
//  FirebaseManager.h
//  Posizione: src/6.api/FirebaseManager.h
//
//  Coordinatore Firebase:
//    - Autenticazione e backoff
//    - RTDBCommands: legge comandi e ricetta da RTDB
//    - Status live: scrive stato su RTDB (periodico + immediato)
//    - FirestorePublisher: scrive storico su Firestore
// ============================================================
#pragma once
#include <Firebase_ESP_Client.h>
#include "FirestorePublisher.h"
#include "RTDBCommands.h"

class FirebaseManager
{
public:
    FirebaseManager(SystemState &state);
    bool begin();
    void receiveCommands();
    void sendStatus();
    void pushStatusNow();
    bool isConnected() const { return _connected; }

private:
    SystemState &_state;
    FirebaseAuth _auth;
    FirebaseConfig _fbConfig;

    FirestorePublisher _publisher;
    RTDBCommands _commands;

    bool _connected = false;
    bool _initialized = false;
    String _deviceId;

    // ── Auth backoff ─────────────────────────────────────
    uint32_t _lastAuthAttempt = 0;
    uint8_t _authFailCount = 0;
    static const uint8_t AUTH_FAIL_THRESHOLD = 3;
    static const uint32_t AUTH_BACKOFF_MS = 300000;

    // ── RTDB status push ─────────────────────────────────
    uint32_t _lastStatusPushMs = 0;
    FirebaseData _statusFbData;

    void _initDeviceId();
    bool _isBackingOff() const;
    void _pushStatusToRTDB();
};
