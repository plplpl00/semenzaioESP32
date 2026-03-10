// ============================================================
//  FirebaseManager.cpp
//  Posizione: src/6.api/FirebaseManager.cpp
// ============================================================
#include <Firebase_ESP_Client.h>
#include "FirebaseManager.h"
#include "utility.h"
#include "secrets.h"
#include <Arduino.h>
#include <esp_mac.h>

FirebaseManager::FirebaseManager(SystemState& state)
    : _state(state),
      _publisher(state),
      _commands(state)
{}

bool FirebaseManager::begin() {
    _initDeviceId();

    _fbConfig.api_key      = FIREBASE_API_KEY;
    _fbConfig.database_url = FIREBASE_DATABASE_URL;
    _auth.user.email       = FIREBASE_USER_EMAIL;
    _auth.user.password    = FIREBASE_USER_PASS;

    Firebase.begin(&_fbConfig, &_auth);
    Firebase.reconnectWiFi(true);

    _publisher.begin(_deviceId);
    _commands.begin(_deviceId);

    _initialized = true;
    LOG_SUCCESS("Firebase", "Device: %s", _deviceId.c_str());
    return true;
}

void FirebaseManager::update() {
    if (!_initialized)          return;
    if (!_state.wifiConnected)  return;
    if (!_state.ntpSynced)      return;

    if (_isBackingOff()) {
        _state.firebaseOnline = false;
        return;
    }

    if (!Firebase.ready()) {
        _lastAuthAttempt = millis();
        _authFailCount++;
        if (_authFailCount >= AUTH_FAIL_THRESHOLD) {
            LOG_ERROR("Firebase", "%d fallimenti — backoff %d min",
                      _authFailCount, AUTH_BACKOFF_MS / 60000);
        }
        _state.firebaseOnline = false;
        return;
    }

    if (_authFailCount > 0) {
        LOG_SUCCESS("Firebase", "Connessione ripristinata");
        _authFailCount = 0;
    }

    _connected            = true;
    _state.firebaseOnline = true;

    _commands.update();
    _publisher.update();
}

bool FirebaseManager::_isBackingOff() const {
    if (_authFailCount < AUTH_FAIL_THRESHOLD) return false;
    return (millis() - _lastAuthAttempt) < AUTH_BACKOFF_MS;
}

void FirebaseManager::_initDeviceId() {
    // TODO: per ora usa ID fisso "scaffale_a" per matchare RTDB
    // In futuro: generare da MAC address
    _deviceId = "scaffale_a";

    // Alternativa con MAC:
    // uint8_t mac[6];
    // esp_read_mac(mac, ESP_MAC_WIFI_STA);
    // char buf[13];
    // snprintf(buf, sizeof(buf), "%02x%02x%02x%02x%02x%02x",
    //          mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    // _deviceId = String(buf);
}
