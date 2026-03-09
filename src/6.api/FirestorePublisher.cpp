// ============================================================
//  FirestorePublisher.cpp
//  Posizione: src/api/FirestorePublisher.cpp
// ============================================================

#include "FirestorePublisher.h"
#include "utility.h"
#include "secrets.h"
#include <Arduino.h>
#include <time.h>

FirestorePublisher::FirestorePublisher(SystemConfig& config, SystemState& state)
    : _cfg(config), _state(state)
{}

void FirestorePublisher::begin(const String& deviceId) {
    _deviceId = deviceId;
    LOG_SUCCESS("Firestore", "Publisher pronto");
}

void FirestorePublisher::update() {
    uint32_t now = millis();
    if (now - _lastPushMs < _cfg.firebase.pushInterval) return;
    _lastPushMs = now;

    _pushState();
    _pushReading();
}

// ─────────────────────────────────────────────────────────────
//  _pushState()
// ─────────────────────────────────────────────────────────────
void FirestorePublisher::_pushState() {
    FirebaseJson doc;
    _buildStateDoc(doc);

    if (Firebase.Firestore.patchDocument(&_fbData,
            FIREBASE_PROJECT_ID, "",
            _pathState().c_str(),
            doc.raw(), "")) {
        LOG_SUCCESS("Firestore", "State aggiornato");
    } else {
        LOG_ERROR("Firestore", "Errore push state: %s",
                  _fbData.errorReason().c_str());
        _state.firebaseOnline = false;
    }
}

// ─────────────────────────────────────────────────────────────
//  _pushReading()
// ─────────────────────────────────────────────────────────────
void FirestorePublisher::_pushReading() {
    FirebaseJson doc;
    _buildReadingDoc(doc);

    String docId   = _getTimestampId();
    String docPath = _pathReadings();
    docPath += "/";
    docPath += docId;

    if (Firebase.Firestore.createDocument(&_fbData,
            FIREBASE_PROJECT_ID, "",
            docPath.c_str(),
            doc.raw())) {
        LOG_SUCCESS("Firestore", "Reading salvato: %s", docId.c_str());
    } else {
        LOG_ERROR("Firestore", "Errore push reading: %s",
                  _fbData.errorReason().c_str());
    }
}

// ─────────────────────────────────────────────────────────────
//  _buildStateDoc()
// ─────────────────────────────────────────────────────────────
void FirestorePublisher::_buildStateDoc(FirebaseJson& doc) {
    auto& e = _state.environment;
    auto& v = _state.ventilation;
    auto& l = _state.light;

    doc.set("fields/temperature/doubleValue",    e.temperature);
    doc.set("fields/humidity/doubleValue",       e.humidity);
    doc.set("fields/tempSHT31/doubleValue",      e.tempSHT31);
    doc.set("fields/tempValid/booleanValue",     e.tempStatus     == SensorStatus::OK);
    doc.set("fields/humidityValid/booleanValue", e.humidityStatus == SensorStatus::OK);

    doc.set("fields/fanSpeed/integerValue",      (int)v.speedPercent);
    doc.set("fields/fanRPM/integerValue",        (int)v.rpm);
    doc.set("fields/fanStalled/booleanValue",    v.stalled);
    doc.set("fields/fanCause/integerValue",      (int)v.cause);
    doc.set("fields/fanMode/integerValue",       (int)v.mode);

    bool lightOn = (l.state == LightState::ON || l.state == LightState::ON_MANUAL);
    doc.set("fields/lightOn/booleanValue",       lightOn);
    doc.set("fields/lightState/integerValue",    (int)l.state);
    doc.set("fields/lightMode/integerValue",     (int)l.mode);
    doc.set("fields/lightOnHour/integerValue",   (int)l.onHour);
    doc.set("fields/lightOffHour/integerValue",  (int)l.offHour);

    doc.set("fields/wifiConnected/booleanValue", _state.wifiConnected);
    doc.set("fields/ntpSynced/booleanValue",     _state.ntpSynced);
    doc.set("fields/uptimeMs/integerValue",      (int)millis());
}

// ─────────────────────────────────────────────────────────────
//  _buildReadingDoc()
// ─────────────────────────────────────────────────────────────
void FirestorePublisher::_buildReadingDoc(FirebaseJson& doc) {
    auto& e = _state.environment;
    auto& v = _state.ventilation;

    doc.set("fields/temperature/doubleValue", e.temperature);
    doc.set("fields/humidity/doubleValue",    e.humidity);
    doc.set("fields/fanSpeed/integerValue",   (int)v.speedPercent);
    doc.set("fields/fanRPM/integerValue",     (int)v.rpm);
    doc.set("fields/fanCause/integerValue",   (int)v.cause);
    doc.set("fields/timestamp/stringValue",   _getTimestampId());
}

// ─────────────────────────────────────────────────────────────
//  _getTimestampId()
// ─────────────────────────────────────────────────────────────
String FirestorePublisher::_getTimestampId() {
    struct tm timeInfo;
    if (!getLocalTime(&timeInfo)) return String(millis());
    char buf[20];
    strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%S", &timeInfo);
    return String(buf);
}