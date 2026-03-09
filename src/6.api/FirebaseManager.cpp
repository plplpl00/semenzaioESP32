// ============================================================
//  FirebaseManager.cpp
//  Posizione: src/api/FirebaseManager.cpp
// ============================================================

#include <Firebase_ESP_Client.h>
#include "FirebaseManager.h"
#include "utility.h"
#include "secrets.h"
#include <Arduino.h>
#include <esp_mac.h>

// ─────────────────────────────────────────────────────────────
//  Costruttore
// ─────────────────────────────────────────────────────────────
FirebaseManager::FirebaseManager(SystemConfig& config, SystemState& state)
    : _cfg(config),
      _state(state),
      _publisher(config, state),
      _commands(config, state)
{}

// ─────────────────────────────────────────────────────────────
//  begin()
// ─────────────────────────────────────────────────────────────
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
    LOG_SUCCESS("Firebase", "Inizializzato — Device: %s", _deviceId.c_str());
    return true;
}

// ─────────────────────────────────────────────────────────────
//  update()
// ─────────────────────────────────────────────────────────────
void FirebaseManager::update() {
    if (!_initialized)         return;
    if (!_state.wifiConnected) return;
    if (!_state.ntpSynced)     return;

    if (_isBackingOff()) {
        uint32_t remaining = (AUTH_BACKOFF_MS -
                             (millis() - _lastAuthAttempt)) / 1000;
        static uint32_t lastBackoffLog = 0;
        if (millis() - lastBackoffLog >= 30000) {
            lastBackoffLog = millis();
            LOG_WARNING("Firebase", "Backoff attivo — riprovo tra %lu s", remaining);
        }
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

    _publisher.update();
    _commands.update();
}

// ─────────────────────────────────────────────────────────────
//  _isBackingOff()
// ─────────────────────────────────────────────────────────────
bool FirebaseManager::_isBackingOff() const {
    if (_authFailCount < AUTH_FAIL_THRESHOLD) return false;
    return (millis() - _lastAuthAttempt) < AUTH_BACKOFF_MS;
}

// ─────────────────────────────────────────────────────────────
//  _initDeviceId()
// ─────────────────────────────────────────────────────────────
void FirebaseManager::_initDeviceId() {
    uint8_t mac[6];
    esp_read_mac(mac, ESP_MAC_WIFI_STA);
    char buf[13];
    snprintf(buf, sizeof(buf), "%02x%02x%02x%02x%02x%02x",
             mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    _deviceId = String(buf);
}