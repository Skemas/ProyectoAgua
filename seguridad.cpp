#include <Arduino.h>
#include "seguridad.h"
#include "constantes.h"
#include "variables.h"
#include "actuadores.h"
#include "opciones.h"

// Reglas de seguridad:
// - Sobrellenado: cerrar EV de seguridad (si EVSegMode=Auto u On).
// - Zumbador: sólo si habilitado y no silenciado.
// - Restos: apagar zumbador si no hay alerta.

void initSeguridad() {
  // Si necesitas inicializar algo, aquí.
}

void verificarSeguridad() {
  bool haySobrellenado = (nivelAguaPct >= UMBRAL_SOBRELLENADO);

  // Gestion EV Seguridad
  EVSegMode mode = opcionesGetEVSegMode();

  if (mode == EVSegMode::ON) {
    // Forzar cierre siempre
    setEVSeguridad(true);
  } else if (mode == EVSegMode::OFF) {
    // Forzar abierta siempre
    setEVSeguridad(false);
  } else { // AUTO
    // Sólo cerrar si hay sobrellenado, abrir si no
    setEVSeguridad(haySobrellenado);
  }

  // Gestion Zumbador
  bool enableBuzzer = opcionesGetZumbador();
  bool muted        = opcionesGetZumbadorMuted();

  if (haySobrellenado) {
    alertaSeguridad = true;
    if (enableBuzzer && !muted) activarZumbador(true);
  } else {
    alertaSeguridad = false;
    activarZumbador(false);
  }
}
