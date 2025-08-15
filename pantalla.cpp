#include <Arduino.h>
#include <TFT_eSPI.h>

#include "pantalla.h"
#include "variables.h"
#include "modos.h"
#include "ajustes.h"
#include "constantes.h"
#include "modos_asistente.h"
#include "opciones.h"

// ====== Navegación básica de pantallas ======
enum Screen { MENU_PRINCIPAL, MONITOREO, MODOS, OPCIONES, AJUSTES, PROGRAMACION };
static Screen pantallaActual = MENU_PRINCIPAL;

// Layout Ajustes
#define AJS_X   12
#define AJS_Y   20
#define AJS_ROW 28

// Campos editables (índices)
enum AjsCampo {
  A_PPL = 0,   // Pulsos por litro
  A_VMAX,      // Volumen máx
  A_PON,       // Presion ON
  A_POFF,      // Presion OFF
  A_SOBR,      // Sobrellenado %
  A_EMAP,      // EMA Presion
  A_EMAN,      // EMA Nivel
  A_BRIL,      // Brillo
  A_COUNT
};
static int ajsIdx = 0; // campo seleccionado

// ====== Botones Menú Principal ======
#define BTN_MON_X  20
#define BTN_MON_Y  210
#define BTN_MON_W  135
#define BTN_MON_H  44

#define BTN_MOD_X  (BTN_MON_X + BTN_MON_W + 12)
#define BTN_MOD_Y  BTN_MON_Y
#define BTN_MOD_W  135
#define BTN_MOD_H  44

#define BTN_OPC_X  (BTN_MOD_X + BTN_MOD_W + 12)
#define BTN_OPC_Y  BTN_MON_Y
#define BTN_OPC_W  135
#define BTN_OPC_H  44

// ====== Botones comunes ======
#define BTN_VOLVER_X  370
#define BTN_VOLVER_Y  280
#define BTN_VOLVER_W  100
#define BTN_VOLVER_H  30

// ====== Botonera inferior de Ajustes ======
#define BTN_GUARDAR_X    10
#define BTN_GUARDAR_Y    BTN_VOLVER_Y
#define BTN_GUARDAR_W    100
#define BTN_GUARDAR_H    BTN_VOLVER_H

#define BTN_MAS_X    175
#define BTN_MAS_Y    BTN_VOLVER_Y
#define BTN_MAS_W    35
#define BTN_MAS_H    BTN_VOLVER_H

#define BTN_MENOS_X   215
#define BTN_MENOS_Y   BTN_VOLVER_Y
#define BTN_MENOS_W   35
#define BTN_MENOS_H   BTN_VOLVER_H

// ====== Opciones: botones ======
#define BTN_OP_AJS_X  20
#define BTN_OP_AJS_Y  70
#define BTN_OP_AJS_W  200
#define BTN_OP_AJS_H  40

#define BTN_OP_PRG_X  240
#define BTN_OP_PRG_Y  70
#define BTN_OP_PRG_W  200
#define BTN_OP_PRG_H  40

#define BTN_OP_ZB_X   20
#define BTN_OP_ZB_Y   130
#define BTN_OP_ZB_W   420
#define BTN_OP_ZB_H   40

#define BTN_OP_EV_X   20
#define BTN_OP_EV_Y   190
#define BTN_OP_EV_W   420
#define BTN_OP_EV_H   40

// ====== Utilidades UI ======
static inline bool touchDentro(int x, int y, int w, int h, uint16_t tx, uint16_t ty) {
  return (tx >= x) && (tx < x + w) && (ty >= y) && (ty < y + h);
}

static void botonRect(TFT_eSPI &tft, int x, int y, int w, int h, const char* texto) {
  tft.fillRoundRect(x, y, w, h, 8, TFT_DARKGREY);
  tft.setTextColor(TFT_WHITE, TFT_DARKGREY);
  tft.setTextSize(2);
  tft.setCursor(x + 10, y + (h/2) - 8);
  tft.print(texto);
}

