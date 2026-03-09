// ============================================================
//  PWMFan.cpp  -  lib/PWMFan/PWMFan.cpp
// ============================================================

#include "PWMFan.h"

// ─────────────────────────────────────────────
//  Parametri LEDC
// ─────────────────────────────────────────────
static const uint32_t PWM_FREQUENCY  = 25000;  // 25 kHz - standard Intel 4-pin
static const uint8_t  PWM_RESOLUTION = 8;      // 8 bit = 0-255

// ─────────────────────────────────────────────
//  Costruttore
// ─────────────────────────────────────────────
PWMFan::PWMFan(uint8_t pinPWM, uint8_t pinTach)
  : _pinPWM(pinPWM), _pinTach(pinTach) {}

// ─────────────────────────────────────────────
//  Inizializzazione
// ─────────────────────────────────────────────
bool PWMFan::begin() {
  // Configura LEDC per PWM 25kHz
  ledcAttach(_pinPWM, PWM_FREQUENCY, PWM_RESOLUTION);
  ledcWrite(_pinPWM, 0);  // parte ferma

  // Configura pin TACH come input con pull-up
  // Il segnale TACH è open-drain: pull-up necessario
  pinMode(_pinTach, INPUT_PULLUP);

  // Attach interrupt — passa this come argomento per accedere a _pulseCount
  attachInterruptArg(
    digitalPinToInterrupt(_pinTach),
    _tachISR,
    this,
    FALLING  // impulso TACH: fronte di discesa
  );

  _lastCalcMs = millis();

  return true;
}

// ─────────────────────────────────────────────
//  Imposta velocità
// ─────────────────────────────────────────────
void PWMFan::setSpeed(uint8_t percent) {
  _speedPercent = constrain(percent, 0, 100);

  // Converte 0-100% in 0-255 (8 bit)
  uint32_t duty = map(_speedPercent, 0, 100, 0, 255);
  ledcWrite(_pinPWM, duty);
}

// ─────────────────────────────────────────────
//  Lettura stato
// ─────────────────────────────────────────────
uint8_t  PWMFan::getSpeed()  const { return _speedPercent; }
uint16_t PWMFan::getRPM()    const { return _rpm; }

bool PWMFan::isStalled() const {
  return (_speedPercent > 0) && (_rpm < STALL_RPM_THRESHOLD);
}

FanReading PWMFan::read() const {
  return {
    .speedPercent = _speedPercent,
    .rpm          = _rpm,
    .stalled      = isStalled()
  };
}

// ─────────────────────────────────────────────
//  Update — calcola RPM ogni CALC_INTERVAL_MS
//  Da chiamare nel loop()
// ─────────────────────────────────────────────
void PWMFan::update() {
  uint32_t now = millis();
  if (now - _lastCalcMs < CALC_INTERVAL_MS) return;

  // Legge e azzera il contatore in modo atomico
  // noInterrupts/interrupts per evitare race condition
  noInterrupts();
  uint32_t pulses = _pulseCount;
  _pulseCount = 0;
  interrupts();

  uint32_t elapsed = now - _lastCalcMs;
  _lastCalcMs = now;

  // RPM = (impulsi / impulsi_per_giro) * (60000ms / elapsed_ms)
  _rpm = (uint16_t)((pulses * 60000UL) / (PULSES_PER_REV * elapsed));

}

// ─────────────────────────────────────────────
//  Interrupt handler TACH
//  IRAM_ATTR: eseguito dalla RAM interna (più veloce e sicuro)
// ─────────────────────────────────────────────
void IRAM_ATTR PWMFan::_tachISR(void* arg) {
    PWMFan* self = static_cast<PWMFan*>(arg);
    uint32_t now = micros();

    // A 3200 RPM con 2 impulsi/giro → un impulso ogni ~9375µs
    // Soglia a 4000µs: filtra il rumore, passa tutti gli impulsi reali
    if (now - self->_lastPulseUs > 9000) {
        self->_pulseCount++;
        self->_lastPulseUs = now;
    }
}