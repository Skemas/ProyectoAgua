#pragma once
#include <TFT_eSPI.h>
#include "modos.h"        // ModoOperacion
#include "constantes.h"   // pines y constantes
#include "actuadores.h"   // enum PressMode (PM_M1/PM_M2/PM_M3)

// --- Enumeración de las fuentes de extracción ---
enum class ExtraccionFuente : uint8_t {
  DP = 0,       // Depósito Principal
  DA = 1,       // Depósito Auxiliar
  DA_MANT = 2   // Auxiliar vía mantenimiento
};

// Estado actual de selección (índice -1..2)
extern int8_t gExtrActive;  // -1 = ninguna

// Inicialización del asistente (se llama al entrar en MODOS o en setupPantalla)
void modosAsistenteInit();

// Helpers de lógica (usados desde .cpp o por quien lo necesite)
void setPressModeFor(ExtraccionFuente src, PressMode pm);
void aplicarLogicaExtraccion(ExtraccionFuente src, PressMode pm);

// UI del asistente
void modosAsistenteDraw(TFT_eSPI &tft, ModoOperacion modoActual);
bool modosAsistenteTouch(TFT_eSPI &tft, uint16_t tx, uint16_t ty, ModoOperacion modoActual);

// ===== Getters para mostrar “modos activos” en Menú Principal =====
int8_t   getLlenadoSel();                                  // -1 nada, 0..3 L1..L4
int8_t   getExtrActive();                                   // -1 nada, 0..2 (DP,DA,DA_MANT)
PressMode getExtrPressFor(ExtraccionFuente src);            // PM_M1/PM_M2/PM_M3
int8_t   getConsumoSel();                                   // -1 nada, 0..1 (C1,C2)
