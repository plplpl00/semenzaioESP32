#include "WiFiManager.h"

WiFiManager::WiFiManager(const WiFiConfig& config) : _cfg(config) {}

bool WiFiManager::begin() {
  WiFi.setHostname(_cfg.hostname);
  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);
  WiFi.persistent(true);
  bool connected = _connect();
  if (connected) {
    if (_cfg.enableOTA) _setupOTA();
    if (_cfg.enableNTP) syncNTP();
  }
  return connected;
}

void WiFiManager::handle() {
  uint32_t now = millis();
  if (_cfg.enableOTA && isConnected()) ArduinoOTA.handle();
  if (_state == WiFiState::CONNECTED && WiFi.status() != WL_CONNECTED) {
    Serial.println("[WiFi] Connessione persa");
    _ntpSynced = false;
    _setState(WiFiState::CONNECTION_LOST);
  }
  if ((_state == WiFiState::DISCONNECTED || _state == WiFiState::CONNECTION_LOST)
      && (now - _lastReconnectAttempt >= _cfg.reconnectInterval)) {
    _lastReconnectAttempt = now;
    Serial.println("[WiFi] Tentativo di riconnessione...");
    if (_connect()) {
      if (_cfg.enableOTA) _setupOTA();
      if (_cfg.enableNTP) syncNTP();
    }
  }
  if (_ntpSynced && (now - _lastNTPSync >= _cfg.ntpResyncInterval)) syncNTP();
}

bool WiFiManager::_connect() {
  _setState(WiFiState::CONNECTING);
  Serial.printf("[WiFi] Connessione a: %s ", _cfg.ssid);
  WiFi.begin(_cfg.ssid, _cfg.password);
  uint8_t attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < _cfg.maxRetries) {
    delay(_cfg.retryDelay);
    Serial.print(".");
    attempts++;
  }
  if (WiFi.status() == WL_CONNECTED) {
    _setState(WiFiState::CONNECTED);
    Serial.printf("\n[WiFi] Connesso! IP: %s  RSSI: %d dBm\n",
                  WiFi.localIP().toString().c_str(), WiFi.RSSI());
    return true;
  }
  _setState(WiFiState::DISCONNECTED);
  Serial.println("\n[WiFi] Fallita - modalita autonoma attiva");
  return false;
}

void WiFiManager::_setupOTA() {
  ArduinoOTA.setHostname(_cfg.hostname);
  ArduinoOTA.setPassword(_cfg.otaPassword);
  ArduinoOTA.onStart([]() {
    Serial.printf("\n[OTA] Inizio: %s\n",
      (ArduinoOTA.getCommand() == U_FLASH) ? "firmware" : "filesystem");
  });
  ArduinoOTA.onEnd([]()   { Serial.println("[OTA] Completato! Riavvio..."); });
  ArduinoOTA.onProgress([](unsigned int p, unsigned int t) {
    Serial.printf("[OTA] %u%%\r", (p * 100) / t);
  });
  ArduinoOTA.onError([](ota_error_t e) {
    Serial.printf("[OTA] Errore [%u]\n", e);
  });
  ArduinoOTA.begin();
  Serial.printf("[OTA] Pronto. Hostname: %s\n", _cfg.hostname);
}

bool WiFiManager::syncNTP() {
  if (!isConnected()) { Serial.println("[NTP] WiFi non connesso"); return false; }
  Serial.print("[NTP] Sincronizzazione...");
  configTime(_cfg.gmtOffset, _cfg.daylightOffset, "pool.ntp.org", "time.nist.gov");
  struct tm timeinfo;
  if (getLocalTime(&timeinfo, 5000)) {
    _ntpSynced = true; _lastNTPSync = millis();
    _setState(WiFiState::NTP_SYNCED);
    Serial.printf(" OK! %s %s\n", getDateStr().c_str(), getTimeStr().c_str());
    return true;
  }
  _ntpSynced = false;
  Serial.println(" FALLITA");
  return false;
}

void WiFiManager::_setState(WiFiState s) {
  if (s == _state) return;
  _lastState = _state; _state = s;
  if (_callback) _callback(s);
}

bool      WiFiManager::isConnected()  const { return WiFi.status() == WL_CONNECTED; }
bool      WiFiManager::isNTPSynced()  const { return _ntpSynced; }
WiFiState WiFiManager::getState()     const { return _state; }
String    WiFiManager::getIP()        const { return isConnected() ? WiFi.localIP().toString() : "N/A"; }
int       WiFiManager::getRSSI()      const { return isConnected() ? WiFi.RSSI() : 0; }
void      WiFiManager::onStateChange(WiFiStateCallback cb) { _callback = cb; }

String WiFiManager::getStateStr() const {
  switch (_state) {
    case WiFiState::DISCONNECTED:    return "Disconnesso";
    case WiFiState::CONNECTING:      return "Connessione...";
    case WiFiState::CONNECTED:       return "Connesso";
    case WiFiState::CONNECTION_LOST: return "Connessione persa";
    case WiFiState::NTP_SYNCED:      return "Connesso + NTP OK";
    default:                         return "Sconosciuto";
  }
}

bool   WiFiManager::_getLocalTime(struct tm& t) const { return _ntpSynced && getLocalTime(&t); }
int    WiFiManager::getHour()   const { struct tm t; return _getLocalTime(t) ? t.tm_hour : -1; }
int    WiFiManager::getMinute() const { struct tm t; return _getLocalTime(t) ? t.tm_min  : -1; }
int    WiFiManager::getDay()    const { struct tm t; return _getLocalTime(t) ? t.tm_mday : -1; }
time_t WiFiManager::getEpoch()  const { return isNTPSynced() ? time(nullptr) : 0; }

String WiFiManager::getTimeStr() const {
  struct tm t; if (!_getLocalTime(t)) return "--:--:--";
  char buf[9]; snprintf(buf, sizeof(buf), "%02d:%02d:%02d", t.tm_hour, t.tm_min, t.tm_sec);
  return String(buf);
}

String WiFiManager::getDateStr() const {
  struct tm t; if (!_getLocalTime(t)) return "--/--/----";
  char buf[11]; snprintf(buf, sizeof(buf), "%02d/%02d/%04d", t.tm_mday, t.tm_mon+1, t.tm_year+1900);
  return String(buf);
}

void WiFiManager::printInfo() {
  Serial.println("--- WiFiManager Info -----------");
  Serial.printf("  SSID     : %s\n",     _cfg.ssid);
  Serial.printf("  Hostname : %s\n",     _cfg.hostname);
  Serial.printf("  Stato    : %s\n",     getStateStr().c_str());
  Serial.printf("  IP       : %s\n",     getIP().c_str());
  Serial.printf("  RSSI     : %d dBm\n", getRSSI());
  Serial.printf("  OTA      : %s\n",     _cfg.enableOTA ? "Abilitato" : "Disabilitato");
  Serial.printf("  NTP      : %s\n",     _ntpSynced ? "OK" : "Non sincronizzato");
  if (_ntpSynced) Serial.printf("  Ora      : %s %s\n", getDateStr().c_str(), getTimeStr().c_str());
  Serial.println("--------------------------------");
}