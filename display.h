// display.h — ST7789 240×320 Display Module
// Board:   ESP32-S3 Super Mini
// Screen:  GMT020-02 (COG ST7789V, 240×320, SPI 4-line)
// Library: LovyanGFX (конфіг у lgfx_config.h — у папці проекту)
//
// 3 сторінки з авто-ротацією (7 с):
//   Page 0 — Dashboard:  SOC, напруга, струм, потужність, цикли, FET, температури, mini cell bars
//   Page 1 — Cell V:     детальна сітка напруг всіх клітин (до 24)
//   Page 2 — Status:     WiFi, BMS, захист, конфігурація
//
// Public API:
//   initDisplay()        → викликати в setup()
//   handleDisplay()      → викликати в loop()
//   nextDisplayPage()    → перемкнути сторінку (наприклад, кнопка GPIO9)
//   setDisplayBrightness(0-255)
//   setDisplayPower(bool)
//   setPageInterval(ms)
// ─────────────────────────────────────────────────────────

#pragma once

#include "lgfx_config.h"   // LGFX клас (SPI bus + ST7789V panel + PWM backlight)
#include <WiFi.h>
#include "GLOBAL_STATE.h"

// ─── Color Palette (RGB565) ───────────────────────────
#define C_BG       0x0841   // #0A0C20  фон
#define C_SUR      0x1082   // #102040  поверхня / картка
#define C_SUR2     0x18A3   // #183060  підвищена поверхня
#define C_BRD      0x2965   // border/separator
#define C_ACCENT   0x07FF   // #00D4FF  ціан
#define C_GREEN    0x07E0   // #00FF00  зелений
#define C_GREEN2   0x3666   // #33CC30  м'який зелений
#define C_YELLOW   0xFD20   // #FFA800  жовтий/попередження
#define C_RED      0xF800   // #FF0000  червоний/помилка
#define C_BLUE     0x001F   // #0000FF  холодний
#define C_WHITE    0xFFFF
#define C_LGRAY    0xAD75   // світло-сірий
#define C_GRAY     0x6B6D   // сірий
#define C_DGRAY    0x2965   // темно-сірий

// ─── Layout (Portrait 240×320) ────────────────────────
#define SCRW        240
#define SCRH        320
#define HDR_H        28     // висота хедера
#define FTR_H        26     // висота футера
#define CTX_Y       HDR_H
#define CTX_H       (SCRH - HDR_H - FTR_H)  // 266 px контенту

// ─── LovyanGFX instance ───────────────────────────────
// Конфіг SPI/Panel/Backlight — у lgfx_config.h (LGFX клас)
static LGFX _tft;

// ─── State ────────────────────────────────────────────
static uint8_t   _pg        = 0;
static uint8_t   _PG_CNT    = 3;
static uint32_t  _pgTimer   = 0;
static uint32_t  _PG_MS     = 7000;   // авто-перемикання мс
static uint32_t  _drawTimer = 0;
static uint32_t  _DRAW_MS   = 1000;   // частота оновлення мс
static uint8_t   _brt       = 220;    // яскравість 0-255
static bool      _inited    = false;

static const char* _pgNames[3] = { "Dashboard", "Cell V", "Status" };

// ─── Helpers ──────────────────────────────────────────
static inline uint16_t _socClr(uint8_t s) {
  if (s < 15) return C_RED;
  if (s < 30) return C_YELLOW;
  return C_GREEN2;
}
static inline uint16_t _tmpClr(float t) {
  if (t > 50) return C_RED;
  if (t > 40) return C_YELLOW;
  if (t <  5) return C_BLUE;
  return C_GREEN2;
}
static inline float _clamp(float v, float lo, float hi) {
  return v < lo ? lo : v > hi ? hi : v;
}