static void dibujaBotonVolver(TFT_eSPI &tft) {
  botonRect(tft, BTN_VOLVER_X, BTN_VOLVER_Y, BTN_VOLVER_W, BTN_VOLVER_H, "Volver");
}

static void drawBadge(TFT_eSPI &tft, int x, int y, int r, uint16_t col) {
  tft.drawCircle(x, y, r, TFT_WHITE);
  tft.fillCircle(x, y, r-3, col);
}

// ====== DIBUJOS ======
static void dibujaMenuPrincipal(TFT_eSPI &tft) {
  tft.fillScreen(TFT_BLACK);

  // Título
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(3);
  tft.setCursor(16, 10);
  tft.println("Menu Principal");

  // Panel estados a la izquierda (bomba + nivel)
  tft.fillRoundRect(16, 54, 190, 130, 8, TFT_DARKGREY);
  tft.drawRoundRect(16, 54, 190, 130, 8, TFT_WHITE);

  // Estado bomba (círculo)
  uint16_t col = alertaSeguridad ? TFT_ORANGE : (bombaEncendida ? TFT_GREEN : TFT_RED);
  drawBadge(tft, 40, 88, 14, col);
  tft.setTextColor(TFT_WHITE, TFT_DARKGREY); tft.setTextSize(2);
  tft.setCursor(64, 78);
  if (alertaSeguridad)      tft.print("Bomba: FALLO");
  else if (bombaEncendida)  tft.print("Bomba: ON");
  else                      tft.print("Bomba: OFF");

  // Nivel depósito principal
  tft.setTextSize(2);
  tft.setCursor(24, 120);
  tft.printf("Nivel DP: %.0f%%", nivelAguaPct);

  // Recuadro “Modos activos” a la derecha
  tft.fillRoundRect(220, 54, 220, 130, 8, TFT_DARKGREY);
  tft.drawRoundRect(220, 54, 220, 130, 8, TFT_WHITE);
  tft.setTextColor(TFT_CYAN, TFT_DARKGREY); tft.setTextSize(2);
  tft.setCursor(230, 64); tft.print("Modos activos");
  tft.setTextColor(TFT_WHITE, TFT_DARKGREY); tft.setTextSize(1);

  // Usamos getters del asistente
  auto lSel  = getLlenadoSel();          // -1 nada, 0..3
  auto eSel  = getExtrActive();          // -1 nada, 0..2
  auto cSel  = getConsumoSel();          // -1 nada, 0..1

  // Llenado
  tft.setCursor(230, 88);
  if (lSel == -1) tft.print("Llenado: (ninguno)");
  else {
    const char* lnames[4] = { "L1 AG-ctrl","L2 AG-bp","L3 AP-ctrl","L4 AP-bp" };
    tft.print("Llenado: "); tft.print(lnames[lSel]);
  }

  // Extracción
  tft.setCursor(230, 106);
  if (eSel == -1) tft.print("Extracc.: (ninguna)");
  else {
    const char* enames[3] = { "DP","DA","DA-mant." };
    PressMode pm = getExtrPressFor((ExtraccionFuente)eSel);
    const char* pmn = (pm==PM_M1?"M1": pm==PM_M2?"M2":"M3");
    tft.print("Extracc.: "); tft.print(enames[eSel]); tft.print(" "); tft.print(pmn);
  }

  // Consumo
  tft.setCursor(230, 124);
  if (cSel == -1) tft.print("Consumo: (ninguno)");
  else {
    const char* cnames[2] = { "C1 AP", "C2 AG" };
    tft.print("Consumo: "); tft.print(cnames[cSel]);
  }

  // Botones
  botonRect(tft, BTN_MON_X, BTN_MON_Y, BTN_MON_W, BTN_MON_H, "Monitoreo");
  botonRect(tft, BTN_MOD_X, BTN_MOD_Y, BTN_MOD_W, BTN_MOD_H, "Modos");
  botonRect(tft, BTN_OPC_X, BTN_OPC_Y, BTN_OPC_W, BTN_OPC_H, "Opciones");
}

