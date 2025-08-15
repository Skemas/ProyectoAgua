#include <Arduino.h>
#include <TFT_eSPI.h>

#include "modos_asistente.h"
#include "modos.h"
#include "constantes.h"
#include "actuadores.h"

// ========== ESTADO PERSISTENTE DEL ASISTENTE (no se reinicia al re-entrar) ==========

// Llenado (L1..L4): -1 = nada seleccionado
static int8_t gLlenadoSel = -1;

// Extracción: fuente activa -1 = ninguna (0=DP,1=DA,2=DA_MANT)
// y Presscontrol por fuente (M1 por defecto la 1ª vez)
int8_t gExtrActive = -1;
static PressMode gPressBySrc[3] = { PM_M1, PM_M1, PM_M1 };

// Consumo: -1 = nada, 0 = C1 (AP), 1 = C2 (AG)
static int8_t gConsumoSel = -1;

// Pestañas
enum class Tab : uint8_t { LLENADO = 0, EXTRACCION = 1, CONSUMO = 2 };
static Tab gTab = Tab::LLENADO;

// ========== LAYOUT SENCILLO ==========
static const int TAB_Y = 6;
static const int TAB_W = 110;
static const int TAB_H = 28;
static const int TAB_X0 = 10;
static const int TAB_GAP = 6;

static const int ROW_X = 14;
static const int ROW_Y0 = 50;
static const int ROW_W = 360;
static const int ROW_H = 46;
static const int ROW_GAP = 8;

static const int CHECK_R = 8;

static const int BTN_BACK_X = 370;
static const int BTN_BACK_Y = 280;
static const int BTN_BACK_W = 100;
static const int BTN_BACK_H = 30;

// Info box
static const int INFO_X = 14;
static const int INFO_Y = 210;
static const int INFO_W = 456 - 2 * INFO_X;
static const int INFO_H = 96;

// ========== HELPERS UI ==========
static inline bool hit(int x, int y, int w, int h, uint16_t tx, uint16_t ty) {
  return (tx >= x && tx < x + w && ty >= y && ty < y + h);
}

static void drawTab(TFT_eSPI &tft, int x, const char* label, bool active) {
  tft.fillRoundRect(x, TAB_Y, TAB_W, TAB_H, 6, active ? TFT_BLUE : TFT_DARKGREY);
  tft.setTextColor(TFT_WHITE, active ? TFT_BLUE : TFT_DARKGREY);
  tft.setTextSize(1);
  int tx = x + 10;
  int ty = TAB_Y + 8;
  tft.setCursor(tx, ty);
  tft.print(label);
}

static void drawCheck(TFT_eSPI &tft, int cx, int cy, bool on) {
  tft.drawCircle(cx, cy, CHECK_R, TFT_WHITE);
  if (on) tft.fillCircle(cx, cy, CHECK_R - 3, TFT_GREEN);
}

static void drawRow(TFT_eSPI &tft, int y, const char* title, const char* subtitle, bool checked) {
  // fondo
  tft.fillRoundRect(ROW_X, y, ROW_W, ROW_H, 6, TFT_NAVY);
  // check
  int cx = ROW_X + 16; 
  int cy = y + ROW_H / 2;
  drawCheck(tft, cx, cy, checked);
  // textos
  tft.setTextColor(TFT_WHITE, TFT_NAVY);
  tft.setTextSize(2);
  tft.setCursor(ROW_X + 36, y + 6);
  tft.print(title);
  tft.setTextSize(1);
  tft.setCursor(ROW_X + 36, y + 26);
  tft.print(subtitle);
}

static void infoBox(TFT_eSPI &tft, const char* l1, const char* l2, const char* l3, const char* l4=nullptr) {
  tft.fillRoundRect(INFO_X, INFO_Y, INFO_W, INFO_H, 6, TFT_BLUE);
  tft.drawRoundRect(INFO_X, INFO_Y, INFO_W, INFO_H, 6, TFT_WHITE);
  tft.setTextColor(TFT_WHITE, TFT_BLUE);
  tft.setTextSize(1);
  int y = INFO_Y + 8;
  tft.setCursor(INFO_X + 8, y); tft.print(l1); y += 16;
  tft.setCursor(INFO_X + 8, y); tft.print(l2); y += 16;
  tft.setCursor(INFO_X + 8, y); tft.print(l3); y += 16;
  if (l4) { tft.setCursor(INFO_X + 8, y); tft.print(l4); }
}

