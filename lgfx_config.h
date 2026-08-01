// lgfx_config.h — LovyanGFX конфігурація дисплею
// Board:   ESP32-S3 Super Mini
// Display: GMT020-02 (ST7789V, 240×320, SPI 4-line)
// Library: LovyanGFX  https://github.com/lovyan03/LovyanGFX
//
// ПЕРЕВАГИ LovyanGFX над TFT_eSPI:
//  ✓ Конфіг прямо в коді — не треба лізти в папку бібліотеки
//  ✓ Вбудований PWM підсвітки (setBrightness 0-255)
//  ✓ Автоматичний DMA — швидше малювання без blocking
//  ✓ Sprite з alpha blending
//  ✓ API 100% сумісний з TFT_eSPI
//
// Цей файл включається тільки з display.h
// ─────────────────────────────────────────────────────────

#pragma once
#include <LovyanGFX.hpp>

// ═════════════════════════════════════════════════════════
//  LGFX — клас конфігурації (один екземпляр у display.h)
// ═════════════════════════════════════════════════════════
class LGFX : public lgfx::LGFX_Device {

  lgfx::Panel_ST7789  _panel;   // GMT020-02: COG ST7789V
  lgfx::Bus_SPI       _bus;     // SPI2 апаратний SPI (ESP32-S3)
  lgfx::Light_PWM     _light;   // PWM підсвітка через LEDC

public:
  LGFX() {
    Serial.println("[LGFX] Constructor - configuring SPI and panel");

    // ── SPI Bus ─────────────────────────────────────────
    {
      auto cfg = _bus.config();

      cfg.spi_host    = SPI2_HOST;       // SPI2 на ESP32-S3
      cfg.spi_mode    = 0;               // CPOL=0 CPHA=0 (ST7789)
      cfg.freq_write  = 40000000;        // 40MHz запис (GMT020-02 max ~62MHz)
      cfg.freq_read   = 16000000;        // 16MHz читання
      cfg.spi_3wire   = false;           // 4-wire: окремий DC пін
      cfg.use_lock    = true;            // потокобезпека
      cfg.dma_channel = SPI_DMA_CH_AUTO; // авто вибір DMA каналу

      // GPIO → ESP32S3SuperMini піни
      cfg.pin_sclk    = 13;   // SCLK
      cfg.pin_mosi    = 12;   // MOSI
      cfg.pin_miso    = -1;   // не використовується
      cfg.pin_dc      = 10;   // DC

      Serial.println("[LGFX] SPI pins: SCLK=13, MOSI=12, DC=10");
      _bus.config(cfg);
      _panel.setBus(&_bus);
    }

    // ── Panel ST7789V ────────────────────────────────────
    {
      auto cfg = _panel.config();

      cfg.pin_cs           =  9;   // CS
      cfg.pin_rst          = 11;   // RST
      cfg.pin_busy         = -1;   // немає

      Serial.println("[LGFX] Panel pins: CS=9, RST=11");

      cfg.memory_width     = 240;  // фізична ширина пам'яті
      cfg.memory_height    = 320;  // фізична висота пам'яті
      cfg.panel_width      = 240;  // активна область
      cfg.panel_height     = 320;

      cfg.offset_x         =   0;
      cfg.offset_y         =   0;
      cfg.offset_rotation  =   0;  // коригування при повороті

      cfg.dummy_read_pixel =   8;  // біт для читання пікселів
      cfg.dummy_read_bits  =   1;  // доп. такти перед читанням

      cfg.readable         = false;  // не читаємо з дисплею
      // ⚠ Деякі ST7789 модулі потребують invert = true
      // Якщо кольори інвертовані — змінити на true:
      cfg.invert           = true;
      cfg.rgb_order        = false;  // BGR (стандарт для ST7789)
      cfg.dlen_16bit       = false;  // 8-bit SPI transfer
      cfg.bus_shared       = false;  // SPI не ділиться з іншими

      _panel.config(cfg);
    }

    // ── Backlight PWM ────────────────────────────────────
    // Вбудована підтримка — setBrightness(0-255) замість analogWrite!
    {
      auto cfg = _light.config();

      cfg.pin_bl      = -1;     // GPIO11 → Pin1 LED+ (через резистор 33Ом)
      cfg.invert      = false;  // HIGH = яскраво (не інвертований)
      cfg.freq        = 12000;  // 12kHz PWM (вище слуху людини)
      cfg.pwm_channel = 0;      // LEDC канал (ESP32-S3)

      _light.config(cfg);
      _panel.setLight(&_light);
    }

    setPanel(&_panel);
  }
};
// ═════════════════════════════════════════════════════════
// Використання:
//   #include "lgfx_config.h"
//   static LGFX tft;
//   tft.init();
//   tft.setRotation(0);
//   tft.fillScreen(TFT_BLACK);
//   tft.setBrightness(220);   // ← вбудований PWM підсвітки!
// ═════════════════════════════════════════════════════════