static void _bar(int16_t x, int16_t y, int16_t w, int16_t h,
                 float pct, uint16_t fill, uint16_t bg, uint16_t brd) {
  _tft.drawRect(x, y, w, h, brd);
  int16_t fw = (int16_t)((w-2) * _clamp(pct, 0.0f, 1.0f));
  if (fw > 0)   _tft.fillRect(x+1,    y+1, fw,      h-2, fill);
  if (fw < w-2) _tft.fillRect(x+1+fw, y+1, w-2-fw, h-2, bg);
}

static void _dot(int16_t cx, int16_t cy, int16_t r, bool on) {
  _tft.fillCircle(cx, cy, r,   on ? C_GREEN : C_RED);
  _tft.drawCircle(cx, cy, r+1, on ? C_GREEN2: C_RED);
}

// ─── Header ───────────────────────────────────────────
static void _hdr(const char* title) {
  _tft.fillRect(0, 0, SCRW, HDR_H, C_SUR);
  _tft.drawFastHLine(0, HDR_H-1, SCRW, C_BRD);

  _tft.setTextColor(C_ACCENT, C_SUR);
  _tft.setTextDatum(ML_DATUM);
  _tft.setTextFont(2);
  _tft.drawString(title, 8, HDR_H/2+1);

  // WiFi dot
  _dot(SCRW-38, HDR_H/2, 4, wifiConnected);
  _tft.setTextFont(1);
  _tft.setTextColor(C_GRAY, C_SUR);
  _tft.setTextDatum(ML_DATUM);
  _tft.drawString("W", SCRW-31, HDR_H/2+1);

  // BMS dot
  _dot(SCRW-18, HDR_H/2, 4, bmsConnected);
  _tft.drawString("B", SCRW-11, HDR_H/2+1);
}

// ─── Footer ───────────────────────────────────────────
static void _ftr(uint8_t pg) {
  int16_t fy = SCRH - FTR_H;
  _tft.fillRect(0, fy, SCRW, FTR_H, C_SUR);
  _tft.drawFastHLine(0, fy, SCRW, C_BRD);

  // Page dots
  int16_t dx = SCRW/2 - (_PG_CNT*12)/2;
  for (uint8_t i=0; i<_PG_CNT; i++) {
    bool act = (i==pg);
    _tft.fillCircle(dx+i*12, fy+FTR_H/2, act?5:3, act?C_ACCENT:C_DGRAY);
  }

  // Page name (right)
  _tft.setTextFont(1);
  _tft.setTextColor(C_GRAY, C_SUR);
  _tft.setTextDatum(MR_DATUM);
  _tft.drawString(_pgNames[pg], SCRW-6, fy+FTR_H/2+1);

  // Auto-rotate bar (left, 32px wide)
  uint32_t el = millis() - _pgTimer;
  float rem = 1.0f - _clamp((float)el/_PG_MS, 0.0f, 1.0f);
  _tft.fillRect(4, fy+FTR_H/2-2, 32, 4, C_BRD);
  int16_t bfw = (int16_t)(32*rem);
  if (bfw>0) _tft.fillRect(4, fy+FTR_H/2-2, bfw, 4, C_ACCENT);
}