// ========== TEXTOS SEGÚN MODO (válvulas) ==========
// Llenado L1..L4
static void infoLlenado(TFT_eSPI &tft, int8_t sel) {
  switch (sel) {
    case 0: // L1
      infoBox(tft,
        "L1: Desde deposito auxiliar (A.G), por sistema de control",
        "Valvulas abiertas: (2, 4, 7, 37)",
        "Valvulas cerradas: (3, 39)");
      break;
    case 1: // L2
      infoBox(tft,
        "L2: Desde deposito auxiliar (A.G), por bypass",
        "Valvulas abiertas: (2, 5, 37)",
        "Valvulas cerradas: (3, 4, 7, 39)");
      break;
    case 2: // L3
      infoBox(tft,
        "L3: Desde red presurizada (A.P), por sistema de control",
        "Valvulas abiertas: (1, 6, 7, 37)",
        "Valvulas cerradas: (3, 5, 8, 39)");
      break;
    case 3: // L4
      infoBox(tft,
        "L4: Desde red presurizada (A.P), por bypass",
        "Valvulas abiertas: (3, 5, 37)",
        "Valvulas cerradas: (4, 6, 7, 8, 39)");
      break;
    default:
      infoBox(tft, "Selecciona un modo de llenado", "", "");
      break;
  }
}

// Extraccion por fuente
static void infoExtr(TFT_eSPI &tft, int8_t src) {
  switch (src) {
    case 0: // DP
      infoBox(tft,
        "E1x: Extraccion desde deposito principal (M1/M2/M3)",
        "Valvulas abiertas: (9, 38)",
        "Valvulas cerradas: (8, 39)");
      break;
    case 1: // DA
      infoBox(tft,
        "E2x: Extraccion desde deposito auxiliar (M1/M2/M3)",
        "Valvulas abiertas: (2, 4, 7, 9, 25, 26, 39)",
        "Valvulas cerradas: (3, 5, 8, 37, 38)");
      break;
    case 2: // Mantenimiento
      infoBox(tft,
        "E3: Extraccion desde auxiliar via mantenimiento",
        "Valvulas abiertas: (5, 39)",
        "Valvulas cerradas: (4, 6, 7, 37, 38)");
      break;
    default:
      infoBox(tft, "Selecciona una fuente de extraccion", "", "");
      break;
  }
}

// Consumo C1/C2
static void infoConsumo(TFT_eSPI &tft, int8_t sel) {
  switch (sel) {
    case 0: // C1
      infoBox(tft,
        "C1: Consumo desde A.P",
        "Valvulas abiertas: (1, 8)",
        "Valvulas cerradas: (3, 6, 9)");
      break;
    case 1: // C2
      infoBox(tft,
        "C2: Consumo desde A.G",
        "Valvulas abiertas: (2, 3, 8, 25, 26)",
        "Valvulas cerradas: (1, 4, 5, 6, 9)");
      break;
    default:
      infoBox(tft, "Selecciona un modo de consumo directo", "", "");
      break;
  }
}

// ========== LÓGICA (dispositivos) ==========

void setPressModeFor(ExtraccionFuente src, PressMode pm) {
  int i = (int)src;
  if (i >= 0 && i < 3) gPressBySrc[i] = pm;
}

// (Opcional) aquí puedes aplicar válvulas/bomba según fuente y presscontrol
void aplicarLogicaExtraccion(ExtraccionFuente src, PressMode pm) {
  // Ejemplo (ajústalo a tu hardware real)
  switch (src) {
    case ExtraccionFuente::DP:
      activarValvula(1, false);
      activarValvula(2, false);
      break;
    case ExtraccionFuente::DA:
      activarValvula(1, true);
      activarValvula(2, false);
      break;
    case ExtraccionFuente::DA_MANT:
      activarValvula(1, false);
      activarValvula(2, true);
      break;
  }
  // Presscontrol (mutua exclusion por pines)
  setPressMode(pm); // usa tu función central (PM_M1/PM_M2/PM_M3)

  // Si PM_M3 implica gestión de bomba por ESP32, lo decides aquí.
  if (pm == PM_M3) {
    // activarBomba(true/false) si corresponde…
  }
}

// ========== DIBUJO ==========

static void drawHeader(TFT_eSPI &tft, const char* title) {
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_ORANGE);
  tft.setTextSize(3);
  tft.setCursor(18, 8);
  tft.print(title);
}

static void drawTabs(TFT_eSPI &tft) {
  int x = TAB_X0;
  drawTab(tft, x, "Llenado",    gTab == Tab::LLENADO);    x += TAB_W + TAB_GAP;
  drawTab(tft, x, "Extraccion", gTab == Tab::EXTRACCION); x += TAB_W + TAB_GAP;
  drawTab(tft, x, "Consumo",    gTab == Tab::CONSUMO);
}

