// ============================================================
//  DS18B20Sensor.h  -  lib/DS18B20Sensor/DS18B20Sensor.h
//  Responsabilita unica: lettura temperatura con gestione errori.
//  Per la scansione indirizzi usa DS18B20Scanner.
// ============================================================

#pragma once
#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// ─────────────────────────────────────────────
//  Codici di errore
// ─────────────────────────────────────────────
enum class TempError {
  NONE,
  NOT_CONNECTED,
  CRC_FAILED,
  OUT_OF_RANGE,
  READ_FAILED
};

// ─────────────────────────────────────────────
//  Risultato lettura
// ─────────────────────────────────────────────
struct TempReading {
  float     value;
  TempError error;

  bool isValid()  const { return error == TempError::NONE; }
  bool hasError() const { return error != TempError::NONE; }
};

// ─────────────────────────────────────────────
//  Classe DS18B20Sensor
// ─────────────────────────────────────────────
class DS18B20Sensor {
  public:
    // Singolo sensore sul bus - usa indice 0
    DS18B20Sensor(uint8_t pin);

    // Sensore specifico - usa indirizzo trovato con DS18B20Scanner
    DS18B20Sensor(uint8_t pin, DeviceAddress address);

    bool        begin();
    TempReading read();
    float       readRaw();

    bool        isConnected();
    TempError   getLastError()    const;
    const char* getLastErrorMsg() const;

    // Risoluzione: 9-12 bit (default 12 = 0.0625 C, 750ms)
    void setResolution(uint8_t bits);
    void setValidRange(float minTemp, float maxTemp);
    void setRetryCount(uint8_t retries);

    void        printInfo();
    static const char* errorToString(TempError err);

  private:
    OneWire           _oneWire;
    DallasTemperature _sensors;
    bool              _useAddress = false;
    DeviceAddress     _address;
    float             _minTemp    = -55.0;
    float             _maxTemp    = 125.0;
    uint8_t           _retries    = 3;
    TempError         _lastError  = TempError::NONE;

    TempReading _readWithRetry();
    TempError   _validate(float value);
};