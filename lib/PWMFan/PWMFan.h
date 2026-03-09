// ============================================================
//  PWMFan.h  -  lib/PWMFan/PWMFan.h
//  Responsabilità unica: controllo ventola PWM 4-pin.
//  PWM 25kHz via LEDC hardware ESP32.
//  RPM via interrupt su pin TACH (2 impulsi/giro, standard).
// ============================================================

#pragma once
#include <Arduino.h>

// ─────────────────────────────────────────────
//  Risultato lettura
// ─────────────────────────────────────────────
struct FanReading {
  uint8_t  speedPercent;  // duty cycle comandato (0-100%)
  uint16_t rpm;           // RPM misurati via TACH
  bool     stalled;       // true se PWM > 0 ma RPM = 0
};

// ─────────────────────────────────────────────
//  Classe PWMFan
// ─────────────────────────────────────────────
class PWMFan {
  public:
    // pinPWM  : GPIO uscita PWM verso ventola
    // pinTach : GPIO ingresso segnale TACH dalla ventola
    PWMFan(uint8_t pinPWM, uint8_t pinTach);

    // Inizializza LEDC e interrupt TACH
    bool begin();

    // Imposta velocità: 0-100%
    // 0 = stop (duty cycle 0, ventola si ferma se lo permette)
    void setSpeed(uint8_t percent);

    // Lettura stato corrente
    uint8_t    getSpeed()  const;   // duty cycle attuale (0-100%)
    uint16_t   getRPM()    const;   // RPM calcolati
    bool       isStalled() const;   // ventola bloccata

    // Snapshot completo
    FanReading read() const;

    // Da chiamare nel loop() — aggiorna RPM ogni secondo
    void update();

  private:
    uint8_t  _pinPWM;
    uint8_t  _pinTach;
    uint8_t  _speedPercent = 0;
    uint16_t _rpm          = 0;

    // TACH — contatore impulsi via interrupt
    volatile uint32_t _pulseCount = 0;
    volatile uint32_t _lastPulseUs = 0;
    uint32_t          _lastCalcMs = 0;
    static const uint32_t CALC_INTERVAL_MS = 1000;  // calcola RPM ogni 1s
    static const uint8_t  PULSES_PER_REV   = 2;     // standard ventole PC

    // Stall detection
    static const uint16_t STALL_RPM_THRESHOLD = 50;  // sotto 50 RPM = stalled

    // Interrupt handler (deve essere static)
    static void IRAM_ATTR _tachISR(void* arg);
};