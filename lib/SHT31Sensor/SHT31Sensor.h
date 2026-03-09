// ============================================================
//  SHT31Sensor.h  -  lib/SHT31Sensor/SHT31Sensor.h
//  Responsabilita unica: lettura temperatura + umidità SHT31.
//  Comunicazione I2C, indirizzo configurabile (default 0x44).
// ============================================================

#pragma once
#include <Arduino.h>
#include <Wire.h>

// ─────────────────────────────────────────────
//  Codici di errore
// ─────────────────────────────────────────────
enum class SHT31Error {
  NONE,
  NOT_CONNECTED,   // sensore non risponde sul bus I2C
  CRC_FAILED,      // checksum dati non valido
  OUT_OF_RANGE,    // valore fuori dal range configurato
  READ_FAILED      // lettura fallita dopo tutti i tentativi
};

// ─────────────────────────────────────────────
//  Risultato lettura — temperatura + umidità
//  sempre insieme (unica transazione I2C)
// ─────────────────────────────────────────────
struct SHT31Reading {
  float      temperature;   // °C
  float      humidity;      // %RH
  SHT31Error error;

  bool isValid()  const { return error == SHT31Error::NONE; }
  bool hasError() const { return error != SHT31Error::NONE; }
};

// ─────────────────────────────────────────────
//  Classe SHT31Sensor
// ─────────────────────────────────────────────
class SHT31Sensor {
  public:
    // address: 0x44 (ADR a GND, default)
    //          0x45 (ADR a VCC)
    SHT31Sensor(uint8_t address = 0x44);

    // Inizializza Wire e verifica presenza sensore
    bool begin();

    // Lettura temperatura + umidità in una sola transazione
    SHT31Reading read();

    // Stato
    bool        isConnected();
    SHT31Error  getLastError()    const;
    const char* getLastErrorMsg() const;

    // Configurazione range di validazione
    // Fuori range → error = OUT_OF_RANGE
    void setTempRange    (float minTemp, float maxTemp);
    void setHumidityRange(float minHum,  float maxHum);
    void setRetryCount   (uint8_t retries);

    static const char* errorToString(SHT31Error err);

  private:
    uint8_t    _address;
    float      _minTemp    = -10.0f;
    float      _maxTemp    =  60.0f;
    float      _minHum     =   0.0f;
    float      _maxHum     = 100.0f;
    uint8_t    _retries    = 3;
    SHT31Error _lastError  = SHT31Error::NONE;

    // Legge 6 byte raw dal sensore (temp MSB/LSB/CRC + hum MSB/LSB/CRC)
    bool         _readRaw(uint16_t& rawTemp, uint16_t& rawHum);
    bool         _checkCRC(uint8_t data[2], uint8_t crc);
    SHT31Reading _readWithRetry();
    SHT31Error   _validate(float temp, float hum);
};