// ═══════════════════════════════════════════════════════
//  PAGE 0 — DASHBOARD
//  y= 28.. 92  SOC + bar + Ah         (65px)
//  y= 93       divider
//  y= 94..173  Metrics 2×2            (80px)
//  y=174       divider
//  y=175..214  Temps + FET            (40px)
//  y=215       divider
//  y=216..293  Cell bars overview     (78px)
// ═══════════════════════════════════════════════════════
static void _pg0() {
  uint8_t  soc   = bmsData.soc;
  float    volt  = bmsData.totalVoltage;
  float    curr  = bmsData.current;
  bool     isChg = (curr < -0.05f);
  float    pwr   = fabsf(volt * curr);
  float    capR  = bmsData.capacityRemaining;
  float    capT  = bmsData.capacityTotal;
  uint16_t sc    = _socClr(soc);
  char     buf[24];

  // ── SOC (28..92) ──────────────────────────────────
  int16_t y0 = CTX_Y;
  _tft.fillRect(0, y0, SCRW, 65, C_BG);

  // Велике число SOC (font6 = 48px цифри)
  snprintf(buf, sizeof(buf), "%d%%", soc);
  _tft.setTextFont(6);
  _tft.setTextColor(sc, C_BG);
  _tft.setTextDatum(MC_DATUM);
  _tft.drawString(buf, 58, y0+28);

  // Ємність під числом
  snprintf(buf, sizeof(buf), "%.1f / %.1f Ah", capR, capT);
  _tft.setTextFont(1);
  _tft.setTextColor(C_GRAY, C_BG);
  _tft.setTextDatum(MC_DATUM);
  _tft.drawString(buf, 58, y0+56);

  // SOC бар (правий бік, 104×22px)
  _bar(126, y0+8, 108, 22, soc/100.0f, sc, C_SUR2, C_BRD);

  // Позначки 25%/50%/75%
  for (int8_t t : {25, 50, 75}) {
    int16_t tx = 127 + (int16_t)(106*t/100.0f);
    _tft.drawFastVLine(tx, y0+31, 5, C_DGRAY);
  }

  // Струм + напрямок
  snprintf(buf, sizeof(buf), "%s %.2fA", isChg?"<":">", fabsf(curr));
  _tft.setTextFont(1);
  _tft.setTextColor(isChg?C_GREEN:C_YELLOW, C_BG);
  _tft.setTextDatum(ML_DATUM);
  _tft.drawString(buf, 127, y0+45);

  // Балансування
  if (bmsData.balanceStatus || bmsData.balanceStatusHigh) {
    _tft.setTextColor(C_YELLOW, C_BG);
    _tft.setTextDatum(MR_DATUM);
    _tft.drawString("~BAL", SCRW-4, y0+45);
  }

  _tft.drawFastHLine(0, y0+65, SCRW, C_BRD);

  // ── Metrics 2×2 (94..173) ─────────────────────────
  int16_t mY = y0+66, hw = SCRW/2;

  struct Metric {
    float       v;
    const char* lbl;
    const char* unit;
    uint16_t    col;
    bool        isInt;
  } M[4] = {
    { volt,            "НАПРУГА",  "В",  C_ACCENT,  false },
    { fabsf(curr),     isChg?"ЗАРЯД":"РОЗРЯД", "А", C_YELLOW, false },
    { pwr,             "ПОТУЖНІСТЬ","Вт", C_GREEN2, true  },
    { (float)bmsData.cycleCount, "ЦИКЛИ","шт", C_GRAY, true },
  };

  for (uint8_t i=0; i<4; i++) {
    int16_t mx = (i%2)*hw, my = mY+(i/2)*40;
    _tft.fillRect(mx, my, hw, 40, C_BG);
    if (i%2==1) _tft.drawFastVLine(mx, my, 40, C_BRD);
    if (i/2==1) _tft.drawFastHLine(mx, my, hw, C_BRD);

    // Значення
    if (M[i].isInt) snprintf(buf, sizeof(buf), "%.0f", M[i].v);
    else            snprintf(buf, sizeof(buf), "%.2f", M[i].v);
    _tft.setTextFont(4);
    _tft.setTextColor(M[i].col, C_BG);
    _tft.setTextDatum(ML_DATUM);
    _tft.drawString(buf, mx+5, my+14);

    // Одиниця (праворуч вгорі)
    _tft.setTextFont(1);
    _tft.setTextColor(C_DGRAY, C_BG);
    _tft.setTextDatum(MR_DATUM);
    _tft.drawString(M[i].unit, mx+hw-3, my+8);

    // Підпис (ліворуч внизу)
    _tft.setTextColor(C_GRAY, C_BG);
    _tft.setTextDatum(ML_DATUM);
    _tft.drawString(M[i].lbl, mx+5, my+33);
  }
  _tft.drawFastHLine(0, mY+80, SCRW, C_BRD);

  // ── Temps + FET (175..214) ────────────────────────
  int16_t tY = mY+81;
  _tft.fillRect(0, tY, SCRW, 40, C_BG);

  // Температури (ліва половина)
  uint8_t tc = (bmsData.tempSensorCount < 3) ? bmsData.tempSensorCount : 3;
  if (tc > 0) {
    int16_t tCW = (SCRW/2) / tc;
    for (uint8_t i=0; i<tc; i++) {
      float t = bmsData.temperatures[i];
      snprintf(buf, sizeof(buf), "%.1f\xB0", t);  // °
      _tft.setTextFont(2);
      _tft.setTextColor(_tmpClr(t), C_BG);
      _tft.setTextDatum(MC_DATUM);
      _tft.drawString(buf, i*tCW + tCW/2, tY+13);
      _tft.setTextFont(1);
      _tft.setTextColor(C_GRAY, C_BG);
      char tn[4]; snprintf(tn, sizeof(tn), "T%d", i+1);
      _tft.drawString(tn, i*tCW + tCW/2, tY+28);
    }
  } else {
    _tft.setTextFont(1); _tft.setTextColor(C_DGRAY, C_BG);
    _tft.setTextDatum(MC_DATUM);
    _tft.drawString("T: n/a", SCRW/4, tY+20);
  }

  // FET (права половина)
  _tft.drawFastVLine(SCRW/2, tY, 40, C_BRD);
  bool chg = (bmsData.fetStatus>>0)&1, dsg = (bmsData.fetStatus>>1)&1;
  int16_t fX = SCRW/2+5;
  _tft.setTextFont(1); _tft.setTextDatum(ML_DATUM);

  _tft.setTextColor(C_GRAY, C_BG);  _tft.drawString("CHG:", fX, tY+12);
  _tft.setTextColor(chg?C_GREEN:C_RED, C_BG);
  _tft.drawString(chg?"ON ":"OFF", fX+28, tY+12);
  _dot(SCRW-12, tY+12, 3, chg);

  _tft.setTextColor(C_GRAY, C_BG);  _tft.drawString("DSG:", fX, tY+28);
  _tft.setTextColor(dsg?C_GREEN:C_RED, C_BG);
  _tft.drawString(dsg?"ON ":"OFF", fX+28, tY+28);
  _dot(SCRW-12, tY+28, 3, dsg);

  _tft.drawFastHLine(0, tY+40, SCRW, C_BRD);

  // ── Cell bars (216..293) ──────────────────────────
  int16_t cvY = tY+41;
  int16_t cvH = SCRH - FTR_H - cvY;
  _tft.fillRect(0, cvY, SCRW, cvH, C_BG);

  uint8_t n = bmsData.cellCount;
  if (n > 0 && n <= 24 && bmsConnected) {
    float mnV = bmsData.cellVoltages[0], mxV = mnV;
    for (uint8_t i=1;i<n;i++){
      if(bmsData.cellVoltages[i]<mnV) mnV=bmsData.cellVoltages[i];
      if(bmsData.cellVoltages[i]>mxV) mxV=bmsData.cellVoltages[i];
    }
    float rng = mxV-mnV; if(rng<0.001f) rng=0.001f;

    // Info line
    snprintf(buf, sizeof(buf), "%.3fV  \xCE\x94%.0fmV  %dS", volt, rng*1000, n);
    _tft.setTextFont(1); _tft.setTextColor(C_GRAY, C_BG);
    _tft.setTextDatum(MC_DATUM);
    _tft.drawString(buf, SCRW/2, cvY+7);

    // Вертикальні bars
    int16_t bW = (SCRW-4)/n, bH = cvH-18, bY0 = cvY+14;
    for (uint8_t i=0;i<n;i++){
      float v   = bmsData.cellVoltages[i];
      bool  bal = (i<16) ? ((bmsData.balanceStatus   >> i)     & 1)
                          : ((bmsData.balanceStatusHigh >> (i-16)) & 1);
      float pct = (v-mnV)/rng;
      uint16_t vc = (v==mxV)?C_GREEN:(v==mnV)?C_YELLOW:C_ACCENT;
      if(bal) vc = C_YELLOW;

      int16_t bx = 2+i*bW;
      _tft.fillRect(bx, bY0, bW-1, bH, C_SUR2);
      int16_t fh = (int16_t)(bH*_clamp(pct,0,1));
      if(fh<1) fh=1;
      _tft.fillRect(bx, bY0+bH-fh, bW-1, fh, vc);
      if(bal) _tft.fillRect(bx, bY0, bW-1, 2, C_YELLOW);

      // Номер клітини (якщо є місце)
      if(bW>=12){
        char cn[4]; snprintf(cn,sizeof(cn),"%d",i+1);
        _tft.setTextFont(1); _tft.setTextColor(C_DGRAY, C_BG);
        _tft.setTextDatum(MC_DATUM);
        _tft.drawString(cn, bx+bW/2, cvY+cvH-5);
      }
    }
  } else {
    _tft.setTextFont(2); _tft.setTextColor(C_DGRAY, C_BG);
    _tft.setTextDatum(MC_DATUM);
    _tft.drawString(bmsConnected?"Дані...":"BMS не підключено", SCRW/2, cvY+cvH/2);
  }
}

