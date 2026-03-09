// ============================================================
//  WiFiManager.h
//  Libreria per gestione connessione WiFi con:
//    - Connessione con retry e timeout
//    - Riconnessione automatica in background
//    - OTA (aggiornamento firmware via WiFi)
//    - Sincronizzazione NTP
//    - Callback su cambio stato connessione
//
//  Posizione nel progetto PlatformIO:
//  lib/WiFiManager/WiFiManager.h
// ============================================================

#pragma once
#include <Arduino.h>
#include <WiFi.h>
#include <ArduinoOTA.h>
#include <time.h>

// ─────────────────────────────────────────────
//  Stati della connessione WiFi
// ─────────────────────────────────────────────
enum class WiFiState {
  DISCONNECTED,     // non connesso
  CONNECTING,       // tentativo in corso
  CONNECTED,        // connesso alla rete
  CONNECTION_LOST,  // connessione persa
  NTP_SYNCED        // connesso e ora sincronizzata
};

// ─────────────────────────────────────────────
//  Configurazione
// ─────────────────────────────────────────────
struct WiFiConfig {
  const char* ssid;
  const char* password;
  const char* hostname     = "esp32-device";   // nome sulla rete locale
  const char* otaPassword  = "ota1234";         // password aggiornamenti OTA
  bool        enableOTA    = true;
  bool        enableNTP    = true;
  long        gmtOffset    = 3600;              // UTC+1 Italia
  int         daylightOffset = 3600;            // ora legale
  uint8_t     maxRetries   = 20;                // tentativi prima di rinunciare
  uint32_t    retryDelay   = 500;               // ms tra un tentativo e l'altro
  uint32_t    reconnectInterval = 30000;        // ms tra un tentativo di riconnessione
  uint32_t    ntpResyncInterval = 3600000;      // ms tra risincronizzazioni NTP (1h)
};

// ─────────────────────────────────────────────
//  Tipo callback per cambio stato
//  Esempio uso:
//  manager.onStateChange([](WiFiState s) {
//      if (s == WiFiState::CONNECTED) { ... }
//  });
// ─────────────────────────────────────────────
typedef void (*WiFiStateCallback)(WiFiState newState);

// ─────────────────────────────────────────────
//  Classe WiFiManager
// ─────────────────────────────────────────────
class WiFiManager {
  public:
    // ── Costruttore ──────────────────────────
    WiFiManager(const WiFiConfig& config);

    // ── Inizializzazione ─────────────────────
    // Chiama nel setup(). Ritorna true se connesso.
    bool begin();

    // ── Loop ─────────────────────────────────
    // DEVE essere chiamato nel loop() principale
    // Gestisce riconnessione automatica e OTA
    void handle();

    // ── Stato ────────────────────────────────
    bool       isConnected()  const;
    bool       isNTPSynced()  const;
    WiFiState  getState()     const;
    String     getIP()        const;
    int        getRSSI()      const;   // potenza segnale in dBm
    String     getStateStr()  const;

    // ── NTP ──────────────────────────────────
    bool    syncNTP();
    int     getHour()    const;   // -1 se non sincronizzato
    int     getMinute()  const;
    int     getDay()     const;
    String  getTimeStr() const;   // "HH:MM:SS"
    String  getDateStr() const;   // "DD/MM/YYYY"
    time_t  getEpoch()   const;   // unix timestamp

    // ── Callback ─────────────────────────────
    void onStateChange(WiFiStateCallback cb);

    // ── Diagnostica ──────────────────────────
    void printInfo();

  private:
    WiFiConfig          _cfg;
    WiFiState           _state      = WiFiState::DISCONNECTED;
    WiFiState           _lastState  = WiFiState::DISCONNECTED;
    WiFiStateCallback   _callback   = nullptr;
    bool                _ntpSynced  = false;

    uint32_t _lastReconnectAttempt  = 0;
    uint32_t _lastNTPSync           = 0;

    // Metodi interni
    bool _connect();
    void _setupOTA();
    void _setState(WiFiState newState);
    bool _getLocalTime(struct tm& timeinfo) const;
};