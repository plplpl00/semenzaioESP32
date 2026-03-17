// ============================================================
//  FirestorePublisher.cpp
//  Posizione: src/6.api/FirestorePublisher.cpp
//
//  Solo storico: un documento readings/{timestamp} ogni 2 min.
// ============================================================
#include <Arduino.h>
#include <time.h>
#include "FirestorePublisher.h"
#include "utility.h"
#include "secrets.h"

FirestorePublisher::FirestorePublisher(SystemState &state)
    : _state(state)
{
}

void FirestorePublisher::begin(const String &deviceId)
{
    _deviceId = deviceId;
    LOG_SUCCESS("Firestore", "Publisher pronto (solo storico)");
}

void FirestorePublisher::update() {
    _pushReading();
}


void FirestorePublisher::_pushReading()
{
    FirebaseJson doc;
    _buildReadingDoc(doc);

    String docPath = _pathReadings();
    docPath += "/";
    docPath += _getTimestampId();

    if (Firebase.Firestore.createDocument(&_fbData,
                                          FIREBASE_PROJECT_ID, "",
                                          docPath.c_str(), doc.raw()))
    {
        LOG_SUCCESS("Firestore", "Reading: %s", _getTimestampId().c_str());
    }
    else
    {
        LOG_ERROR("Firestore", "Errore reading: %s",
                  _fbData.errorReason().c_str());
    }
}

void FirestorePublisher::_buildReadingDoc(FirebaseJson &doc)
{
    auto &e = _state.environment;
    auto &v = _state.ventilation;
    auto &l = _state.light;

    doc.set("fields/temperature/doubleValue", e.temperature);
    doc.set("fields/tempExternal/doubleValue", e.tempExternal);
    doc.set("fields/tempDelta/doubleValue", e.tempDelta);
    doc.set("fields/humidity/doubleValue", e.humidity);
    doc.set("fields/fanSpeed/integerValue", (int)v.speedPercent);
    doc.set("fields/fanRPM/integerValue", (int)v.rpm);
    doc.set("fields/fanCause/integerValue", (int)v.cause);
    doc.set("fields/lightOn/booleanValue",
            l.state == LightState::ON || l.state == LightState::ON_MANUAL);
    doc.set("fields/timestamp/stringValue", _getTimestampId());
}

String FirestorePublisher::_getTimestampId()
{
    struct tm timeInfo;
    if (!getLocalTime(&timeInfo))
        return String(millis());
    char buf[20];
    strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%S", &timeInfo);
    return String(buf);
}
