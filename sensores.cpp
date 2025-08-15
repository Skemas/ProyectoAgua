#include <Arduino.h>
#include "sensores.h"
#include "constantes.h"  // TRIG_PIN, ECHO_PIN, pines de sensores
#include "variables.h"
#include "ajustes.h"


// --- Flujo YF-B10 ---
static volatile uint32_t pulsosFlujoYF1 = 0;
static volatile uint32_t pulsosFlujoYF2 = 0;


// ISRs
void IRAM_ATTR contarPulsosYF1() { pulsosFlujoYF1++; }
void IRAM_ATTR contarPulsosYF2() { pulsosFlujoYF2++; }

void initSensores() {
  // Flujo
  pinMode(PIN_SENSOR_FLUJO_1, INPUT_PULLUP);
  pinMode(PIN_SENSOR_FLUJO_2, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(PIN_SENSOR_FLUJO_1), contarPulsosYF1, FALLING);
  attachInterrupt(digitalPinToInterrupt(PIN_SENSOR_FLUJO_2), contarPulsosYF2, FALLING);

  // Analógicos
  pinMode(PIN_SENSOR_NIVEL,     INPUT);
  pinMode(PIN_SENSOR_PRESION_1, INPUT);
  pinMode(PIN_SENSOR_PRESION_2, INPUT);
  pinMode(PIN_SENSOR_PRESION_3, INPUT);

  // Ultrasónico
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
}

// --- helpers de conversión ---
// (Sustituye por la curva real de tus sensores de presión)
static float adcAPresionBar(int adc) {
  return (adc / 4095.0f) * 10.0f; // ejemplo 0..10 bar
}

static float adcANivelPct(int adc) {
  float pct = (adc / 4095.0f) * 100.0f;
  if (pct < 0)   pct = 0;
  if (pct > 100) pct = 100;
  return pct;
}

static float nivelPctAVolumenL(float pct) {
  float vmax = getVolumenMaxL();
  return (pct / 100.0f) * vmax;
}

static float calcFlujoLmin(uint32_t& pulsos, uint32_t dt_ms) {
  const float ppl = getPulsosPorLitro();
  float litros = pulsos / (ppl > 1e-3f ? ppl : 450.0f);
  float lmin   = (dt_ms > 0) ? (litros * (60000.0f / dt_ms)) : 0.0f;
  pulsos = 0;
  return lmin;
}

// EMA (Exponential Moving Average)
static inline float ema(float prev, float val, float alpha) {
  return prev * (1.0f - alpha) + val * alpha;
}


void leerSensores() {
  // ADCs
  int adcNivel = analogRead(PIN_SENSOR_NIVEL);
  int adcP1    = analogRead(PIN_SENSOR_PRESION_1);
  int adcP2    = analogRead(PIN_SENSOR_PRESION_2);
  int adcP3    = analogRead(PIN_SENSOR_PRESION_3);

  // Convertidos (temporales)
  float p1 = adcAPresionBar(adcP1);
  float p2 = adcAPresionBar(adcP2);
  float p3 = adcAPresionBar(adcP3);
  float nv = adcANivelPct(adcNivel);

  
  // Suavizado (primera muestra sin EMA)
  static bool first = true;
  if (first) {
    presion1 = p1;
    presion2 = p2;
    presion3 = p3;
    nivelAguaPct = nv;
    first = false;
  } else {
    presion1     = ema(presion1,    p1, gAjustes.emaPresionAlpha);
    presion2     = ema(presion2,    p2, gAjustes.emaPresionAlpha);
    presion3     = ema(presion3,    p3, gAjustes.emaPresionAlpha);
    nivelAguaPct = ema(nivelAguaPct, nv, gAjustes.emaNivelAlpha);
  }

  // Volumen estimado (usa ajustes)
  volumenAgua = (nivelAguaPct / 100.0f) * getVolumenMaxL();

  // Ultrasónico
  distancia = medirDistancia();

  // Flujo (cada 500 ms)
  static uint32_t lastMs = 0;
  uint32_t now = millis();
  uint32_t dt  = now - lastMs;
  if (dt >= 500) {
    noInterrupts();
    uint32_t p1 = pulsosFlujoYF1;
    uint32_t p2 = pulsosFlujoYF2;
    pulsosFlujoYF1 = 0;
    pulsosFlujoYF2 = 0;
    interrupts();

    flujoYF1 = calcFlujoLmin(p1, dt);
    flujoYF2 = calcFlujoLmin(p2, dt);
    lastMs = now;
  }
}

float medirDistancia() {
  // pulso TRIG
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  // eco con timeout
  unsigned long duracion = pulseIn(ECHO_PIN, HIGH, 30000UL); // ~5 m
  if (duracion == 0) return -1.0f;

  // 0.0343 cm/us (ida y vuelta → /2)
  float distancia_cm = (duracion * 0.0343f) / 2.0f;
  return distancia_cm;
}