static void drawTabLlenado(TFT_eSPI &tft) {
  drawHeader(tft, "Modos: Llenado");
  drawTabs(tft);
  int y = ROW_Y0;
  // Títulos más cortos y subtítulos con detalle (para que quepan)
  drawRow(tft, y,               "L1: A.G (control)",  "Desde deposito auxiliar por sistema de control", gLlenadoSel==0); y += ROW_H + ROW_GAP;
  drawRow(tft, y,               "L2: A.G (bypass)",   "Desde deposito auxiliar por bypass",             gLlenadoSel==1); y += ROW_H + ROW_GAP;
  drawRow(tft, y,               "L3: A.P (control)",  "Desde red presurizada por sistema de control",   gLlenadoSel==2); y += ROW_H + ROW_GAP;
  drawRow(tft, y,               "L4: A.P (bypass)",   "Desde red presurizada por bypass",               gLlenadoSel==3);
  infoLlenado(tft, gLlenadoSel);

  // Botón Volver
  tft.fillRoundRect(BTN_BACK_X, BTN_BACK_Y, BTN_BACK_W, BTN_BACK_H, 6, TFT_DARKGREY);
  tft.setTextColor(TFT_WHITE, TFT_DARKGREY); tft.setTextSize(2);
  tft.setCursor(BTN_BACK_X + 12, BTN_BACK_Y + 6); tft.print("Volver");
}

static void drawPressRow(TFT_eSPI &tft, int y, int srcIndex) {
  // Título según fuente
  const char* title =
    (srcIndex==0) ? "Deposito Principal" :
    (srcIndex==1) ? "Deposito Auxiliar" :
                    "Auxiliar (mant.)";

  bool active = (gExtrActive == srcIndex);
  drawRow(tft, y, title, "Selecciona Presscontrol M1/M2/M3", active);

  // Botoncitos M1/M2/M3 dentro de la fila
  int bx = ROW_X + 260;
  int by = y + 10;
  int bw = 28, bh = 24, gap = 8;

  PressMode pm = gPressBySrc[srcIndex];
  auto drawBtn = [&](const char* txt, PressMode mine){
    uint16_t col = (pm==mine) ? TFT_GREEN : TFT_DARKGREY;
    tft.fillRoundRect(bx, by, bw, bh, 4, col);
    tft.setTextColor(TFT_WHITE, col); tft.setTextSize(1);
    int tx = bx + ((bw - (int)strlen(txt)*6)/2);
    tft.setCursor(tx<bx+4?bx+6:tx, by+7);
    tft.print(txt);
    bx += bw + gap;
  };
  drawBtn("M1", PM_M1);
  drawBtn("M2", PM_M2);
  drawBtn("M3", PM_M3);
}

static void drawTabExtraccion(TFT_eSPI &tft) {
  drawHeader(tft, "Modos: Extraccion");
  drawTabs(tft);

  int y = ROW_Y0;
  drawPressRow(tft, y,   0); y += ROW_H + ROW_GAP;
  drawPressRow(tft, y,   1); y += ROW_H + ROW_GAP;
  drawPressRow(tft, y,   2);

  infoExtr(tft, gExtrActive);

  // Volver
  tft.fillRoundRect(BTN_BACK_X, BTN_BACK_Y, BTN_BACK_W, BTN_BACK_H, 6, TFT_DARKGREY);
  tft.setTextColor(TFT_WHITE, TFT_DARKGREY); tft.setTextSize(2);
  tft.setCursor(BTN_BACK_X + 12, BTN_BACK_Y + 6); tft.print("Volver");
}

static void drawTabConsumo(TFT_eSPI &tft) {
  drawHeader(tft, "Modos: Consumo Directo");
  drawTabs(tft);

  int y = ROW_Y0;
  drawRow(tft, y,               "C1: Desde A.P", "Consumo directo desde red presurizada", gConsumoSel==0); y += ROW_H + ROW_GAP;
  drawRow(tft, y,               "C2: Desde A.G", "Consumo directo desde deposito auxiliar", gConsumoSel==1);

  infoConsumo(tft, gConsumoSel);

  // Volver
  tft.fillRoundRect(BTN_BACK_X, BTN_BACK_Y, BTN_BACK_W, BTN_BACK_H, 6, TFT_DARKGREY);
  tft.setTextColor(TFT_WHITE, TFT_DARKGREY); tft.setTextSize(2);
  tft.setCursor(BTN_BACK_X + 12, BTN_BACK_Y + 6); tft.print("Volver");
}

// API pública
void modosAsistenteDraw(TFT_eSPI &tft, ModoOperacion /*modoActual*/) {
  switch (gTab) {
    case Tab::LLENADO:    drawTabLlenado(tft);    break;
    case Tab::EXTRACCION: drawTabExtraccion(tft); break;
    case Tab::CONSUMO:    drawTabConsumo(tft);    break;
  }
}