static void dibujaPantallaMonitoreo(TFT_eSPI &tft) {
  tft.fillScreen(TFT_BLACK);

  tft.setTextColor(TFT_CYAN);
  tft.setTextSize(3);
  tft.setCursor(80, 5);
  tft.println("Monitoreo");

  tft.setTextSize(2);
  tft.setTextColor(TFT_WHITE);
  int y = 40;

  tft.setCursor(10, y); tft.printf("Modo: %s\n", obtenerNombreModo(modoActual)); y += 22;
  tft.setCursor(10, y); tft.printf("Nivel: %.1f%% (%.1f L)\n", nivelAguaPct, volumenAgua); y += 22;
  tft.setCursor(10, y); tft.printf("Presion1: %.2f bar\n", presion1); y += 20;
  tft.setCursor(10, y); tft.printf("Presion2: %.2f bar\n", presion2); y += 20;
  tft.setCursor(10, y); tft.printf("Presion3: %.2f bar\n", presion3); y += 22;
  tft.setCursor(10, y); tft.printf("Flujo IN (YF1): %.2f L/min\n", flujoYF1); y += 20;
  tft.setCursor(10, y); tft.printf("Flujo OUT(YF2): %.2f L/min\n", flujoYF2); y += 22;
  tft.setCursor(10, y); tft.printf("Ultrasonico: %.1f cm\n", distancia); y += 22;

  tft.setCursor(10, y); tft.printf("Bomba: %s\n", bombaEncendida ? "ON" : "OFF"); y += 20;
  tft.setCursor(10, y); tft.printf("EV1: %s   EV2: %s\n",
                                   electrovalvula1 ? "ABIERTA" : "CERRADA",
                                   electrovalvula2 ? "ABIERTA" : "CERRADA"); y += 22;

  tft.setCursor(10, y); tft.printf("Consumo Bomba: %.1f W\n", consumoBomba); y += 24;

  tft.setTextColor(alertaSeguridad ? TFT_RED : TFT_GREEN);
  tft.setCursor(10, y); tft.printf("Seguridad: %s\n", alertaSeguridad ? "ALERTA" : "OK");

  // Indicadores de nivel tipo “semáforo” al final
  int indicadorY = 250, ancho = 60, espacio = 14, x = 10;
  tft.fillRoundRect(x, indicadorY, ancho, 24, 5, nivelAguaPct < 10 ? TFT_RED : TFT_DARKGREY);
  tft.setCursor(x + 6, indicadorY + 5); tft.setTextColor(TFT_WHITE); tft.print("Vacio"); x += ancho + espacio;

  tft.fillRoundRect(x, indicadorY, ancho, 24, 5, (nivelAguaPct >= 10 && nivelAguaPct < 50) ? TFT_YELLOW : TFT_DARKGREY);
  tft.setCursor(x + 6, indicadorY + 5); tft.print("Medio"); x += ancho + espacio;

  tft.fillRoundRect(x, indicadorY, ancho, 24, 5, (nivelAguaPct >= 50 && nivelAguaPct < 90) ? TFT_GREEN : TFT_DARKGREY);
  tft.setCursor(x + 6, indicadorY + 5); tft.print("Lleno"); x += ancho + espacio;

  tft.fillRoundRect(x, indicadorY, ancho, 24, 5, nivelAguaPct >= 90 ? TFT_RED : TFT_DARKGREY);
  tft.setCursor(x + 6, indicadorY + 5); tft.print("Sobre");

  // Botón Volver
  dibujaBotonVolver(tft);
}

// Delegado al asistente
static void dibujaPantallaModos(TFT_eSPI &tft) {
  modosAsistenteDraw(tft, modoActual);
}

