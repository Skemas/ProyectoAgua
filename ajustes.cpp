#include "ajustes.h"
#include <Arduino.h>

Ajustes gAjustes;
static Preferences prefs;

void initAjustes() {
  if (!prefs.begin("agua-ajustes", false)) {
    Serial.println("[AJUSTES] Error abriendo NVS");
    return;
  }

  // Cargar con defaults si no existen
  gAjustes.pulsosPorLitroYF  = prefs.getFloat("ppl",  450.0f);
  gAjustes.volumenMaxL       = prefs.getFloat("vmax", 1000.0f);

  gAjustes.presionOnBar      = prefs.getFloat("p_on", 2.0f);
  gAjustes.presionOffBar     = prefs.getFloat("p_off",3.0f);

  gAjustes.umbralSobrellenado= prefs.getFloat("sobr", 90.0f);

  gAjustes.emaPresionAlpha   = prefs.getFloat("emaP", 0.25f);
  gAjustes.emaNivelAlpha     = prefs.getFloat("emaN", 0.25f);

  gAjustes.brillo            = prefs.getUChar("bril", 200);
}

void guardarAjustes() {
  prefs.putFloat("ppl",  gAjustes.pulsosPorLitroYF);
  prefs.putFloat("vmax", gAjustes.volumenMaxL);
  prefs.putFloat("p_on", gAjustes.presionOnBar);
  prefs.putFloat("p_off",gAjustes.presionOffBar);
  prefs.putFloat("sobr", gAjustes.umbralSobrellenado);
  prefs.putFloat("emaP", gAjustes.emaPresionAlpha);
  prefs.putFloat("emaN", gAjustes.emaNivelAlpha);
  prefs.putUChar("bril", gAjustes.brillo);
  Serial.println("[AJUSTES] Guardados en NVS");
}

// Helpers para Sensores
float getPulsosPorLitro() { return gAjustes.pulsosPorLitroYF; }
void  setPulsosPorLitro(float v) { gAjustes.pulsosPorLitroYF = v; }

float getVolumenMaxL() { return gAjustes.volumenMaxL; }
void  setVolumenMaxL(float v) { gAjustes.volumenMaxL = v; }
                              