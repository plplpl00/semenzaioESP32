// ============================================================
//  RTDBCommands.cpp
//  Posizione: src/6.api/RTDBCommands.cpp
// ============================================================
#include <Arduino.h>
#include "RTDBCommands.h"
#include "utility.h"

RTDBCommands::RTDBCommands(SystemState& state)
    : _state(state)
{}

void RTDBCommands::begin(const String& deviceId) {
    _deviceId = deviceId;
    LOG_SUCCESS("RTDB", "Commands pronto — shelf 0");
}

void RTDBCommands::update() {
    // ── Prima lettura completa al boot ───────────────────
    if (!_initialLoad) {
        _loadInitialData();
        return;
    }

    // ── Streaming ────────────────────────────────────────
    if (!_streamStarted) {
        _startStream();
        return;
    }

    if (!_stream.httpConnected()) {
        LOG_WARNING("RTDB", "Stream caduto — riavvio");
        _streamStarted = false;
        return;
    }

    if (Firebase.RTDB.readStream(&_stream)) {
        if (_stream.streamAvailable()) {
            _applyStream();
        }
    }
}

// ─────────────────────────────────────────────────────────────
//  _loadInitialData() — lettura singola al boot
// ─────────────────────────────────────────────────────────────
void RTDBCommands::_loadInitialData() {
    FirebaseData fbData;
    String path = _basePath();

    if (!Firebase.RTDB.getJSON(&fbData, path.c_str())) {
        LOG_ERROR("RTDB", "Lettura iniziale fallita: %s",
                  fbData.errorReason().c_str());
        return;
    }

    FirebaseJson json;
    json.setJsonData(fbData.jsonString());

    // ── Mode ─────────────────────────────────────────────
    FirebaseJsonData result;
    if (json.get(result, "mode")) {
        _state.ventilation.mode = _stringToMode(result.stringValue);
        _state.light.mode       = _stringToMode(result.stringValue);
        LOG_INFO("RTDB", "Mode iniziale: %s", result.stringValue.c_str());
    }

    // ── Cycle ────────────────────────────────────────────
    FirebaseJsonData cycleResult;
    if (json.get(cycleResult, "cycle")) {
        FirebaseJson cycleJson;
        cycleJson.setJsonData(cycleResult.stringValue.length() > 0
            ? cycleResult.stringValue : "{}");

        // Riestrai cycle come sotto-oggetto
        String cycleStr;
        json.toString(cycleStr);
        FirebaseJson fullJson;
        fullJson.setJsonData(fbData.jsonString());
        _parseCycle(fullJson);
    }

    // ── Safety ───────────────────────────────────────────
    _parseSafety(json);

    _initialLoad = true;
    LOG_SUCCESS("RTDB", "Dati iniziali caricati — ciclo %s",
                _state.recipe.active ? "ATTIVO" : "inattivo");
}

// ─────────────────────────────────────────────────────────────
//  _startStream()
// ─────────────────────────────────────────────────────────────
void RTDBCommands::_startStream() {
    String path = _basePath();
    if (Firebase.RTDB.beginStream(&_stream, path.c_str())) {
        _streamStarted = true;
        LOG_SUCCESS("RTDB", "Stream avviato: %s", path.c_str());
    } else {
        LOG_ERROR("RTDB", "Errore avvio stream: %s",
                  _stream.errorReason().c_str());
    }
}

// ─────────────────────────────────────────────────────────────
//  _applyStream()
// ─────────────────────────────────────────────────────────────
void RTDBCommands::_applyStream() {
    String path = _stream.dataPath();
    String type = _stream.dataType();

    LOG_DEBUG("RTDB", "Stream: path=%s type=%s", path.c_str(), type.c_str());

     // ── Ignora aggiornamenti su /status (evita loop) ─────
    if (path.startsWith("/status")) return;

    // ── Aggiornamento singolo valore ─────────────────────
    if (type != "json") {
        if (path == "/mode") {
            _parseMode(path, _stream.stringData());
        }
        return;
    }

    // ── Aggiornamento JSON ───────────────────────────────
    FirebaseJson json;
    json.setJsonData(_stream.jsonString());

    if (path == "/" || path == "") {
        // Aggiornamento completo del nodo shelf
        FirebaseJsonData result;
        if (json.get(result, "mode")) {
            _state.ventilation.mode = _stringToMode(result.stringValue);
            _state.light.mode       = _stringToMode(result.stringValue);
        }
        _parseCycle(json);
        _parseSafety(json);
        LOG_INFO("RTDB", "Aggiornamento completo ricevuto");
    }
    else if (path == "/cycle") {
        _parseCycle(json);
        LOG_INFO("RTDB", "Ciclo aggiornato — giorno %d",
                 _state.recipe.dayOfCycle);
    }
    else if (path == "/safety") {
        _parseSafety(json);
        LOG_INFO("RTDB", "Safety aggiornato");
    }
}