// ====== OPCIONES ======
static void dibujaPantallaOpciones(TFT_eSPI &tft) {
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_ORANGE); tft.setTextSize(3);
  tft.setCursor(16, 8); tft.println("Opciones");

  // Botones Ajustes / Programacion
  botonRect(tft, BTN_OP_AJS_X, BTN_OP_AJS_Y, BTN_OP_AJS_W, BTN_OP_AJS_H, "Ajustes");
  botonRect(tft, BTN_OP_PRG_X, BTN_OP_PRG_Y, BTN_OP_PRG_W, BTN_OP_PRG_H, "Programacion");

  // Zumbador (On / Off / Silenciar)
  tft.fillRoundRect(BTN_OP_ZB_X, BTN_OP_ZB_Y, BTN_OP_ZB_W, BTN_OP_ZB_H, 8, TFT_DARKGREY);
  tft.setTextColor(TFT_WHITE, TFT_DARKGREY); tft.setTextSize(2);
  tft.setCursor(BTN_OP_ZB_X + 10, BTN_OP_ZB_Y + 10); tft.print("Zumbador:");

  int bx = BTN_OP_ZB_X + 170;
  int by = BTN_OP_ZB_Y + 6;
  int bw = 70, bh = 28, gp = 10;

  auto drawBtn = [&](const char* txt, bool active){
    uint16_t col = active ? TFT_GREEN : TFT_NAVY;
    tft.fillRoundRect(bx, by, bw, bh, 6, col);
    tft.setTextColor(TFT_WHITE, col); tft.setTextSize(2);
    tft.setCursor(bx + 10, by + 6); tft.print(txt);
    bx += bw + gp;
  };
  bool zbOn   = opcionesGetZumbador();
  bool zbMute = opcionesGetZumbadorMuted();
  drawBtn("On",  zbOn && !zbMute);
  drawBtn("Off", !zbOn && !zbMute);
  drawBtn("Sil", zbMute);

  // EV Seguridad (Auto / On / Off)
  tft.fillRoundRect(BTN_OP_EV_X, BTN_OP_EV_Y, BTN_OP_EV_W, BTN_OP_EV_H, 8, TFT_DARKGREY);
  tft.setTextColor(TFT_WHITE, TFT_DARKGREY); tft.setTextSize(2);
  tft.setCursor(BTN_OP_EV_X + 10, BTN_OP_EV_Y + 10); tft.print("EV Seguridad:");

  bx = BTN_OP_EV_X + 230; by = BTN_OP_EV_Y + 6;
  auto drawBtnEV = [&](const char* txt, EVSegMode mine){
    bool active = (opcionesGetEVSegMode() == mine);
    uint16_t col = active ? TFT_GREEN : TFT_NAVY;
    tft.fillRoundRect(bx, by, bw, bh, 6, col);
    tft.setTextColor(TFT_WHITE, col); tft.setTextSize(2);
    tft.setCursor(bx + 10, by + 6); tft.print(txt);
    bx += bw + gp;
  };
  drawBtnEV("Auto", EVSegMode::AUTO);
  drawBtnEV("On",   EVSegMode::ON);
  drawBtnEV("Off",  EVSegMode::OFF);

  // Pie de ayuda
  tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK); tft.setTextSize(1);
  tft.setCursor(18, 246);
  tft.print("Nota: 'Programacion' exige estar en Llenado desde A.P (control) o desde A.G (control)");

  dibujaBotonVolver(tft);
}

