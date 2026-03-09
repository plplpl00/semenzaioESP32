// ============================================================
//  SHT31Sensor.cpp  -  lib/SHT31Sensor/SHT31Sensor.cpp
// ============================================================

#include "SHT31Sensor.h"

// ─────────────────────────────────────────────
//  Comandi SHT31 (Single Shot, High Repeatability)
// ─────────────────────────────────────────────
static const uint8_t CMD_SINGLE_HIGH_MSB = 0x2C;
static const uint8_t CMD_SINGLE_HIGH_LSB = 0x06;
static const uint8_t CMD_SOFT_RESET_MSB  = 0x30;
static const uint8_t CMD_SOFT_RESET_LSB  = 0xA2;

// ─────────────────────────────────────────────
//  Costruttore
// ─────────────────────────────────────────────
SHT31Sensor::SHT31Sensor(uint8_t address) : _address(address) {}

// ─────────────────────────────────────────────
//  Inizializzazione
// ─────────────────────────────────────────────
bool SHT31Sensor::begin() {
  Wire.begin();

  // Soft reset per portare il chip in stato noto
  Wire.beginTransmission(_address);
  Wire.write(CMD_SOFT_RESET_MSB);
  Wire.write(CMD_SOFT_RESET_LSB);
  uint8_t err = Wire.endTransmission();

  if (err != 0) {
    _lastError = SHT31Error::NOT_CONNECTED;
    return false;
  }

  delay(10); // attesa reset (datasheet: max 1.5ms)

  _lastError = SHT31Error::NONE;
  return true;
}

// ─────────────────────────────────────────────
//  Lettura pubblica
// ─────────────────────────────────────────────
SHT31Reading SHT31Sensor::read() {
  SHT31Reading result = _readWithRetry();
  _lastError = result.error;
  return result;
}

// ─────────────────────────────────────────────
//  Lettura interna con retry
// ─────────────────────────────────────────────
SHT31Reading SHT31Sensor::_readWithRetry() {
  SHT31Reading result = { 0.0f, 0.0f, SHT31Error::READ_FAILED };

  for (uint8_t attempt = 0; attempt < _retries; attempt++) {
    uint16_t rawTemp = 0;
    uint16_t rawHum  = 0;

    if (!_readRaw(rawTemp, rawHum)) {
      result.error = SHT31Error::NOT_CONNECTED;
      delay(50);
      continue;
    }

    // Conversione secondo datasheet Sensirion
    // T [°C] = -45 + 175 * rawTemp / 65535
    // RH [%] = 100 * rawHum / 65535
    float temp = -45.0f + 175.0f * ((float)rawTemp / 65535.0f);
    float hum  = 100.0f * ((float)rawHum  / 65535.0f);

    SHT31Error validErr = _validate(temp, hum);
    if (validErr != SHT31Error::NONE) {
      result.error = validErr;
      delay(50);
      continue;
    }

    result.temperature = temp;
    result.humidity    = hum;
    result.error       = SHT31Error::NONE;
    return result;
  }

  return result;
}

// ─────────────────────────────────────────────
//  Lettura 6 byte raw dal sensore
// ─────────────────────────────────────────────
bool SHT31Sensor::_readRaw(uint16_t& rawTemp, uint16_t& rawHum) {
  // Invia comando Single Shot High Repeatability
  Wire.beginTransmission(_address);
  Wire.write(CMD_SINGLE_HIGH_MSB);
  Wire.write(CMD_SINGLE_HIGH_LSB);
  if (Wire.endTransmission() != 0) return false;

  // Attesa conversione (datasheet: max 15ms per high repeatability)
  delay(16);

  // Leggi 6 byte: [T_MSB][T_LSB][T_CRC][H_MSB][H_LSB][H_CRC]
  uint8_t received = Wire.requestFrom(_address, (uint8_t)6);
  if (received != 6) return false;

  uint8_t tMSB = Wire.read();
  uint8_t tLSB = Wire.read();
  uint8_t tCRC = Wire.read();
  uint8_t hMSB = Wire.read();
  uint8_t hLSB = Wire.read();
  uint8_t hCRC = Wire.read();

  // Verifica CRC temperatura
  uint8_t tData[2] = { tMSB, tLSB };
  if (!_checkCRC(tData, tCRC)) {
    _lastError = SHT31Error::CRC_FAILED;
    return false;
  }

  // Verifica CRC umidità
  uint8_t hData[2] = { hMSB, hLSB };
  if (!_checkCRC(hData, hCRC)) {
    _lastError = SHT31Error::CRC_FAILED;
    return false;
  }

  rawTemp = ((uint16_t)tMSB << 8) | tLSB;
  rawHum  = ((uint16_t)hMSB << 8) | hLSB;
  return true;
}

// ─────────────────────────────────────────────
//  CRC-8 Sensirion (polynomial 0x31, init 0xFF)
// ─────────────────────────────────────────────
bool SHT31Sensor::_checkCRC(uint8_t data[2], uint8_t crc) {
  uint8_t computed = 0xFF;
  for (uint8_t i = 0; i < 2; i++) {
    computed ^= data[i];
    for (uint8_t bit = 0; bit < 8; bit++) {
      computed = (computed & 0x80)
                  ? (computed << 1) ^ 0x31
                  : (computed << 1);
    }
  }
  return computed == crc;
}

// ─────────────────────────────────────────────
//  Validazione
// ─────────────────────────────────────────────
SHT31Error SHT31Sensor::_validate(float temp, float hum) {
  if (temp < _minTemp || temp > _maxTemp) return SHT31Error::OUT_OF_RANGE;
  if (hum  < _minHum  || hum  > _maxHum)  return SHT31Error::OUT_OF_RANGE;
  return SHT31Error::NONE;
}

// ─────────────────────────────────────────────
//  Stato
// ─────────────────────────────────────────────
bool SHT31Sensor::isConnected() {
  Wire.beginTransmission(_address);
  return Wire.endTransmission() == 0;
}

SHT31Error   SHT31Sensor::getLastError()    const { return _lastError; }
const char*  SHT31Sensor::getLastErrorMsg() const { return errorToString(_lastError); }

// ─────────────────────────────────────────────
//  Configurazione
// ─────────────────────────────────────────────
void SHT31Sensor::setTempRange(float minTemp, float maxTemp) {
  _minTemp = minTemp;
  _maxTemp = maxTemp;
}

void SHT31Sensor::setHumidityRange(float minHum, float maxHum) {
  _minHum = minHum;
  _maxHum = maxHum;
}

void SHT31Sensor::setRetryCount(uint8_t retries) {
  _retries = max((uint8_t)1, retries);
}

// ─────────────────────────────────────────────
//  Messaggi errore
// ─────────────────────────────────────────────
const char* SHT31Sensor::errorToString(SHT31Error err) {
  switch (err) {
    case SHT31Error::NONE:          return "Nessun errore";
    case SHT31Error::NOT_CONNECTED: return "Sensore non connesso";
    case SHT31Error::CRC_FAILED:    return "Errore CRC";
    case SHT31Error::OUT_OF_RANGE:  return "Valore fuori range";
    case SHT31Error::READ_FAILED:   return "Lettura fallita";
    default:                        return "Errore sconosciuto";
  }
}