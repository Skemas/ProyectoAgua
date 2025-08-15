#ifndef AJUSTES_H
#define AJUSTES_H

#include <Preferences.h>

// Estructura en RAM con los ajustes editables
struct Ajustes {
  // Sensores
  float pulsosPorLitroYF = 450.0f; // YF-B10 ~450
  float volumenMaxL      = 1000.0f;

  // Presión (curva 0.5–4.5V → 0–RANGO la dejamos en constantes; aquí umbrales control)
  float presionOnBar     = 2.0f;   // M3 ON
  float presionOffBar    = 3.0f;   // M3 OFF

  // Nivel / seguridad
  float umbralSobrellenado = 90.0f;

  // Suavizados
  float emaPresionAlpha   = 0.25f;
  float emaNivelAlpha     = 0.25f;

  // UI
  uint8_t brillo          = 200;   // 0-255 si tu TFT lo soporta (PWM externo)
};

extern Ajustes gAjustes;

// Inicializa NVS y carga ajustes (o por defecto si no existen)
void initAjustes();

// Guarda todos los ajustes actuales en NVS
void guardarAjustes();

// Helpers: get/set pulsos por litro en Sensores
float getPulsosPorLitro();
void  setPulsosPorLitro(float v);

// Helpers: exponer volumen máx
float getVolumenMaxL();
void  setVolumenMaxL(float v);

#endif // AJUSTES_H
