// ============================================================
//  utility.h
//  Posizione: include/utility.h
//
//  Pin, indirizzi sensori e macro di logging.
// ============================================================
#pragma once
#include <DS18B20Sensor.h>

// ─────────────────────────────────────────────────────────────
//  Pin
// ─────────────────────────────────────────────────────────────
#define PIN_ONE_WIRE          4   // Bus OneWire (entrambe le sonde DS18B20)
#define PIN_FAN_PWM           18
#define PIN_FAN_TACH          19
#define PIN_I2C_SDA           21
#define PIN_I2C_SCL           22
#define PIN_VENTILATION_RELAY 16
#define PIN_LIGHT_RELAY       17

// ─────────────────────────────────────────────────────────────
//  Indirizzi sensori
// ─────────────────────────────────────────────────────────────
#define ADDR_SHT31 0x44

// Sonda DS18B20 INTERNA (dentro il ripiano)
inline DeviceAddress SONDA_INTERNA = {
    0x28, 0x48, 0x5b, 0xbc,
    0x00, 0x00, 0x00, 0xac
};

// Sonda DS18B20 ESTERNA (temperatura ambiente)
// TODO: scansiona con DS18B20Scanner e inserisci l'indirizzo reale
inline DeviceAddress SONDA_ESTERNA = {
    0x28, 0xA0, 0x16, 0x6F, 
    0x00, 0x00, 0x00, 0xA8
};

// ─────────────────────────────────────────────────────────────
//  Timing
// ─────────────────────────────────────────────────────────────
#define ENV_READ_INTERVAL     2000   // ms tra letture sensori
#define VENT_UPDATE_INTERVAL  5000   // ms tra aggiornamenti ventilazione
#define FIREBASE_PUSH_INTERVAL 60000 // ms tra push telemetria

// ─────────────────────────────────────────────────────────────
//  Macro di logging
// ─────────────────────────────────────────────────────────────
#ifdef DEBUG_MODE
    #include <DebugLogger.h>
    #define LOG_DEBUG(tag, fmt, ...)   Debug.logf(LogLevel::DEBUG,   tag, fmt, ##__VA_ARGS__)
    #define LOG_INFO(tag, fmt, ...)    Debug.logf(LogLevel::INFO,    tag, fmt, ##__VA_ARGS__)
    #define LOG_WARNING(tag, fmt, ...) Debug.logf(LogLevel::WARNING, tag, fmt, ##__VA_ARGS__)
    #define LOG_ERROR(tag, fmt, ...)   Debug.logf(LogLevel::ERROR,   tag, fmt, ##__VA_ARGS__)
    #define LOG_SUCCESS(tag, fmt, ...) Debug.logf(LogLevel::SUCCESS, tag, fmt, ##__VA_ARGS__)
#else
    #define LOG_DEBUG(tag, fmt, ...)   Serial.printf("[DEBUG] [" tag "] " fmt "\n", ##__VA_ARGS__)
    #define LOG_INFO(tag, fmt, ...)    Serial.printf("[INFO] ["  tag "] " fmt "\n", ##__VA_ARGS__)
    #define LOG_WARNING(tag, fmt, ...) Serial.printf("[WARN] ["  tag "] " fmt "\n", ##__VA_ARGS__)
    #define LOG_ERROR(tag, fmt, ...)   Serial.printf("[ERROR] [" tag "] " fmt "\n", ##__VA_ARGS__)
    #define LOG_SUCCESS(tag, fmt, ...) Serial.printf("[OK] ["    tag "] " fmt "\n", ##__VA_ARGS__)
#endif
