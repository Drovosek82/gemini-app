// main.cpp - BMS Controller Local Monitor
// ESP32-S3 Super Mini with JBD BMS, WiFi Manager, Web Interface

#include <Arduino.h>
#include "GLOBAL_STATE.h"
#include "wifi_manager.h"
#include "bms.h"
#include "display.h"
#include "web_server.h"
#include "api_client.h"

void setup() {
  // Initialize Serial
  Serial.begin(115200);
  Serial.println("[Main] =========================================");
  Serial.println("[Main] ESP32-C3 BMS Controller v2.0");
  Serial.println("[Main] =========================================");

  // Load saved preferences at startup
  loadBMSConfig();
  loadWiFiConfig();

  // Initialize Mutex for BMS data protection
  bmsDataMutex = xSemaphoreCreateMutex();
  if (bmsDataMutex == nullptr) {
    Serial.println("[Main] Помилка створення мутексу!");
  }

  Serial.println("[Main] Крок 1: Ініціалізація BLE...");
  // Initialize BMS BLE FIRST - must be before WiFi for proper radio initialization on ESP32-S3
  initBMS();
  Serial.println("[Main] BLE ініціалізовано");
  Serial.println("[Main] Крок 2: Ініціалізація WiFi Manager...");

  // Initialize WiFi Manager (after BLE for radio coexistence)
  initWiFiManager();

  // Initialize PSRAM for external RAM access (2MB on Super Mini)
  psramInit();
  Serial.println("[Main] PSRAM ініціалізовано");

  Serial.println("[Main] Крок 3: Ініціалізація дисплея...");

  // Initialize display
  initDisplay();
  Serial.println("[Main] Крок 4: Ініціалізація веб-сервера...");

  // Initialize Web Server
  initWebServer();
  Serial.println("[Main] Крок 5: Завершення налаштувань...");

  Serial.println("[Main] Ініціалізація завершена");

  // Try to auto-connect to last BMS if configured
  if (BMS_MAC.length() > 0 && !bmsConnected) {
    Serial.println("[Main] Спроба автоматичного підключення до BMS...");
    if (connectToBMS(BMS_MAC, BMS_NAME)) {
      Serial.println("[Main] Автоматично підключено до BMS");
    }
  }
}

void loop() {
  // Handle WiFi Manager (AP/STA mode switching)
  handleWiFiManager();

  // Soft WiFi reconnect check (non-blocking)
  if (WiFi.status() != WL_CONNECTED) {
    static unsigned long lastWiFiReconnect = 0;
    if (millis() - lastWiFiReconnect > 15000) {
      lastWiFiReconnect = millis();
      Serial.println("[Main] WiFi не підключено, неблокуюча спроба reconnect...");
      if (wifiSSID.length() > 0) {
        WiFi.disconnect();
        WiFi.begin(wifiSSID.c_str(), wifiPassword.c_str());
      } else {
        Serial.println("[Main] Немає збереженої мережі Wi-Fi");
      }
    }
  }

  // Handle Web Server clients
  server.handleClient();

  // Update BMS data every 5 seconds (reduced frequency)
  if (millis() - lastBMSUpdate > BMS_UPDATE_INTERVAL) {
    if (bmsConnected) {
      updateBMSData();
    }
    lastBMSUpdate = millis();
  }

  // Update display every 1 second
  handleDisplay();

  // Handle API data sending
  handleAPIData();

  // Auto-reconnect to BMS if configured and disconnected
  static unsigned long lastBMSReconnect = 0;
  if (BMS_MAC.length() > 0 && !bmsConnected &&
      millis() - lastBMSReconnect > 25000) {
    Serial.println("[Main] Спроба автоматичного перепідключення до BMS...");
    if (connectToBMS(BMS_MAC, BMS_NAME)) {
      Serial.println("[Main] Автоматично перепідключено до BMS");
    }
    lastBMSReconnect = millis();
  }

  // Handle restart request
  if (shouldRestart) {
    Serial.println("[Main] Перезапуск за запитом...");
    delay(1000);
    ESP.restart();
  }

  // Yield to allow background WiFi/BLE processing
  yield();

  // Non-blocking delay for CPU load reduction
  static unsigned long lastLoopDelay = 0;
  if (millis() - lastLoopDelay > 50) {
    lastLoopDelay = millis();
  }

  delay(5);
}
