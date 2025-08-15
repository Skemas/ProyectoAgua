// config.cpp
#include <Arduino.h>
#include "config.h"
#include "constantes.h"

// Inicializa dirección de pines y pone actuadores en estado seguro
void initConfig() {
  // --- Actuadores ---
  pinMode(PIN_VALVULA_1, OUTPUT);
  pinMode(PIN_VALVULA_2, OUTPUT);
  pinMode(PIN_BOMBA,     OUTPUT);
  pinMode(PIN_ZUMBADOR,  OUTPUT);

  // Estados seguros al arranque (todo apagado/cerrado)
  digitalWrite(PIN_VALVULA_1, LOW);
  digitalWrite(PIN_VALVULA_2, LOW);
  digitalWrite(PIN_BOMBA,     LOW);
  digitalWrite(PIN_ZUMBADOR,  LOW);

  // --- Sensores ---
  pinMode(PIN_SENSOR_NIVEL,     INPUT);
  pinMode(PIN_SENSOR_PRESION_1, INPUT);
  pinMode(PIN_SENSOR_PRESION_2, INPUT);
  pinMode(PIN_SENSOR_PRESION_3, INPUT);
  pinMode(PIN_SENSOR_FLUJO_1,   INPUT_PULLUP); // YF-B10 suele ir con PULLUP
  pinMode(PIN_SENSOR_FLUJO_2,   INPUT_PULLUP);
  // ----Presscontrol----
  pinMode(PIN_MODO_M1, OUTPUT);
  pinMode(PIN_MODO_M2, OUTPUT);
  pinMode(PIN_MODO_M3, OUTPUT);
  digitalWrite(PIN_MODO_M1, LOW);
  digitalWrite(PIN_MODO_M2, LOW);
  digitalWrite(PIN_MODO_M3, LOW);
}
