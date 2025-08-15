#ifndef MODOS_H
#define MODOS_H

// Enum con todos los modos definidos
enum ModoOperacion {
  MODO_NINGUNO = 0,
  LLENADO_L1, LLENADO_L2, LLENADO_L3, LLENADO_L4,
  EXTRACCION_E1A, EXTRACCION_E1B, EXTRACCION_E1C,   // E1*: Principal (M1/M2/M3)
  EXTRACCION_E2A, EXTRACCION_E2B, EXTRACCION_E2C,   // E2*: Auxiliar  (M1/M2/M3)
  EXTRACCION_E3,                                    // Mantenimiento
  CONSUMO_C1, CONSUMO_C2
};

// Modo activo global (definido en modos.cpp)
extern ModoOperacion modoActual;

// Nombre legible del modo (para UI/logs)
const char* obtenerNombreModo(ModoOperacion);

// Activación / desactivación de modos (orquestan actuadores)
void activarModo(ModoOperacion);
void desactivarModo();

// Bucle de control de modos: gobierna la bomba según el tipo de presscontrol
void modosLoop();

#endif // MODOS_H
