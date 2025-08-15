#include <Arduino.h>
#include "modos.h"
#include "actuadores.h"   // PressMode + seleccionarModoPresscontrol()
#include "variables.h"
#include "constantes.h"
#include "ajustes.h"
#include "modos_asistente.h"


// ==============================
// Estado de modos
ModoOperacion modoActual = MODO_NINGUNO;

// Histeresis/temporización para M3
static uint32_t lastCambioBombaMs = 0;

// (Opcional) Temporizador de fallo simple para M2
static uint32_t tInicioDemandaMs = 0;
static bool     enDemanda = false;

// ==============================
// Utilidades locales
static void _todoSeguro() {
  activarBomba(false);
  activarValvula(1, false);
  activarValvula(2, false);
  seleccionarModoPresscontrol(PM_M1); // si prefieres, apaga todos cambiando aquí
}

static bool _esModoExtraccion(ModoOperacion m) {
  switch (m) {
    case EXTRACCION_E1A: case EXTRACCION_E1B: case EXTRACCION_E1C:
    case EXTRACCION_E2A: case EXTRACCION_E2B: case EXTRACCION_E2C:
    case EXTRACCION_E3:
      return true;
    default: return false;
  }
}

static PressMode _pressModeDe(ModoOperacion m) {
  // A → M1, B → M2, C → M3
  switch (m) {
    case EXTRACCION_E1A: case EXTRACCION_E2A: return PM_M1;
    case EXTRACCION_E1B: case EXTRACCION_E2B: return PM_M2;
    case EXTRACCION_E1C: case EXTRACCION_E2C: return PM_M3;
    default: return PM_M1;
  }
}

// ==============================
// Nombres legibles para UI
const char* obtenerNombreModo(ModoOperacion modo) {
  switch (modo) {
    case LLENADO_L1:     return "L1: Llenado A.G (Control)";
    case LLENADO_L2:     return "L2: Llenado A.G (Bypass)";
    case LLENADO_L3:     return "L3: Llenado A.P (Control)";
    case LLENADO_L4:     return "L4: Llenado A.P (Bypass)";
    case EXTRACCION_E1A: return "E1A: Extraccion Principal [M1]";
    case EXTRACCION_E1B: return "E1B: Extraccion Principal [M2]";
    case EXTRACCION_E1C: return "E1C: Extraccion Principal [M3]";
    case EXTRACCION_E2A: return "E2A: Extraccion Auxiliar [M1]";
    case EXTRACCION_E2B: return "E2B: Extraccion Auxiliar [M2]";
    case EXTRACCION_E2C: return "E2C: Extraccion Auxiliar [M3]";
    case EXTRACCION_E3:  return "E3: Extraccion Mantenimiento";
    case CONSUMO_C1:     return "C1: Consumo directo A.P";
    case CONSUMO_C2:     return "C2: Consumo directo A.G";
    case MODO_NINGUNO:
    default:             return "Sin modo activo";
  }
}

// ==============================
// Activación de modos (válvulas + modo de presscontrol)
void activarModo(ModoOperacion modo) {
  if (modo == modoActual) return; // ya activo
  _todoSeguro();

  switch (modo) {
    // ====== Llenados ======
    case LLENADO_L1:
      activarValvula(1, true);
      activarValvula(2, false);
      seleccionarModoPresscontrol(PM_M1);
      activarBomba(false);
      break;

    case LLENADO_L2:
      activarValvula(1, true);
      activarValvula(2, true);
      seleccionarModoPresscontrol(PM_M1);
      activarBomba(false);
      break;

    case LLENADO_L3:
      activarValvula(1, false);
      activarValvula(2, true);
      seleccionarModoPresscontrol(PM_M1);
      activarBomba(false);
      break;

    case LLENADO_L4:
      activarValvula(1, false);
      activarValvula(2, true);
      seleccionarModoPresscontrol(PM_M1);
      activarBomba(false);
      break;

    // ====== Extracciones ======
    case EXTRACCION_E1A: case EXTRACCION_E1B: case EXTRACCION_E1C:
      activarValvula(1, true);
      activarValvula(2, false);
      seleccionarModoPresscontrol(_pressModeDe(modo));
      break;

    case EXTRACCION_E2A: case EXTRACCION_E2B: case EXTRACCION_E2C:
      activarValvula(1, false);
      activarValvula(2, true);
      seleccionarModoPresscontrol(_pressModeDe(modo));
      break;

    case EXTRACCION_E3:
      activarValvula(1, true);
      activarValvula(2, true);
      seleccionarModoPresscontrol(PM_M1);
      activarBomba(false);
      break;

    // ====== Consumos directos ======
    case CONSUMO_C1:
      activarValvula(1, false);
      activarValvula(2, true);
      seleccionarModoPresscontrol(PM_M1);
      activarBomba(false);
      break;

    case CONSUMO_C2:
      activarValvula(1, true);
      activarValvula(2, false);
      seleccionarModoPresscontrol(PM_M1);
      activarBomba(false);
      break;

    case MODO_NINGUNO:
    default:
      _todoSeguro();
      break;
  }

  lastCambioBombaMs = millis();
  enDemanda = false;
  tInicioDemandaMs = 0;

  modoActual = modo;
  Serial.printf("[MODOS] Activado: %s\n", obtenerNombreModo(modo));
}

void desactivarModo() {
  _todoSeguro();
  modoActual = MODO_NINGUNO;
  Serial.println("[MODOS] Desactivado: Sin modo activo");
}

// ==============================
// Bucle de control de modos (llamar en loop())
void modosLoop() {
  if (!_esModoExtraccion(modoActual)) return;

  PressMode pm = _pressModeDe(modoActual);

  // Seguridad manda
  if (alertaSeguridad) {
    activarBomba(false);
    return;
  }

  // --- PM_M1: presscontrol clásico ---
  if (pm == PM_M1) {
    return;
  }

  // --- PM_M2: alimentado permanente, corte por fallo ---
  if (pm == PM_M2) {
    uint32_t now = millis();

    if (presion3 < PRESION_ON_BAR) {
      if (!enDemanda) {
        enDemanda = true;
        tInicioDemandaMs = now;
      } else if ((now - tInicioDemandaMs) > TIEMPO_FALLO_M2_MS) {
        bool sinConsumo = (consumoBomba > 0.1f && consumoBomba < CONSUMO_BOMBA_MIN_W);
        if (presion3 < PRESION_ON_BAR || sinConsumo) {
          Serial.println("[M2] FALLO detectado. Cortando alimentacion presscontrol.");
          seleccionarModoPresscontrol(PM_M1); // si quieres, apaga todos aquí
          activarBomba(false);
        }
      }
    } else {
      enDemanda = false;
      tInicioDemandaMs = 0;
    }
    return;
  }

  // --- PM_M3: ESP32 gobierna por histeresis ---
  if (pm == PM_M3) {
    uint32_t now = millis();
    bool puedeCambiar = (now - lastCambioBombaMs) > TIEMPO_MIN_CAMBIO_MS;

    if (!bombaEncendida && presion3 < gAjustes.presionOnBar && puedeCambiar) {
      activarBomba(true);
      seleccionarModoPresscontrol(PM_M3);
      lastCambioBombaMs = now;
      Serial.println("[M3] Bomba ON por baja presion");
    } else if (bombaEncendida && presion3 > gAjustes.presionOffBar && puedeCambiar) {
      activarBomba(false);
      lastCambioBombaMs = now;
      Serial.println("[M3] Bomba OFF por presion suficiente");
    }
    return;
  }
}
