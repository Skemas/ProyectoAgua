#ifndef COMUNICACION_H
#define COMUNICACION_H

// Activa/desactiva rápidamente la pila de red sin tocar código
#define USE_WIFI_MQTT 0   // 0 = desactivado; 1 = WiFi+MQTT

void initComunicacion();
void gestionarComunicacion();

#endif // COMUNICACION_H
