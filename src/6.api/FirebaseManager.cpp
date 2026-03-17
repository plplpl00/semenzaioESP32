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

FirebaseManager::FirebaseManager(SystemState &state)
    : _state(state),
      _publisher(state),
      _commands(state)
{
}

void FirebaseManager::pushStatusNow()
{
    if (!_connected) return;
    LOG_INFO("RTDB", "Push NOW: lightMode=%d lightState=%d fanMode=%d",
             (int)_state.light.mode, (int)_state.light.state, (int)_state.ventilation.mode);
    _pushStatusToRTDB();
    LOG_INFO("RTDB", "Status push immediato (comando eseguito)");
}


bool FirebaseManager::begin()
{
    _initDeviceId();

    _fbConfig.api_key = FIREBASE_API_KEY;
    _fbConfig.database_url = FIREBASE_DATABASE_URL;
    _auth.user.email = FIREBASE_USER_EMAIL;
    _auth.user.password = FIREBASE_USER_PASS;

    Firebase.begin(&_fbConfig, &_auth);
    Firebase.reconnectWiFi(true);

    _publisher.begin(_deviceId);
    _commands.begin(_deviceId);

    _initialized = true;
    LOG_SUCCESS("Firebase", "Device: %s", _deviceId.c_str());
    return true;
}
void FirebaseManager::receiveCommands()
{
    if (!_initialized)     return;
    if (!_state.wifiConnected) return;
    if (!_state.ntpSynced)     return;

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

    // ── Ricevi comandi da RTDB ───────────────────────────
    _commands.update();
}

void FirebaseManager::sendStatus()
{
    if (!_connected) return;

    // ── Push periodico RTDB ──────────────────────────────
    uint32_t now = millis();
    if (now - _lastStatusPushMs >= FIREBASE_PUSH_INTERVAL)
    {
        _lastStatusPushMs = now;
        _pushStatusToRTDB();
        LOG_INFO("RTDB", "Status push periodico");
    }

    // ── Storico su Firestore (sfalsato 60s) ──────────────
    static uint32_t lastFirestoreMs = 60000;
    if (now - lastFirestoreMs >= FIREBASE_PUSH_INTERVAL)
    {
        lastFirestoreMs = now;
        _publisher.update();
    }
}

// ─────────────────────────────────────────────────────────────
//  _pushStatusToRTDB() — scrive lo stato completo su RTDB
//  Path: devices/{deviceId}/shelves/0/status/
// ─────────────────────────────────────────────────────────────
void FirebaseManager::_pushStatusToRTDB()
{
    auto &e = _state.environment;
    auto &v = _state.ventilation;
    auto &l = _state.light;
    auto &r = _state.recipe;

    FirebaseJson json;

    // Ambiente
    json.set("temperature", e.temperature);
    json.set("tempExternal", e.tempExternal);
    json.set("tempDelta", e.tempDelta);
    json.set("humidity", e.humidity);
    json.set("tempValid", e.tempStatus == SensorStatus::OK);
    json.set("tempExtValid", e.tempExtStatus == SensorStatus::OK);
    json.set("humidityValid", e.humidityStatus == SensorStatus::OK);

    // Ventilazione
    json.set("fanSpeed", (int)v.speedPercent);
    json.set("fanRPM", (int)v.rpm);
    json.set("fanStalled", v.stalled);
    json.set("fanCause", (int)v.cause);
    json.set("fanMode", (int)v.mode);
    json.set("fanSpeedFromTemp", (int)v.speedFromTemp);
    json.set("fanSpeedFromHum", (int)v.speedFromHum);
    json.set("fanTempRampActive", v.tempRampActive);

    // Luce
    bool lightOn = (l.state == LightState::ON || l.state == LightState::ON_MANUAL);
    json.set("lightOn", lightOn);
    json.set("lightState", (int)l.state);
    json.set("lightMode", (int)l.mode);

    // Ricetta
    json.set("cycleActive", r.active);
    json.set("recipeName", String(r.recipeName));
    json.set("dayOfCycle", (int)r.dayOfCycle);

    // Sistema
    json.set("wifiConnected", _state.wifiConnected);
    json.set("ntpSynced", _state.ntpSynced);
    json.set("uptimeMs", (int)millis());

    String path = "/devices/";
    path += _deviceId;
    path += "/shelves/0/status";
    if (!Firebase.RTDB.setJSON(&_statusFbData, path.c_str(), &json))
    {
        LOG_ERROR("RTDB", "Errore push status: %s",
                  _statusFbData.errorReason().c_str());
    }
}

bool FirebaseManager::_isBackingOff() const
{
    if (_authFailCount < AUTH_FAIL_THRESHOLD)
        return false;
    return (millis() - _lastAuthAttempt) < AUTH_BACKOFF_MS;
}

void FirebaseManager::_initDeviceId()
{
    _deviceId = "scaffale_a";
}