// ═══════════════════════════════════════════════════════
//  PAGE 1 — CELL VOLTAGES (детально, 4 колонки)
// ═══════════════════════════════════════════════════════
static void _pg1() {
  _tft.fillRect(0, CTX_Y, SCRW, CTX_H, C_BG);
  uint8_t n = bmsData.cellCount;

  if (!bmsConnected || n==0) {
    _tft.setTextFont(2); _tft.setTextColor(C_GRAY, C_BG);
    _tft.setTextDatum(MC_DATUM);
    _tft.drawString("BMS не підключено", SCRW/2, CTX_Y+CTX_H/2);
    return;
  }
  if (n>24) n=24;

  float mnV=bmsData.cellVoltages[0], mxV=mnV;
  for(uint8_t i=1;i<n;i++){
    if(bmsData.cellVoltages[i]<mnV) mnV=bmsData.cellVoltages[i];
    if(bmsData.cellVoltages[i]>mxV) mxV=bmsData.cellVoltages[i];
  }
  float rng=mxV-mnV; if(rng<0.001f) rng=0.001f;

  // Summary header
  char sbuf[36];
  snprintf(sbuf, sizeof(sbuf), "min%.3f  max%.3f  \xCE\x94%.0fmV", mnV, mxV, rng*1000);
  _tft.setTextFont(1); _tft.setTextColor(C_GRAY, C_BG);
  _tft.setTextDatum(MC_DATUM);
  _tft.drawString(sbuf, SCRW/2, CTX_Y+8);
  _tft.drawFastHLine(0, CTX_Y+16, SCRW, C_BRD);

  // Grid 4×rows
  const uint8_t COLS=4;
  uint8_t  rows = (n+COLS-1)/COLS;
  int16_t  cw   = SCRW/COLS;           // 60px
  int16_t  ch   = (CTX_H-18)/rows;     // висота клітинки

  for (uint8_t i=0;i<n;i++){
    float   v   = bmsData.cellVoltages[i];
    uint8_t col = i%COLS, row = i/COLS;
    int16_t cx  = col*cw, cy = CTX_Y+18+row*ch;

    bool hi  = (v==mxV), lo=(v==mnV);
    bool bal = (i<16)?((bmsData.balanceStatus   >> i)     &1)
                     :((bmsData.balanceStatusHigh>>(i-16)) &1);

    uint16_t bg  = bal?C_SUR:C_BG;
    uint16_t brd = hi?C_GREEN:lo?C_YELLOW:C_BRD;
    uint16_t vc  = hi?C_GREEN:lo?C_YELLOW:C_WHITE;
    if(bal){ brd=C_YELLOW; }

    _tft.fillRect(cx+1, cy+1, cw-2, ch-2, bg);
    _tft.drawRect(cx, cy, cw, ch, brd);

    // Номер (TL)
    char nb[5]; snprintf(nb,sizeof(nb),"C%d",i+1);
    _tft.setTextFont(1); _tft.setTextColor(C_GRAY, bg);
    _tft.setTextDatum(TL_DATUM);
    _tft.drawString(nb, cx+3, cy+3);

    // H/L/BAL маркер (TR)
    _tft.setTextDatum(TR_DATUM);
    if(bal){ _tft.setTextColor(C_YELLOW, bg); _tft.drawString("~",cx+cw-2,cy+2); }
    else if(hi){ _tft.setTextColor(C_GREEN, bg); _tft.drawString("H",cx+cw-2,cy+2); }
    else if(lo){ _tft.setTextColor(C_YELLOW,bg); _tft.drawString("L",cx+cw-2,cy+2); }

    // Напруга (центр)
    char vb[8]; snprintf(vb,sizeof(vb),"%.3f",v);
    _tft.setTextFont(2); _tft.setTextColor(vc, bg);
    _tft.setTextDatum(MC_DATUM);
    _tft.drawString(vb, cx+cw/2, cy+ch/2-2);

    // Mini bar (знизу)
    float pct=(v-mnV)/rng;
    int16_t bw2=cw-8, bby=cy+ch-7;
    _tft.fillRect(cx+4, bby, bw2, 4, C_SUR2);
    int16_t bfw=(int16_t)(bw2*_clamp(pct,0,1));
    if(bfw>0) _tft.fillRect(cx+4, bby, bfw, 4, brd);
  }
}