static void dibujaPantallaAjustes(TFT_eSPI &tft) {
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_ORANGE); tft.setTextSize(3);
  tft.setCursor(60, 8); tft.println("Ajustes");

  tft.setTextSize(2); tft.setTextColor(TFT_WHITE);
  int y = AJS_Y + 40;

  auto row = [&](const char* nombre, const char* val, bool sel){
    tft.setCursor(AJS_X, y);
    tft.setTextColor(sel ? TFT_YELLOW : TFT_WHITE);
    tft.printf("%c %-18s: %s", sel?'>':' ', nombre, val);
    y += AJS_ROW;
  };

  char buf[24];
  snprintf(buf, sizeof(buf), "%.0f", gAjustes.pulsosPorLitroYF);
  row("Pulsos/L (YF)", buf, ajsIdx==A_PPL);

  snprintf(buf, sizeof(buf), "%.0f L", gAjustes.volumenMaxL);
  row("Volumen max", buf, ajsIdx==A_VMAX);

  snprintf(buf, sizeof(buf), "%.2f bar", gAjustes.presionOnBar);
  row("P_ON (M3)", buf, ajsIdx==A_PON);

  snprintf(buf, sizeof(buf), "%.2f bar", gAjustes.presionOffBar);
  row("P_OFF (M3)", buf, ajsIdx==A_POFF);

  snprintf(buf, sizeof(buf), "%.1f %%", gAjustes.umbralSobrellenado);
  row("Sobrellenado", buf, ajsIdx==A_SOBR);

  snprintf(buf, sizeof(buf), "%.2f", gAjustes.emaPresionAlpha);
  row("EMA Presion", buf, ajsIdx==A_EMAP);

  snprintf(buf, sizeof(buf), "%.2f", gAjustes.emaNivelAlpha);
  row("EMA Nivel", buf, ajsIdx==A_EMAN);

  snprintf(buf, sizeof(buf), "%u", (unsigned)gAjustes.brillo);
  row("Brillo", buf, ajsIdx==A_BRIL);

  // Botones
  botonRect(tft, BTN_GUARDAR_X,  BTN_GUARDAR_Y,  BTN_GUARDAR_W,  BTN_GUARDAR_H,  "Guardar");
  botonRect(tft, BTN_MAS_X,      BTN_MAS_Y,      BTN_MAS_W,      BTN_MAS_H,      "+");
  botonRect(tft, BTN_MENOS_X,    BTN_MENOS_Y,    BTN_MENOS_W,    BTN_MENOS_H,    "-");

  // Botón Volver
  dibujaBotonVolver(tft);
}

// ====== Setup Pantalla ======
void setupPantalla(TFT_eSPI &tft) {
  tft.init();

  // Si tienes calData propios, cámbialos aquí:
  uint16_t calData[5] = { 300, 3500, 290, 3600, 7 };
  tft.setTouch(calData);   // requiere TOUCH_CS definido en User_Setup.h de TFT_eSPI

  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);

  opcionesInit();       // <<< importante: inicializa estado de Opciones
  modosAsistenteInit(); // inicializa estado del asistente
  dibujaMenuPrincipal(tft);
}

