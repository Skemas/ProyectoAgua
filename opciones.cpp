#include "opciones.h"

static bool      s_zumbadorOn       = false;            // habilita sonido
static bool      s_zumbadorMuted    = false;            // silencia temporalmente
static EVSegMode s_evSegMode        = EVSegMode::AUTO;  // Auto por defecto

void opcionesInit() {
  s_zumbadorOn    = false;
  s_zumbadorMuted = false;
  s_evSegMode     = EVSegMode::AUTO;
}

// --- Zumbador ---
void opcionesSetZumbador(bool on) { s_zumbadorOn = on; }
bool opcionesGetZumbador()        { return s_zumbadorOn; }

void opcionesSetZumbadorMuted(bool muted) { s_zumbadorMuted = muted; }
bool opcionesGetZumbadorMuted()           { return s_zumbadorMuted; }

// --- EV Seguridad ---
void opcionesSetEVSegMode(EVSegMode m) { s_evSegMode = m; }
EVSegMode opcionesGetEVSegMode()       { return s_evSegMode; }
