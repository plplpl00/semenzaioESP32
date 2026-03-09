// ============================================================
//  DS18B20Sensor.cpp  -  lib/DS18B20Sensor/DS18B20Sensor.cpp
// ============================================================

#include "DS18B20Sensor.h"

// ─────────────────────────────────────────────
//  Costruttori
// ─────────────────────────────────────────────
DS18B20Sensor::DS18B20Sensor(uint8_t pin)
    : _oneWire(pin), _sensors(&_oneWire), _useAddress(false) {}

DS18B20Sensor::DS18B20Sensor(uint8_t pin, DeviceAddress address)
    : _oneWire(pin), _sensors(&_oneWire), _useAddress(true)
{
  memcpy(_address, address, sizeof(DeviceAddress));
}

// ─────────────────────────────────────────────
//  Inizializzazione
// ─────────────────────────────────────────────
bool DS18B20Sensor::begin()
{
  _sensors.begin();
  _sensors.setWaitForConversion(false);

  uint8_t found = _sensors.getDeviceCount();
  if (found == 0)
  {
    _lastError = TempError::NOT_CONNECTED;
    return false;
  }

  if (_useAddress && !_sensors.isConnected(_address))
  {
    _lastError = TempError::NOT_CONNECTED;
    return false;
  }

  _lastError = TempError::NONE;
  return true;
}

// ─────────────────────────────────────────────
//  Lettura pubblica
// ─────────────────────────────────────────────
TempReading DS18B20Sensor::read()
{
  TempReading result = _readWithRetry();
  _lastError = result.error;
  return result;
}

float DS18B20Sensor::readRaw()
{
  TempReading r = read();
  return r.isValid() ? r.value : DEVICE_DISCONNECTED_C;
}

// ─────────────────────────────────────────────
//  Lettura interna con retry
// ─────────────────────────────────────────────
TempReading DS18B20Sensor::_readWithRetry()
{
    TempReading result = {0.0f, TempError::READ_FAILED};

    for (uint8_t attempt = 0; attempt < _retries; attempt++)
    {
        // 1 — richiedi conversione
        _sensors.requestTemperatures();

        // 2 — aspetta senza bloccare
        uint32_t convStart = millis();
        while (!_sensors.isConversionComplete())
        {
            if (millis() - convStart > 1000) {
                result.error = TempError::READ_FAILED;
               delay(50);
                continue;  // prossimo tentativo
            }
            yield();
        }

        // 3 — leggi il valore convertito
        float raw = _useAddress
            ? _sensors.getTempC(_address)
            : _sensors.getTempCByIndex(0);

        // 4 — valida
        if (raw == DEVICE_DISCONNECTED_C || raw == 85.0f) {
            result.error = TempError::NOT_CONNECTED;
            delay(50);
            continue;
        }

        TempError validErr = _validate(raw);
        if (validErr != TempError::NONE) {
            result.error = validErr;
            delay(50);
            continue;
        }

        // 5 — tutto ok
        result.value = raw;
        result.error = TempError::NONE;
        return result;
    }

    
    return result;
}
// ─────────────────────────────────────────────
//  Validazione
// ─────────────────────────────────────────────
TempError DS18B20Sensor::_validate(float value)
{
  if (value < _minTemp || value > _maxTemp)
    return TempError::OUT_OF_RANGE;
  return TempError::NONE;
}

// ─────────────────────────────────────────────
//  Stato
// ─────────────────────────────────────────────
bool DS18B20Sensor::isConnected()
{
  if (_useAddress)
  {
    return _sensors.isConnected(_address);
  }
  return _sensors.getDeviceCount() > 0;
}

TempError DS18B20Sensor::getLastError() const { return _lastError; }
const char *DS18B20Sensor::getLastErrorMsg() const { return errorToString(_lastError); }

// ─────────────────────────────────────────────
//  Configurazione
// ─────────────────────────────────────────────
void DS18B20Sensor::setResolution(uint8_t bits)
{
  bits = constrain(bits, 9, 12);
  if (_useAddress)
  {
    _sensors.setResolution(_address, bits);
  }
  else
  {
    _sensors.setResolution(bits);
  }
}

void DS18B20Sensor::setValidRange(float minTemp, float maxTemp)
{
  _minTemp = minTemp;
  _maxTemp = maxTemp;
}

void DS18B20Sensor::setRetryCount(uint8_t retries)
{
  _retries = max((uint8_t)1, retries);
}

// ─────────────────────────────────────────────
//  Diagnostica
// ─────────────────────────────────────────────
void DS18B20Sensor::printInfo()
{
  if (_useAddress)
  {
    Serial.print("  Indirizzo       : ");
    for (uint8_t i = 0; i < 8; i++)
    {
      Serial.printf("0x%02X", _address[i]);
      if (i < 7)
        Serial.print(", ");
    }
    Serial.println();
  }
  Serial.printf("  Range valido    : %.1f / %.1f C\n", _minTemp, _maxTemp);
  Serial.printf("  Max tentativi   : %d\n", _retries);
  Serial.printf("  Ultimo errore   : %s\n", getLastErrorMsg());
  Serial.println("--------------------------------");
}

const char *DS18B20Sensor::errorToString(TempError err)
{
  switch (err)
  {
  case TempError::NONE:
    return "Nessun errore";
  case TempError::NOT_CONNECTED:
    return "Sensore non connesso";
  case TempError::CRC_FAILED:
    return "Errore CRC";
  case TempError::OUT_OF_RANGE:
    return "Valore fuori range";
  case TempError::READ_FAILED:
    return "Lettura fallita";
  default:
    return "Errore sconosciuto";
  }
}