// ─────────────────────────────────────────────────────────────
//  _parseMode()
// ─────────────────────────────────────────────────────────────
void RTDBCommands::_parseMode(const String& path, const String& value) {
    DeviceMode mode = _stringToMode(value);
    _state.ventilation.mode = mode;
    _state.light.mode       = mode;
    _state.commandReceived  = true;  // trigger push immediato status
    LOG_INFO("RTDB", "Mode → %s", value.c_str());
}

// ─────────────────────────────────────────────────────────────
//  _parseCycle()
// ─────────────────────────────────────────────────────────────
void RTDBCommands::_parseCycle(FirebaseJson& json) {
    FirebaseJsonData r;
    RecipeParams& rp = _state.recipe;

    // Campi base
    if (json.get(r, "cycle/active"))       rp.active      = r.boolValue;
    if (json.get(r, "cycle/dayOfCycle"))    rp.dayOfCycle  = (uint8_t)r.intValue;
    if (json.get(r, "cycle/lightOnHour"))   rp.lightOnHour = (uint8_t)constrain(r.intValue, 0, 23);
    if (json.get(r, "cycle/lightOffHour"))  rp.lightOffHour = (uint8_t)constrain(r.intValue, 0, 23);

    if (json.get(r, "cycle/recipeName")) {
        strncpy(rp.recipeName, r.stringValue.c_str(), sizeof(rp.recipeName) - 1);
        rp.recipeName[sizeof(rp.recipeName) - 1] = '\0';
    }

    // Parametri giorno
    _parseClimateParams(json, "cycle/day", rp.day);

    // Parametri notte
    _parseClimateParams(json, "cycle/night", rp.night);

    // Irrigazione
    _parseIrrigations(json, rp.irrigations);
}

// ─────────────────────────────────────────────────────────────
//  _parseClimateParams()
// ─────────────────────────────────────────────────────────────
void RTDBCommands::_parseClimateParams(FirebaseJson& json,
                                        const String& prefix,
                                        ClimateParams& params) {
    FirebaseJsonData r;
    String p = prefix;
    p += "/";

    String key;
    key = p; key += "tempThreshold";  if (json.get(r, key.c_str()))  params.tempThreshold = r.floatValue;
    key = p; key += "tempMaxAlarm";   if (json.get(r, key.c_str()))  params.tempMaxAlarm  = r.floatValue;
    key = p; key += "humThreshold";   if (json.get(r, key.c_str()))  params.humThreshold  = r.floatValue;
    key = p; key += "humMaxAlarm";    if (json.get(r, key.c_str()))  params.humMaxAlarm   = r.floatValue;
    key = p; key += "speedMin";       if (json.get(r, key.c_str()))  params.speedMin      = (uint8_t)r.intValue;
    key = p; key += "speedMax";       if (json.get(r, key.c_str()))  params.speedMax      = (uint8_t)r.intValue;
}



// ─────────────────────────────────────────────────────────────
//  _parseIrrigations()
// ─────────────────────────────────────────────────────────────
void RTDBCommands::_parseIrrigations(FirebaseJson& json,
                                      IrrigationParams& params) {
    FirebaseJsonData r;

    if (json.get(r, "cycle/irrigations/count"))
        params.count = (uint8_t)constrain(r.intValue, 0, 4);
    if (json.get(r, "cycle/irrigations/durationSec"))
        params.durationSec = (uint16_t)r.intValue;

    // Parse orari irrigazione
    for (uint8_t i = 0; i < params.count && i < 4; i++) {
        String timePath = "cycle/irrigations/times/";
        timePath += String(i);
        if (json.get(r, timePath.c_str())) {
            // Parse "HH:MM"
            String timeStr = r.stringValue;
            int colonIdx = timeStr.indexOf(':');
            if (colonIdx > 0) {
                params.hours[i]   = (uint8_t)timeStr.substring(0, colonIdx).toInt();
                params.minutes[i] = (uint8_t)timeStr.substring(colonIdx + 1).toInt();
            }
        }
    }
}

// ─────────────────────────────────────────────────────────────
//  _parseSafety()
// ─────────────────────────────────────────────────────────────
void RTDBCommands::_parseSafety(FirebaseJson& json) {
    FirebaseJsonData r;
    SafetyParams& sp = _state.recipe.safety;

    if (json.get(r, "safety/externalTempMax"))  sp.externalTempMax = r.floatValue;
    if (json.get(r, "safety/deltaMinForVent"))  sp.deltaMinForVent = r.floatValue;
    if (json.get(r, "safety/offlineSpeedMin"))  sp.offlineSpeedMin = (uint8_t)r.intValue;
    if (json.get(r, "safety/offlineLightOff"))  sp.offlineLightOff = r.boolValue;
}

// ─────────────────────────────────────────────────────────────
//  _stringToMode()
// ─────────────────────────────────────────────────────────────
DeviceMode RTDBCommands::_stringToMode(const String& str) {
    if (str == "ON")  return DeviceMode::ON;
    if (str == "OFF") return DeviceMode::OFF;
    return DeviceMode::AUTO;
}