// ═══════════════════════════════════════════════════════
//  PAGE 2 — SYSTEM STATUS
// ═══════════════════════════════════════════════════════
static void _pg2() {
  _tft.fillRect(0, CTX_Y, SCRW, CTX_H, C_BG);

  int16_t cy = CTX_Y+2;
  const int16_t RH = 19;   // row height
  char lb[28], rb[28];

  // row helper (label, value, color)
  auto row = [&](const char* lbl, const char* val, uint16_t vc) {
    _tft.setTextFont(1);
    _tft.setTextColor(C_GRAY, C_BG); _tft.setTextDatum(ML_DATUM);
    _tft.drawString(lbl, 6, cy+RH/2);
    _tft.setTextColor(vc, C_BG);     _tft.setTextDatum(MR_DATUM);
    _tft.drawString(val, SCRW-6, cy+RH/2);
    cy += RH;
    _tft.drawFastHLine(4, cy-2, SCRW-8, C_SUR);
  };

  auto sect = [&](const char* title) {
    _tft.fillRect(0, cy, SCRW, 14, C_SUR);
    _tft.setTextFont(1); _tft.setTextColor(C_ACCENT, C_SUR);
    _tft.setTextDatum(ML_DATUM);
    _tft.drawString(title, 6, cy+7);
    cy += 14;
  };

  // ── WiFi ──────────────────────────────────────────
  sect(" WiFi");
  row("Статус", wifiConnected?"Connected":"Disconnected",
      wifiConnected?C_GREEN:C_RED);
  if (wifiConnected) {
    row("STA IP", WiFi.localIP().toString().c_str(), C_ACCENT);
    snprintf(lb, sizeof(lb), "%d dBm", WiFi.RSSI());
    row("RSSI", lb, WiFi.RSSI()>-65?C_GREEN2:C_YELLOW);
  }
  snprintf(lb, sizeof(lb), "%s", WiFi.softAPIP().toString().c_str());
  row("AP IP", lb, C_LGRAY);

  // ── BMS ───────────────────────────────────────────
  sect(" BMS (BLE)");
  row("Статус", bmsConnected?"Connected":"Disconnected",
      bmsConnected?C_GREEN:C_RED);
  if (bmsConnected) {
    // Use MAC address instead of name to avoid encoding issues (square boxes)
    // Truncate MAC for display
    String macShort = BMS_MAC.substring(0, 17);  // Show full MAC
    row("Device", macShort.c_str(), C_WHITE);
    row("MAC", BMS_MAC.c_str(), C_GRAY);
    snprintf(lb, sizeof(lb), "v0x%02X  cyc:%d",
             bmsData.softwareVersion, bmsData.cycleCount);
    row("Info", lb, C_LGRAY);
    row("Date", bmsData.productionDate.c_str(), C_LGRAY);
  }

  // ── Protection ────────────────────────────────────
  if (cy < SCRH-FTR_H-RH) {
    sect(" Захист");
    uint16_t ps = bmsData.protectionStatus;
    snprintf(lb, sizeof(lb), "0x%04X", ps);
    row(ps==0?"OK":"FAULT!", lb, ps==0?C_GREEN:C_RED);
    if (ps != 0) {
      static const char* fn[13] = {
        "CellOVP","CellUVP","PackOVP","PackUVP",
        "ChgOTP","ChgUTP","DsgOTP","DsgUTP",
        "ChgOCP","DsgOCP","SCP","AFE","Lock"
      };
      for (uint8_t i=0; i<13 && cy<SCRH-FTR_H-RH; i++) {
        if ((ps>>i)&1) {
          snprintf(lb, sizeof(lb), "  bit%d", i);
          row(lb, fn[i], C_RED);
        }
      }
    }
  }

  // ── Config ────────────────────────────────────────
  if (cy < SCRH-FTR_H-RH*2) {
    sect(" Конфіг");
    row("Device ID", deviceId.c_str(), C_ACCENT);
    String srv = apiServer;
    if(srv.length()>22) srv=srv.substring(0,19)+"...";
    if(srv.length()==0) srv="(не задано)";
    row("API", srv.c_str(), C_GRAY);
  }
}

