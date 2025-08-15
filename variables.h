// variables.h
#ifndef VARIABLES_H
#define VARIABLES_H
#pragma once

// Este archivo declara TODAS las variables globales del proyecto.
// Las definiciones están en variables.cpp para evitar duplicaciones de linkage.

// --- Sensores / Medidas ---
extern float nivelAguaPct;   // % de nivel de agua calculado
extern float volumenAgua;    // Volumen estimado (si lo usas; inicialízalo a 0 por defecto)
extern float presion1;       // bar - Aportación 1
extern float presion2;       // bar - Aportación 2
extern float presion3;       // bar - Consumo / Salida
extern float flujoYF1;       // L/min - Flujo aportación (YF-B10)
extern float flujoYF2;       // L/min - Flujo consumo (YF-B10)
extern float distancia;      // cm - Lectura ultrasónico

// --- Actuadores / Estados ---
extern bool electrovalvula1;
extern bool electrovalvula2;
extern bool bombaEncendida;
extern bool zumbadorActivo;

// --- Consumos / Alertas ---
extern float consumoBomba;   // W - si mides con SCT-013
extern bool alertaSeguridad; // bandera global de seguridad

#endif // VARIABLES_H
