// ============================================================
//  FirebaseManager.h
//  Posizione: src/api/FirebaseManager.h
//
//  Coordinatore Firebase — gestisce autenticazione e
//  inizializza FirestorePublisher e RTDBCommands.
//
//  Backoff automatico: dopo 3 fallimenti consecutivi
//  aspetta AUTH_BACKOFF_MS prima di riprovare, evitando
//  il blocco TOO_MANY_ATTEMPTS di Firebase Auth.
// ============================================================
#pragma once

#include <Firebase_ESP_Client.h>
#include "FirestorePublisher.h"
#include "RTDBCommands.h"

class FirebaseManager {
public:
    FirebaseManager(SystemConfig& config, SystemState& state);

    bool begin();
    void update();

    bool isConnected() const { return _connected; }
    bool isReady()     const { return Firebase.ready(); }

    FirestorePublisher& publisher() { return _publisher; }
    RTDBCommands&       commands()  { return _commands; }

private:
    SystemConfig& _cfg;
    SystemState&  _state;

    FirebaseAuth   _auth;
    FirebaseConfig _fbConfig;

    FirestorePublisher _publisher;
    RTDBCommands       _commands;

    bool     _connected   = false;
    bool     _initialized = false;
    String   _deviceId;

    uint32_t _lastAuthAttempt = 0;
    uint8_t  _authFailCount   = 0;

    static const uint8_t  AUTH_FAIL_THRESHOLD = 3;
    static const uint32_t AUTH_BACKOFF_MS     = 300000;

    void _initDeviceId();
    bool _isBackingOff() const;
};