// ============================================================
//  FirestorePublisher.cpp
//  Posizione: src/6.api/FirestorePublisher.cpp
// ============================================================
#include <Arduino.h>
#include <time.h>
#include "FirestorePublisher.h"
#include "utility.h"
#include "secrets.h"

FirestorePublisher::FirestorePublisher(SystemState& state)
    : _state(state)
{}

void FirestorePublisher::begin(const String& deviceId) {
    _deviceId = deviceId;
    LOG_SUCCESS("Firestore", "Publisher pronto");
}

void FirestorePublisher::update() {
    uint32_t now = millis();
    if (now - _lastPushMs < FIREBASE_PUSH_INTERVAL) return;
    _lastPushMs = now;

    _pushState();
    _pushReading();
}

void FirestorePublisher::_pushState() {
    FirebaseJson doc;
    _buildStateDoc(doc);

    if (Firebase.Firestore.patchDocument(&_fbData,
            FIREBASE_PROJECT_ID, "",
            _pathState().c_str(), doc.raw(), "")) {
        LOG_SUCCESS("Firestore", "State aggiornato");
    } else {
        LOG_ERROR("Firestore", "Errore state: %s",
                  _fbData.errorReason().c_str());
        _state.firebaseOnline = false;
    }
}

void FirestorePublisher::_pushReading() {
    FirebaseJson doc;
    _buildReadingDoc(doc);

    String docPath = _pathReadings() + "/" + _getTimestampId();

    if (Firebase.Firestore.createDocument(&_fbData,
            FIREBASE_PROJECT_ID, "",
            docPath.c_str(), doc.raw())) {
        LOG_SUCCESS("Firestore", "Reading: %s", _getTimestampId().c_str());
    } else {
        LOG_ERROR("Firestore", "Errore reading: %s",
                  _fbData.errorReason().c_str());
    }
}

void FirestorePublisher::_buildStateDoc(FirebaseJson& doc) {
    auto& e = _state.environment;
    auto& v = _state.ventilation;
    auto& l = _state.light;
    auto& r = _state.recipe;

    // Ambiente
    doc.set("fields/temperature/doubleValue",       e.temperature);
    doc.set("fields/tempExternal/doubleValue",      e.tempExternal);
    doc.set("fields/tempDelta/doubleValue",         e.tempDelta);
    doc.set("fields/humidity/doubleValue",          e.humidity);
    doc.set("fields/tempValid/booleanValue",        e.tempStatus == SensorStatus::OK);
    doc.set("fields/tempExtValid/booleanValue",     e.tempExtStatus == SensorStatus::OK);
    doc.set("fields/humidityValid/booleanValue",    e.humidityStatus == SensorStatus::OK);

    // Ventilazione
    doc.set("fields/fanSpeed/integerValue",         (int)v.speedPercent);
    doc.set("fields/fanRPM/integerValue",           (int)v.rpm);
    doc.set("fields/fanStalled/booleanValue",       v.stalled);
    doc.set("fields/fanCause/integerValue",         (int)v.cause);
    doc.set("fields/fanMode/integerValue",          (int)v.mode);
    doc.set("fields/fanSpeedFromTemp/integerValue", (int)v.speedFromTemp);
    doc.set("fields/fanSpeedFromHum/integerValue",  (int)v.speedFromHum);
    doc.set("fields/fanTempRampActive/booleanValue", v.tempRampActive);

    // Luce
    bool lightOn = (l.state == LightState::ON || l.state == LightState::ON_MANUAL);
    doc.set("fields/lightOn/booleanValue",          lightOn);
    doc.set("fields/lightState/integerValue",       (int)l.state);
    doc.set("fields/lightMode/integerValue",        (int)l.mode);

    // Ricetta
    doc.set("fields/cycleActive/booleanValue",      r.active);
    doc.set("fields/recipeName/stringValue",        String(r.recipeName));
    doc.set("fields/dayOfCycle/integerValue",       (int)r.dayOfCycle);

    // Sistema
    doc.set("fields/wifiConnected/booleanValue",    _state.wifiConnected);
    doc.set("fields/ntpSynced/booleanValue",        _state.ntpSynced);
    doc.set("fields/uptimeMs/integerValue",         (int)millis());
}

void FirestorePublisher::_buildReadingDoc(FirebaseJson& doc) {
    auto& e = _state.environment;
    auto& v = _state.ventilation;

    doc.set("fields/temperature/doubleValue",       e.temperature);
    doc.set("fields/tempExternal/doubleValue",      e.tempExternal);
    doc.set("fields/humidity/doubleValue",          e.humidity);
    doc.set("fields/fanSpeed/integerValue",         (int)v.speedPercent);
    doc.set("fields/fanCause/integerValue",         (int)v.cause);
    doc.set("fields/timestamp/stringValue",         _getTimestampId());
}

String FirestorePublisher::_getTimestampId() {
    struct tm timeInfo;
    if (!getLocalTime(&timeInfo)) return String(millis());
    char buf[20];
    strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%S", &timeInfo);
    return String(buf);
}
