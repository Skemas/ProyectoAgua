#ifndef SENSORES_H
#define SENSORES_H

// Inicialización de hardware y ISRs de flujo
void initSensores();

// Lee sensores y actualiza variables globales en variables.h
void leerSensores();

// Medición del ultrasónico JSN-SR04T (cm). Devuelve -1 si timeout.
float medirDistancia();

#endif // SENSORES_H
