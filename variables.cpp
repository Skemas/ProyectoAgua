// variables.cpp
// Definición de TODAS las variables globales declaradas en variables.h.
// Importante: no vuelvas a definir estas variables en otros .cpp

#include "variables.h"

// --- Sensores / Medidas ---
float nivelAguaPct   = 0.0f;
float volumenAgua    = 0.0f;
float presion1       = 0.0f;
float presion2       = 0.0f;
float presion3       = 0.0f;
float flujoYF1       = 0.0f;
float flujoYF2       = 0.0f;
float distancia      = 0.0f;

// --- Actuadores / Estados ---
bool electrovalvula1 = false;
bool electrovalvula2 = false;
bool bombaEncendida  = false;
bool zumbadorActivo  = false;

// --- Consumos / Alertas ---
float consumoBomba   = 0.0f;
bool alertaSeguridad = false;

