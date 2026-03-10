// ============================================================
//  FirebaseManager.h
//  Posizione: src/6.api/FirebaseManager.h
// ============================================================
#pragma once
#include <Firebase_ESP_Client.h>
#include "FirestorePublisher.h"
#include "RTDBCommands.h"

class FirebaseManager {
public:
    FirebaseManager(SystemState& state);
    bool begin();
    void update();
    bool isConnected() const { return _connected; }

private:
    SystemState&  _state;
    FirebaseAuth   _auth;
    FirebaseConfig _fbConfig;

    FirestorePublisher _publisher;
    RTDBCommands       _commands;

    bool     _connected    = false;
    bool     _initialized  = false;
    String   _deviceId;

    uint32_t _lastAuthAttempt = 0;
    uint8_t  _authFailCount   = 0;
    static const uint8_t  AUTH_FAIL_THRESHOLD = 3;
    static const uint32_t AUTH_BACKOFF_MS     = 300000;

    void _initDeviceId();
    bool _isBackingOff() const;
};