// ─── Boot splash ──────────────────────────────────────
static void _splash() {
  _tft.fillScreen(C_BG);

  // Logo
  _tft.setTextFont(4); _tft.setTextColor(C_ACCENT, C_BG);
  _tft.setTextDatum(MC_DATUM);
  _tft.drawString("JBD BMS", SCRW/2, SCRH/2-52);

  _tft.setTextFont(2); _tft.setTextColor(C_GRAY, C_BG);
  _tft.drawString("Battery Management System", SCRW/2, SCRH/2-22);

  _tft.setTextFont(1); _tft.setTextColor(C_DGRAY, C_BG);
  _tft.drawString("ESP32-S3 Super Mini", SCRW/2, SCRH/2+4);
  _tft.drawString("GMT020-02  ST7789  240x320", SCRW/2, SCRH/2+18);

  // Progress bar
  int16_t bx=40, by=SCRH/2+46, bw=SCRW-80;
  _tft.drawRect(bx, by, bw, 8, C_BRD);
  for (uint8_t p=0; p<=100; p+=4) {
    _tft.fillRect(bx+1, by+1, (bw-2)*p/100, 6, C_ACCENT);
    delay(20); yield();
  }

  _tft.setTextFont(1); _tft.setTextColor(C_GREEN2, C_BG);
  _tft.drawString("Готово!", SCRW/2, SCRH/2+66);
  delay(500);
}

