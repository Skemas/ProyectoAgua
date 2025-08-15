#ifndef CONSTANTES_H
#define CONSTANTES_H

// ============================================================
//  CONSTANTES DEL PROYECTO DE CONTROL DE LLENADO DE AGUA
//  ESP32 + Sensores + Actuadores + Modos de Presscontrol
// ============================================================

// Si los jumpers están en 2–3 (LOW trigger) deja true.
// Si los jumpers están en 1–2 (HIGH trigger) pon false.
#define RELAY_ACTIVE_LOW  true


// ------------------------------------------------------------
// UMBRALES GENERALES
// ------------------------------------------------------------
#define UMBRAL_SOBRELLENADO 90.0    // % de nivel para activar alarma de sobrellenado

// ------------------------------------------------------------
// PINES DE SENSORES (AJUSTAR A TU CABLEADO REAL)
// ------------------------------------------------------------
#define PIN_SENSOR_NIVEL        34  // Sensor de nivel (analógico)
#define PIN_SENSOR_PRESION_1    35  // Sensor de presión #1
#define PIN_SENSOR_PRESION_2    32  // Sensor de presión #2
#define PIN_SENSOR_PRESION_3    33  // Sensor de presión #3
#define PIN_SENSOR_FLUJO_1      25  // YF-B10 #1 (Aportación)
#define PIN_SENSOR_FLUJO_2      26  // YF-B10 #2 (Consumo)

// Pines del sensor ultrasónico JSN-SR04T
#define TRIG_PIN                12  // TRIG del ultrasónico
#define ECHO_PIN                36  // ECHO del ultrasónico

// ------------------------------------------------------------
// PINES DE ACTUADORES
// ------------------------------------------------------------
#define PIN_VALVULA_1            5  // Electroválvula 1
#define PIN_VALVULA_2            4  // Electroválvula 2
#define PIN_BOMBA               23  // Relé de la bomba
#define PIN_ZUMBADOR            27  // Alarma sonora

// ------------------------------------------------------------
// MODOS DE PRESSCONTROL - PINES DE LOS ENCHUFES/RELÉS
// ------------------------------------------------------------
// Cada pin controla un modo de alimentación del presscontrol.
// SOLO un modo puede estar activo a la vez.
#define PIN_MODO_M1             15  // Modo 1: Presscontrol clásico (alimentación directa)
#define PIN_MODO_M2              2  // Modo 2: Presscontrol A.R.P. (alimentación regulada permanente)
#define PIN_MODO_M3             18  // Modo 3: Presscontrol A.R.Esp (alimentación controlada por ESP32)

//CONTROL DE BRILLO PANTALLA-LED
//#define PIN_TFT_BL 21   // GPIO para backlight (cámbialo si prefieres otro libre)

// ------------------------------------------------------------
// CONFIGURACIÓN ESPECÍFICA PARA MODO 3 (A.R.Esp)
// ------------------------------------------------------------
// Histeresis de presión para encender/apagar bomba
#define PRESION_ON_BAR        2.0f   // Encender bomba si presión < PRESION_ON_BAR
#define PRESION_OFF_BAR       3.0f   // Apagar bomba si presión > PRESION_OFF_BAR

// Tiempo mínimo entre cambios ON/OFF de la bomba (anti-flapping)
#define TIEMPO_MIN_CAMBIO_MS  3000

// ------------------------------------------------------------
// DETECCIÓN DE FALLO EN MODO 2
// ------------------------------------------------------------
// Si la bomba debería estar activa pero no sube presión,
// se considera fallo tras este tiempo.
#define CONSUMO_BOMBA_MIN_W   50.0f  // Potencia mínima para considerar que la bomba está en marcha
#define TIEMPO_FALLO_M2_MS    20000  // Tiempo máximo para que suba la presión (ms)

#endif