void modosAsistenteInit() {
  // No reseteamos selecciones para conservar “la última usada”.
  // Solo podrías forzar defaults aquí si quisieras.
}

// ========== TOUCH ==========

static bool touchTabs(uint16_t tx, uint16_t ty) {
  int x = TAB_X0;
  if (hit(x, TAB_Y, TAB_W, TAB_H, tx, ty)) { gTab = Tab::LLENADO; return true; }
  x += TAB_W + TAB_GAP;
  if (hit(x, TAB_Y, TAB_W, TAB_H, tx, ty)) { gTab = Tab::EXTRACCION; return true; }
  x += TAB_W + TAB_GAP;
  if (hit(x, TAB_Y, TAB_W, TAB_H, tx, ty)) { gTab = Tab::CONSUMO; return true; }
  return false;
}

bool modosAsistenteTouch(TFT_eSPI &tft, uint16_t tx, uint16_t ty, ModoOperacion /*modoActual*/) {
  // Tabs
  if (touchTabs(tx, ty)) { modosAsistenteDraw(tft, MODO_NINGUNO); return false; }

  // Volver
  if (hit(BTN_BACK_X, BTN_BACK_Y, BTN_BACK_W, BTN_BACK_H, tx, ty)) {
    return true; // pedir volver al menú principal
  }

  // Secciones
  if (gTab == Tab::LLENADO) {
    int y = ROW_Y0;
    for (int i=0;i<4;i++) {
      if (hit(ROW_X, y, ROW_W, ROW_H, tx, ty)) {
        gLlenadoSel = i;  // selección exclusiva en Llenado
        modosAsistenteDraw(tft, MODO_NINGUNO);
        return false;
      }
      y += ROW_H + ROW_GAP;
    }
  }
  else if (gTab == Tab::EXTRACCION) {
    // Click sobre fuente (fila completa)
    int y = ROW_Y0;
    for (int i=0;i<3;i++) {
      if (hit(ROW_X, y, ROW_W, ROW_H, tx, ty)) {
        gExtrActive = i;  // activa esa fuente (exclusiva)
        // Aplicar lógica con el press actual de esa fuente
        aplicarLogicaExtraccion((ExtraccionFuente)i, gPressBySrc[i]);
        modosAsistenteDraw(tft, MODO_NINGUNO);
        return false;
      }
      // Botoncitos M1/M2/M3 (zona derecha de la fila)
      int bx = ROW_X + 260;
      int by = y + 10;
      int bw = 28, bh = 24, gap = 8;
      // M1
      if (hit(bx, by, bw, bh, tx, ty)) {
        gPressBySrc[i] = PM_M1;
        if (gExtrActive == i) aplicarLogicaExtraccion((ExtraccionFuente)i, PM_M1);
        modosAsistenteDraw(tft, MODO_NINGUNO);
        return false;
      }
      bx += bw + gap;
      // M2
      if (hit(bx, by, bw, bh, tx, ty)) {
        gPressBySrc[i] = PM_M2;
        if (gExtrActive == i) aplicarLogicaExtraccion((ExtraccionFuente)i, PM_M2);
        modosAsistenteDraw(tft, MODO_NINGUNO);
        return false;
      }
      bx += bw + gap;
      // M3
      if (hit(bx, by, bw, bh, tx, ty)) {
        gPressBySrc[i] = PM_M3;
        if (gExtrActive == i) aplicarLogicaExtraccion((ExtraccionFuente)i, PM_M3);
        modosAsistenteDraw(tft, MODO_NINGUNO);
        return false;
      }

      y += ROW_H + ROW_GAP;
    }
  }
  else if (gTab == Tab::CONSUMO) {
    int y = ROW_Y0;
    for (int i=0;i<2;i++) {
      if (hit(ROW_X, y, ROW_W, ROW_H, tx, ty)) {
        gConsumoSel = i; // selección exclusiva en Consumo
        modosAsistenteDraw(tft, MODO_NINGUNO);
        return false;
      }
      y += ROW_H + ROW_GAP;
    }
  }

  return false; // quedarse en la pantalla de modos
}

      // ===== Getters para Menú Principal =====
    int8_t getLlenadoSel() {
      extern int8_t gLlenadoSel; // declarado estático en modos_asistente.cpp (arriba de todo)
      return gLlenadoSel;
}

    int8_t getExtrActive() {
      return gExtrActive;
}

    PressMode getExtrPressFor(ExtraccionFuente src) {
      extern PressMode gPressBySrc[3];
      int i = (int)src;
      if (i < 0 || i >= 3) return PM_M1;
      return gPressBySrc[i];
}

    int8_t getConsumoSel() {
      extern int8_t gConsumoSel;
      return gConsumoSel;
}