// ═══════════════════════════════════════════════════════
//  PUBLIC API
// ═══════════════════════════════════════════════════════

inline bool initDisplay() {
  Serial.println("[Display] initDisplay() LovyanGFX + ST7789 240×320");

  // LovyanGFX: init() застосовує конфіг з lgfx_config.h (SPI, Panel, Backlight)
  _tft.init();
  _tft.setRotation(0);    // Portrait, USB connector down
  _tft.fillScreen(C_BG);
  // LovyanGFX не потребує setSwapBytes() — байтовий порядок RGB565
  // налаштовується автоматично через Panel_ST7789 конфіг

  // Підсвітка — вбудований PWM LovyanGFX (Light_PWM з lgfx_config.h)
  // Замінює ручний analogWrite/ledcWrite — більше не потрібен!
  _tft.setBrightness(_brt);   // 0-255, LEDC PWM channel 7, GPIO11
  Serial.printf("[Display] Backlight GPIO11, setBrightness(%d)\n", _brt);

  displayDetected = true;
  displayActive   = true;

  _splash();
  _tft.fillScreen(C_BG);

  _pgTimer  = millis();
  _drawTimer = 0;
  _inited   = true;

  Serial.println("[Display] initDisplay() LovyanGFX OK");
  return true;
}

// Оновлення — викликати у loop()
inline void handleDisplay() {
  if (!_inited || !displayDetected || !displayActive) return;

  uint32_t now = millis();

  // Авто-перемикання
  if (now - _pgTimer >= _PG_MS) {
    _pg = (_pg+1) % _PG_CNT;
    _pgTimer = now;
    _drawTimer = 0;
    Serial.printf("[Display] Page %d → %s\n", _pg, _pgNames[_pg]);
  }

  // Throttle
  if (now - _drawTimer < _DRAW_MS) return;
  _drawTimer = now;

  // Render
  switch (_pg) {
    case 0: _hdr("JBD BMS Controller"); _pg0(); break;
    case 1: _hdr("Напруги клітин");      _pg1(); break;
    case 2: _hdr("Статус системи");      _pg2(); break;
  }
  _ftr(_pg);

  yield();
}

