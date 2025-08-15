
#pragma once
#include <TFT_eSPI.h>

// Inicializa la pantalla y dibuja el menú principal
void setupPantalla(TFT_eSPI &tft);

// Bucle principal de gestión de la pantalla
void loopPantalla(TFT_eSPI &tft);