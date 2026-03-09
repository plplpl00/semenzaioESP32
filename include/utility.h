// ============================================================
//  utility.h
//  Posizione: include/utility.h
//
//  Pin, indirizzi sensori e macro di logging.
//  Incluso da tutti i moduli del progetto.
// ============================================================
#pragma once

#include <DS18B20Sensor.h>  // usa la libreria custom, non DallasTemperature

// ─────────────────────────────────────────────────────────────
//  Pin
// ─────────────────────────────────────────────────────────────
#define PIN_ONE_WIRE          4
#define PIN_FAN_PWM           18
#define PIN_FAN_TACH          19
#define PIN_I2C_SDA           21
#define PIN_I2C_SCL           22
#define PIN_VENTILATION_RELAY 16
#define PIN_LIGHT_RELAY       17

// ─────────────────────────────────────────────────────────────
//  Indirizzi sensori
//  Trovati con DS18B20Scanner e identificati fisicamente
// ─────────────────────────────────────────────────────────────
#define ADDR_SHT31 0x44

inline DeviceAddress SONDA_INTERNA = {
    0x28, 0x48, 0x5b, 0xbc,
    0x00, 0x00, 0x00, 0xac
};

// ─────────────────────────────────────────────────────────────
//  Macro di logging
//
//  DEBUG_MODE attivo   → Debug.logf() → visibile sul browser
//  DEBUG_MODE inattivo → Serial.printf() → visibile su seriale
//
//  Uso:
//    LOG_INFO("VentCtrl", "Speed=%d%%", speed);
//    LOG_WARNING("ENV", "Temp alta: %.1f°C", temp);
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