// Перемкнути сторінку вручну
inline void nextDisplayPage() {
  _pg = (_pg+1) % _PG_CNT;
  _pgTimer = millis();
  _drawTimer = 0;
}

// Яскравість підсвітки 0-255 — LovyanGFX Light_PWM (GPIO11, LEDC ch7)
inline void setDisplayBrightness(uint8_t brt) {
  _brt = brt;
  _tft.setBrightness(brt);   // вбудований PWM — не треба analogWrite!
}

// Увімкнути/вимкнути підсвітку (використовується з /display?cmd=toggle)
inline void setDisplayPower(bool on) {
  displayActive = on;
  _tft.setBrightness(on ? _brt : 0);   // LovyanGFX: 0 = підсвітка вимкнена
  if (!on) _tft.fillScreen(C_BG);
}

// Інтервал авто-ротації (мс)
inline void setPageInterval(uint32_t ms) { _PG_MS = ms; }

// ─── OTA Display Feedback ──────────────────────────
inline void otaUpdateStart() {
  if (!_inited || !displayActive) return;
  _tft.fillScreen(C_BG);
  _tft.setTextColor(C_RED, C_BG);
  _tft.setTextSize(2);
  _tft.setCursor(10, 10);
  _tft.println("OTA UPDATE");
  _tft.setTextColor(C_WHITE, C_BG);
  _tft.setTextSize(1);
  _tft.setCursor(10, 40);
  _tft.println("Downloading...");
  _tft.drawRect(10, 65, 220, 18, C_WHITE);
}

inline void otaUpdateProgress(int percent) {
  if (!_inited || !displayActive) return;
  int pw = (218 * percent) / 100;
  if (pw > 0) _tft.fillRect(11, 66, pw, 16, C_GREEN);
  _tft.setTextColor(C_WHITE, C_BG);
  _tft.setTextSize(1);
  _tft.setCursor(10, 90);
  _tft.printf("Progress: %d%%", percent);
}

inline void otaUpdateComplete() {
  if (!_inited || !displayActive) return;
  _tft.fillScreen(C_BG);
  _tft.setTextColor(C_GREEN, C_BG);
  _tft.setTextSize(2);
  _tft.setCursor(10, 40);
  _tft.println("SUCCESS!");
  _tft.setTextSize(1);
  _tft.setCursor(10, 70);
  _tft.println("Rebooting...");
  delay(1000);
}
