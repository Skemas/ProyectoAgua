#include "actuadores.h"

// Helpers para relés LOW/HIGH trigger
static inline uint8_t releOn()  { return RELAY_ACTIVE_LOW ? LOW  : HIGH; }
static inline uint8_t releOff() { return RELAY_ACTIVE_LOW ? HIGH : LOW; }

void initActuadores() {
  // Pines de válvulas
  pinMode(PIN_VALVULA_1, OUTPUT);
  pinMode(PIN_VALVULA_2, OUTPUT);

  // Bomba y zumbador
  pinMode(PIN_BOMBA, OUTPUT);
  pinMode(PIN_ZUMBADOR, OUTPUT);

  // Pines de modos M1/M2/M3 (suelen ser señales lógicas, no banco de relés)
  pinMode(PIN_MODO_M1, OUTPUT);
  pinMode(PIN_MODO_M2, OUTPUT);
  pinMode(PIN_MODO_M3, OUTPUT);

  // Estados seguros al arrancar
  digitalWrite(PIN_VALVULA_1, releOff());
  digitalWrite(PIN_VALVULA_2, releOff());
  digitalWrite(PIN_BOMBA,     releOff());
  digitalWrite(PIN_ZUMBADOR,  releOff());

  // Modo de presscontrol: apaga todos
  digitalWrite(PIN_MODO_M1, LOW);
  digitalWrite(PIN_MODO_M2, LOW);
  digitalWrite(PIN_MODO_M3, LOW);

  // Refleja estados globales
  electrovalvula1 = false;
  electrovalvula2 = false;
  bombaEncendida  = false;
  zumbadorActivo  = false;

#ifdef PIN_EV_SEGURIDAD
  pinMode(PIN_EV_SEGURIDAD, OUTPUT);
  // Por defecto, EV seguridad cerrada (corte) = ON si el relé es activo bajo.
  digitalWrite(PIN_EV_SEGURIDAD, releOn());
#endif
}

void activarValvula(int id, bool on) {
  uint8_t pin = 255;
  bool* estadoPtr = nullptr;

  if (id == 1) { pin = PIN_VALVULA_1; estadoPtr = &electrovalvula1; }
  else if (id == 2) { pin = PIN_VALVULA_2; estadoPtr = &electrovalvula2; }
  else return; // id inválido

  digitalWrite(pin, on ? releOn() : releOff());
  if (estadoPtr) *estadoPtr = on;
}

void activarBomba(bool on) {
  digitalWrite(PIN_BOMBA, on ? releOn() : releOff());
  bombaEncendida = on;
}

void activarZumbador(bool on) {
  digitalWrite(PIN_ZUMBADOR, on ? releOn() : releOff());
  zumbadorActivo = on;
}

void setPressMode(PressMode pm) {
  // Mutuamente exclusivo: solo un pin M* en HIGH
  // Nota: estos pines suelen ser lógicos (no relés); mantenemos HIGH=activo.
  digitalWrite(PIN_MODO_M1, LOW);
  digitalWrite(PIN_MODO_M2, LOW);
  digitalWrite(PIN_MODO_M3, LOW);

  switch (pm) {
    case PM_M1: digitalWrite(PIN_MODO_M1, HIGH); break;
    case PM_M2: digitalWrite(PIN_MODO_M2, HIGH); break;
    case PM_M3: digitalWrite(PIN_MODO_M3, HIGH); break;
  }
}

void setEVSeguridad(bool cerrada) {
#ifdef PIN_EV_SEGURIDAD
  // cerrada=true ⇒ cortar llenado del Depósito Principal
  // Si el relé es activo-bajo, cerrada => ON (energizada) para forzar cierre.
  digitalWrite(PIN_EV_SEGURIDAD, cerrada ? releOn() : releOff());
#else
  // Si no tienes el pin definido, no hacer nada (evita errores de link)
  (void)cerrada;
#endif
}
