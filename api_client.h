// api_client.h - BMS Data Push to External Server
#pragma once

#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include "GLOBAL_STATE.h"

// Timing
const unsigned long API_UPDATE_INTERVAL = 30000; // 30 секунд - збільшено для стабільності
unsigned long lastAPISend = 0;

// Push BMS data to server
inline void pushBMSData(float voltage, float current, int soc) {
  if (!wifiConnected) {
    Serial.println("[API] WiFi не підключено, пропуск відправки");
    return;
  }
  
  HTTPClient http;
  
  // Встановлюємо таймаут
  http.setTimeout(10000);
  
  // Формуємо URL
  String url = apiServer;
  
  WiFiClientSecure client;
  client.setInsecure();
  if (http.begin(client, url)) {
    // Встановлюємо заголовки
    http.addHeader("Content-Type", "application/json");
    http.addHeader("User-Agent", "ESP32-BMS-Controller");
    
    // Формуємо JSON
    DynamicJsonDocument doc(256);
    doc["device_id"] = deviceId;
    doc["voltage"] = voltage;
    doc["current"] = current;
    doc["soc"] = soc;
    doc["timestamp"] = millis();
    
    // Додаємо додаткові дані BMS якщо доступні
    if (bmsConnected) {
      doc["cell_count"] = bmsData.cellCount;
      doc["cycle_count"] = bmsData.cycleCount;
      doc["fet_status"] = bmsData.fetStatus;
      
      // Додаємо температури
      JsonArray temps = doc.createNestedArray("temperatures");
      for (int i = 0; i < bmsData.tempSensorCount && i < 6; i++) {
        temps.add(bmsData.temperatures[i]);
      }
    }
    
    String jsonString;
    serializeJson(doc, jsonString);
    
    Serial.printf("[API] Відправка даних: %s\n", jsonString.c_str());
    
    // Відправляємо POST запит
    int httpCode = http.POST(jsonString);
    
    if (httpCode > 0) {
      if (httpCode == HTTP_CODE_OK) {
        Serial.printf("[API] Дані відправлено успішно, код: %d\n", httpCode);
      } else {
        Serial.printf("[API] Помилка сервера, код: %d\n", httpCode);
        String response = http.getString();
        Serial.printf("[API] Відповідь: %s\n", response.c_str());
      }
    } else {
      Serial.printf("[API] Помилка відправки: %s\n", http.errorToString(httpCode).c_str());
    }
    
    http.end();
  } else {
    Serial.println("[API] Не вдалося ініціалізувати HTTPS-з'єднання");
  }
}

// Автоматична відправка даних
inline void handleAPIData() {
  if (millis() - lastAPISend > API_UPDATE_INTERVAL) {
    if (bmsConnected) {
      Serial.println("[API] Відправка даних BMS на сервер...");
      pushBMSData(bmsData.totalVoltage, bmsData.current, bmsData.soc);
    } else {
      Serial.println("[API] BMS не підключено, пропуск відправки");
    }
    lastAPISend = millis();
  }
}

// Тестове підключення до сервера
inline void testAPIServer() {
  HTTPClient http;
  WiFiClientSecure client;
  client.setInsecure();
  if (http.begin(client, apiServer)) {
    int httpCode = http.GET();
    if (httpCode > 0) {
      Serial.printf("[API] Сервер доступний, код: %d\n", httpCode);
    } else {
      Serial.printf("[API] Сервер недоступний: %s\n", http.errorToString(httpCode).c_str());
    }
    http.end();
  } else {
    Serial.println("[API] Помилка підключення до сервера");
  }
}
