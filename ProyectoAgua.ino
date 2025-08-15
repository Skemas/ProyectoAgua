
// ProyectoAgua.ino
// Punto de entrada. Mantener este archivo limpio facilita el mantenimiento.
// Orquesta la inicialización y el bucle principal delegando en módulos.

#include <Arduino.h>
#include <TFT_eSPI.h>

#include "constantes.h"
#include "config.h"
#include "variables.h"

#include "sensores.h"
#include "actuadores.h"
#include "seguridad.h"
#include "comunicacion.h"
#include "pantalla.h"
#include "modos.h"
#include "ajustes.h"

// Instancia única de la pantalla
TFT_eSPI tft;

// --- Ciclo de inicialización ---
void setup() {
  Serial.begin(115200);
  delay(100);

  initAjustes();
  initConfig();        // Pines y estados seguros
  initActuadores();    // Estados internos de actuadores
  initSensores();      // ISRs y entradas de sensores
  initSeguridad();     // Banderas/umbrales, si aplica
  initComunicacion();  // WiFi/MQTT (si procede)

  setupPantalla(tft);  // UI inicial (menú principal)
  Serial.println(F("Sistema de llenado: inicialización completa."));
}

// --- Bucle principal ---
// Mantenerlo en “alto nivel”: leer sensores, actualizar lógica y UI.
void loop() {
  leerSensores();          // Actualiza medidas globales
  verificarSeguridad();    // Aplica reglas de seguridad (zumbador, cierres)
  modosLoop();             // <-- Nuevo: gobierna bomba según M1/M2/M3
  gestionarComunicacion(); // Publica/recibe (si lo tienes implementado)
  loopPantalla(tft);       // Gestiona interacción táctil y redibuja
}
