#pragma once
#include <stdint.h>

// Modo de la Electrovalvula de seguridad
enum class EVSegMode : uint8_t { AUTO = 0, ON = 1, OFF = 2 };

// Inicializa estado por defecto
void opcionesInit();

// Zumbador
void opcionesSetZumbador(bool on);
bool opcionesGetZumbador();

void opcionesSetZumbadorMuted(bool muted);
bool opcionesGetZumbadorMuted();

// EV de seguridad
void opcionesSetEVSegMode(EVSegMode m);
EVSegMode opcionesGetEVSegMode();
