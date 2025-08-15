#pragma once
#include <Arduino.h>
#include "constantes.h"   // Pines
#include "variables.h"    // Estados globales: electrovalvula1/2, bombaEncendida, zumbadorActivo

// Si tu banco de relés se activa a nivel BAJO, deja esto en 1.
// Si cambias jumpers a HIGH-trigger, ponlo en 0.
#ifndef RELAY_ACTIVE_LOW
#define RELAY_ACTIVE_LOW 1
#endif

// Modo del presscontrol (selección de los tres “enchufes” M1/M2/M3)
enum PressMode {
  PM_M1 = 0,
  PM_M2 = 1,
  PM_M3 = 2
};

// Inicialización de pines/estados seguros
void initActuadores();

// Actuadores básicos
void activarValvula(int id, bool on);  // id=1 -> PIN_VALVULA_1, id=2 -> PIN_VALVULA_2
void activarBomba(bool on);
void activarZumbador(bool on);

// Selección de modo de presscontrol (mutuamente exclusivo)
void setPressMode(PressMode pm);

// Alias por compatibilidad con código previo
inline void seleccionarModoPresscontrol(PressMode pm) { setPressMode(pm); }

// (Opcional) Electroválvula de seguridad si defines PIN_EV_SEGURIDAD en constantes.h
void setEVSeguridad(bool cerrada);
