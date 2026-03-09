// ============================================================
//  RTDBCommands.cpp
//  Posizione: src/api/RTDBCommands.cpp
// ============================================================

#include "RTDBCommands.h"
#include "utility.h"
#include <Arduino.h>

RTDBCommands::RTDBCommands(SystemConfig& config, SystemState& state)
    : _cfg(config), _state(state)
{}

void RTDBCommands::begin(const String& deviceId) {
    _deviceId = deviceId;
    LOG_SUCCESS("RTDB", "Commands pronto");
}

// ─────────────────────────────────────────────────────────────
//  update()
// ─────────────────────────────────────────────────────────────
void RTDBCommands::update() {
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
//  _startStream()
// ─────────────────────────────────────────────────────────────
void RTDBCommands::_startStream() {
    if (Firebase.RTDB.beginStream(&_stream, _pathCommands().c_str())) {
        _streamStarted = true;
        LOG_SUCCESS("RTDB", "Stream avviato: %s", _pathCommands().c_str());
    } else {
        LOG_ERROR("RTDB", "Errore avvio stream: %s",
                  _stream.errorReason().c_str());
    }
}

// ─────────────────────────────────────────────────────────────
//  _applyStream()
// ─────────────────────────────────────────────────────────────
void RTDBCommands::_applyStream() {
    LOG_DEBUG("RTDB", "Dati ricevuti — path: %s  type: %s",
              _stream.dataPath().c_str(),
              _stream.dataType().c_str());

    FirebaseJson json;

    if (_stream.dataType() == "json") {
        json.setJsonData(_stream.jsonString());
    } else {
        String path = _stream.dataPath();

        
        if (path == "/ventilation/mode") {
            _state.ventilation.mode = _parseMode(_stream.stringData());
            LOG_INFO("RTDB", "Ventilation mode: %s", _stream.stringData().c_str());
            return;
        }
        if (path == "/ventilation/manualSpeed") {
            _state.ventilation.manualSpeed = (uint8_t)constrain(_stream.intData(), 0, 100);
            LOG_INFO("RTDB", "Manual speed: %d%%", _state.ventilation.manualSpeed);
            return;
        }
        if (path == "/light/mode") {
            _state.light.mode = _parseMode(_stream.stringData());
            LOG_INFO("RTDB", "Light mode: %s", _stream.stringData().c_str());
            return;
        }
        if (path == "/light/onHour") {
            _cfg.light.onHour = (uint8_t)constrain(_stream.intData(), 0, 23);
            LOG_INFO("RTDB", "Light onHour: %d", _cfg.light.onHour);
            return;
        }
        if (path == "/light/offHour") {
            _cfg.light.offHour = (uint8_t)constrain(_stream.intData(), 0, 23);
            LOG_INFO("RTDB", "Light offHour: %d", _cfg.light.offHour);
            return;
        }
        if (path == "/thresholds/tempIdealMax") {
            _cfg.ventilation.tempIdealMax = _stream.floatData(); return;
        }
        if (path == "/thresholds/tempWarnMax") {
            _cfg.ventilation.tempWarnMax = _stream.floatData(); return;
        }
        if (path == "/thresholds/tempCritical") {
            _cfg.ventilation.tempCritical = _stream.floatData(); return;
        }
        if (path == "/thresholds/humidityIdealMax") {
            _cfg.ventilation.humidityIdealMax = _stream.floatData(); return;
        }
        if (path == "/thresholds/humidityCritical") {
            _cfg.ventilation.humidityCritical = _stream.floatData(); return;
        }
        return;
    }

    _applyVentilation(json);
    _applyLight(json);
    _applyThresholds(json);
}


// ─────────────────────────────────────────────────────────────
//  _applyVentilation()
// ─────────────────────────────────────────────────────────────
void RTDBCommands::_applyVentilation(FirebaseJson& json) {
    FirebaseJsonData result;

    if (json.get(result, "ventilation/mode")) {
        _state.ventilation.mode = _parseMode(result.stringValue);
        LOG_INFO("RTDB", "Ventilation mode: %s", result.stringValue.c_str());
    }
    if (json.get(result, "ventilation/manualSpeed")) {
        _state.ventilation.manualSpeed = (uint8_t)constrain(result.intValue, 0, 100);
        LOG_INFO("RTDB", "Manual speed: %d%%", _state.ventilation.manualSpeed);
    }
}

// ─────────────────────────────────────────────────────────────
//  _applyLight()
// ─────────────────────────────────────────────────────────────
void RTDBCommands::_applyLight(FirebaseJson& json) {
    FirebaseJsonData result;

    if (json.get(result, "light/mode")) {
        _state.light.mode = _parseMode(result.stringValue);
        LOG_INFO("RTDB", "Light mode: %s", result.stringValue.c_str());
    }
    if (json.get(result, "light/onHour")) {
        _cfg.light.onHour = (uint8_t)constrain(result.intValue, 0, 23);
        LOG_INFO("RTDB", "Light onHour: %d", _cfg.light.onHour);
    }
    if (json.get(result, "light/offHour")) {
        _cfg.light.offHour = (uint8_t)constrain(result.intValue, 0, 23);
        LOG_INFO("RTDB", "Light offHour: %d", _cfg.light.offHour);
    }
}

// ─────────────────────────────────────────────────────────────
//  _applyThresholds()
// ─────────────────────────────────────────────────────────────
void RTDBCommands::_applyThresholds(FirebaseJson& json) {
    FirebaseJsonData result;

    if (json.get(result, "thresholds/tempIdealMax"))
        _cfg.ventilation.tempIdealMax = result.floatValue;
    if (json.get(result, "thresholds/tempWarnMax"))
        _cfg.ventilation.tempWarnMax = result.floatValue;
    if (json.get(result, "thresholds/tempCritical"))
        _cfg.ventilation.tempCritical = result.floatValue;
    if (json.get(result, "thresholds/humidityIdealMax"))
        _cfg.ventilation.humidityIdealMax = result.floatValue;
    if (json.get(result, "thresholds/humidityCritical"))
        _cfg.ventilation.humidityCritical = result.floatValue;

    LOG_INFO("RTDB", "Soglie aggiornate");
}

// ─────────────────────────────────────────────────────────────
//  _parseMode()
// ─────────────────────────────────────────────────────────────
DeviceMode RTDBCommands::_parseMode(const String& modeStr) {
    if (modeStr == "ON")  return DeviceMode::ON;
    if (modeStr == "OFF") return DeviceMode::OFF;
    return DeviceMode::AUTO;
}