// ====== Loop Pantalla ======
void loopPantalla(TFT_eSPI &tft) {
  uint16_t tx, ty;
  if (tft.getTouch(&tx, &ty)) {

    // --- Menú principal ---
    if (pantallaActual == MENU_PRINCIPAL) {
      if (touchDentro(BTN_MON_X, BTN_MON_Y, BTN_MON_W, BTN_MON_H, tx, ty)) {
        pantallaActual = MONITOREO;
        tft.fillScreen(TFT_BLACK);
        dibujaPantallaMonitoreo(tft);
        return;
      }
      if (touchDentro(BTN_MOD_X, BTN_MOD_Y, BTN_MOD_W, BTN_MOD_H, tx, ty)) {
        pantallaActual = MODOS;
        tft.fillScreen(TFT_BLACK);
        dibujaPantallaModos(tft);  // delega al asistente
        return;
      }
      if (touchDentro(BTN_OPC_X, BTN_OPC_Y, BTN_OPC_W, BTN_OPC_H, tx, ty)) {
        pantallaActual = OPCIONES;
        tft.fillScreen(TFT_BLACK);
        dibujaPantallaOpciones(tft);
        return;
      }
    }

    // --- Interacción en MONITOREO: volver ---
    if (pantallaActual == MONITOREO &&
        touchDentro(BTN_VOLVER_X, BTN_VOLVER_Y, BTN_VOLVER_W, BTN_VOLVER_H, tx, ty)) {
      pantallaActual = MENU_PRINCIPAL;
      tft.fillScreen(TFT_BLACK);
      dibujaMenuPrincipal(tft);
      return;
    }

    // --- Interacción en MODOS (delegada al asistente) ---
    if (pantallaActual == MODOS) {
      if (modosAsistenteTouch(tft, tx, ty, modoActual)) {
        // El asistente ha pedido volver al menú
        pantallaActual = MENU_PRINCIPAL;
        tft.fillScreen(TFT_BLACK);
        dibujaMenuPrincipal(tft);
      }
      return;
    }

    // --- Interacción en OPCIONES ---
    if (pantallaActual == OPCIONES) {
      // Volver
      if (touchDentro(BTN_VOLVER_X, BTN_VOLVER_Y, BTN_VOLVER_W, BTN_VOLVER_H, tx, ty)) {
        pantallaActual = MENU_PRINCIPAL;
        tft.fillScreen(TFT_BLACK);
        dibujaMenuPrincipal(tft);
        return;
      }
      // Ajustes
      if (touchDentro(BTN_OP_AJS_X, BTN_OP_AJS_Y, BTN_OP_AJS_W, BTN_OP_AJS_H, tx, ty)) {
        pantallaActual = AJUSTES;
        tft.fillScreen(TFT_BLACK);
        dibujaPantallaAjustes(tft);
        return;
      }
      // Programación (UI dummy por ahora)
      if (touchDentro(BTN_OP_PRG_X, BTN_OP_PRG_Y, BTN_OP_PRG_W, BTN_OP_PRG_H, tx, ty)) {
        pantallaActual = PROGRAMACION;
        tft.fillScreen(TFT_BLACK);
        tft.setTextColor(TFT_ORANGE); tft.setTextSize(3);
        tft.setCursor(16, 8); tft.println("Programacion");
        tft.setTextColor(TFT_WHITE); tft.setTextSize(2);
        tft.setCursor(16, 60);
        tft.println("Configurar fecha, horario y litros a aportar");
        tft.setTextSize(1);
        tft.setCursor(16, 90);
        tft.println("Requisito: estar en Llenado desde A.P (control) o A.G (control).");
        dibujaBotonVolver(tft);
        return;
      }
      // Zumbador: 3 zonas dentro del botón ancho
      if (touchDentro(BTN_OP_ZB_X, BTN_OP_ZB_Y, BTN_OP_ZB_W, BTN_OP_ZB_H, tx, ty)) {
        int bx = BTN_OP_ZB_X + 170;
        int by = BTN_OP_ZB_Y + 6;
        int bw = 70, bh = 28, gp = 10;
        // On
        if (touchDentro(bx, by, bw, bh, tx, ty)) {
          opcionesSetZumbador(true);  opcionesSetZumbadorMuted(false);
        } else if (touchDentro(bx+bw+gp, by, bw, bh, tx, ty)) {
          opcionesSetZumbador(false); opcionesSetZumbadorMuted(false);
        } else if (touchDentro(bx+2*(bw+gp), by, bw, bh, tx, ty)) {
          opcionesSetZumbadorMuted(true);
        }
        dibujaPantallaOpciones(tft);
        return;
      }
      // EV Seguridad: 3 zonas
      if (touchDentro(BTN_OP_EV_X, BTN_OP_EV_Y, BTN_OP_EV_W, BTN_OP_EV_H, tx, ty)) {
        int bx = BTN_OP_EV_X + 230;
        int by = BTN_OP_EV_Y + 6;
        int bw = 70, bh = 28, gp = 10;
        if (touchDentro(bx, by, bw, bh, tx, ty)) {
          opcionesSetEVSegMode(EVSegMode::AUTO);
        } else if (touchDentro(bx+bw+gp, by, bw, bh, tx, ty)) {
          opcionesSetEVSegMode(EVSegMode::ON);
        } else if (touchDentro(bx+2*(bw+gp), by, bw, bh, tx, ty)) {
          opcionesSetEVSegMode(EVSegMode::OFF);
        }
        dibujaPantallaOpciones(tft);
        return;
      }
      return;
    }

    // --- Interacción en AJUSTES ---
    if (pantallaActual == AJUSTES) {
      // Volver
      if (touchDentro(BTN_VOLVER_X, BTN_VOLVER_Y, BTN_VOLVER_W, BTN_VOLVER_H, tx, ty)) {
        pantallaActual = OPCIONES;
        tft.fillScreen(TFT_BLACK);
        dibujaPantallaOpciones(tft);
        return;
      }

      // Navegación por filas
      int y = AJS_Y + 40;
      for (int i=0;i<A_COUNT;i++) {
        int rowY = y + i*AJS_ROW - 5;
        if (ty >= rowY && ty <= rowY + AJS_ROW) {
          ajsIdx = i;
          dibujaPantallaAjustes(tft);
          return;
        }
      }

      // Guardar
      if (touchDentro(BTN_GUARDAR_X, BTN_GUARDAR_Y, BTN_GUARDAR_W, BTN_GUARDAR_H, tx, ty)) {
        guardarAjustes();
        tft.setTextColor(TFT_GREEN); tft.setTextSize(1);
        tft.setCursor(BTN_GUARDAR_X, BTN_GUARDAR_Y - 16);
        tft.print("Guardado!");
        return;
      }

      // +
      if (touchDentro(BTN_MAS_X, BTN_MAS_Y, BTN_MAS_W, BTN_MAS_H, tx, ty)) {
        switch (ajsIdx) {
          case A_PPL:  gAjustes.pulsosPorLitroYF += 10; break;
          case A_VMAX: gAjustes.volumenMaxL += 10;      break;
          case A_PON:  gAjustes.presionOnBar += 0.05f;  break;
          case A_POFF: gAjustes.presionOffBar += 0.05f; break;
          case A_SOBR: gAjustes.umbralSobrellenado += 1; break;
          case A_EMAP: gAjustes.emaPresionAlpha = min(0.95f, gAjustes.emaPresionAlpha+0.05f); break;
          case A_EMAN: gAjustes.emaNivelAlpha   = min(0.95f, gAjustes.emaNivelAlpha+0.05f);   break;
          case A_BRIL: gAjustes.brillo = (uint8_t)min(255, (int)gAjustes.brillo + 5);  break;
        }
        dibujaPantallaAjustes(tft);
        return;
      }

      // -
      if (touchDentro(BTN_MENOS_X, BTN_MENOS_Y, BTN_MENOS_W, BTN_MENOS_H, tx, ty)) {
        switch (ajsIdx) {
          case A_PPL:  gAjustes.pulsosPorLitroYF = max(100.0f, gAjustes.pulsosPorLitroYF-10); break;
          case A_VMAX: gAjustes.volumenMaxL      = max(100.0f, gAjustes.volumenMaxL-10);     break;
          case A_PON:  gAjustes.presionOnBar     = max(0.5f, gAjustes.presionOnBar-0.05f);   break;
          case A_POFF: gAjustes.presionOffBar    = max(gAjustes.presionOnBar+0.05f, gAjustes.presionOffBar-0.05f); break;
          case A_SOBR: gAjustes.umbralSobrellenado = max(10.0f, gAjustes.umbralSobrellenado-1); break;
          case A_EMAP: gAjustes.emaPresionAlpha  = max(0.05f, gAjustes.emaPresionAlpha-0.05f); break;
          case A_EMAN: gAjustes.emaNivelAlpha    = max(0.05f, gAjustes.emaNivelAlpha-0.05f);   break;
          case A_BRIL: gAjustes.brillo = (uint8_t)max(0, (int)gAjustes.brillo - 5); break;
        }
        dibujaPantallaAjustes(tft);
        return;
      }
    }

    // --- Programación: sólo volver por ahora ---
    if (pantallaActual == PROGRAMACION &&
        touchDentro(BTN_VOLVER_X, BTN_VOLVER_Y, BTN_VOLVER_W, BTN_VOLVER_H, tx, ty)) {
      pantallaActual = OPCIONES;
      tft.fillScreen(TFT_BLACK);
      dibujaPantallaOpciones(tft);
      return;
    }

  } // fin if getTouch

  delay(60);
}
