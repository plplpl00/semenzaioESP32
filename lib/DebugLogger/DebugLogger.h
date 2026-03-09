// ============================================================
//  DebugLogger.h  -  lib/DebugLogger/DebugLogger.h
// ============================================================
#pragma once
#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <vector>

enum class LogLevel { DEBUG, INFO, WARNING, ERROR, SUCCESS };

class DebugLogger {
  public:
    DebugLogger(uint16_t port = 80);
    void begin();
    void handle();  // lasciato per compatibilita, AsyncWebServer e' gia non-bloccante

    void log(LogLevel level, const String& tag, const String& message);
    void debug(const String& tag, const String& msg);
    void info(const String& tag, const String& msg);
    void warning(const String& tag, const String& msg);
    void error(const String& tag, const String& msg);
    void success(const String& tag, const String& msg);
    void logf(LogLevel level, const String& tag, const char* format, ...);

    bool     isClientConnected() const;
    uint8_t  getClientCount()    const;
    String   getURL()            const;

    void setMirrorToSerial(bool enable);
    void setHistorySize(uint8_t lines);

  private:
    AsyncWebServer  _server;
    AsyncWebSocket  _ws;
    uint16_t        _port;
    bool            _mirrorSerial = true;
    uint8_t         _historySize  = 50;

    std::vector<String> _history;

    void        _onWebSocketEvent(AsyncWebSocket* server, AsyncWebSocketClient* client,
                                  AwsEventType type, void* arg, uint8_t* data, size_t len);
    String      _buildJson(LogLevel level, const String& tag,
                           const String& msg, const char* time);
    static const char* _levelName(LogLevel level);
};

extern DebugLogger Debug;