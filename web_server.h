// web_server.h — Web Server Module (Header-only)
// ESP32-S3 Super Mini + JBD BMS
// ─────────────────────────────────────────────────────────────
#pragma once

#include <WebServer.h>
#include <Update.h>
#include "GLOBAL_STATE.h"
#include "display.h"
#include "html_page.h"

// Forward declarations
inline void handleWiFiScan();

// WebServer instance — port 80
inline WebServer server(80);

// ─── handleRoot — serve actual web interface from PROGMEM ───
inline void handleRoot() {
  Serial.println("[Web] GET /");
  server.send_P(200, "text/html", HTML_PAGE);
  Serial.println("[Web] Sent main HTML page");
}

// ─── sanitizeJsonString — remove control chars (< 32) that break JSON ───
inline String sanitizeJsonString(const String& input) {
  String output;
  output.reserve(input.length());
  for (size_t i = 0; i < input.length(); i++) {
    char c = input.charAt(i);
    if (c < 0x20 && c != '\t' && c != '\n' && c != '\r') {
      continue;
    }
    output += c;
  }
  return output;
}

// ─── handleData — GET /data ───────────────────────────────
inline void handleData() {
  Serial.println("[Web] GET /data");
  Serial.println("[Web] 1 - Headers");
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
  server.sendHeader("Access-Control-Allow-Headers", "Content-Type");

  Serial.println("[Web] 2 - StaticJsonDocument");
  static StaticJsonDocument<4096> doc;
  doc.clear();

  Serial.println("[Web] 3 - Connection status");
  // Lock mutex to protect BMS data during read
  bool mutexLocked = false;
  if (bmsDataMutex != nullptr && xSemaphoreTake(bmsDataMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
    mutexLocked = true;

    // Connection status
    doc["wifiConnected"]  = wifiConnected;
    doc["wifiInfo"]       = wifiConnected ? localIP.toString() : "";
    doc["bmsConnected"]   = bmsConnected;
    doc["bmsInfo"]        = bmsConnected ? sanitizeJsonString(BMS_NAME) : "";
    doc["displayActive"]  = displayActive;
    doc["displayDetected"]= displayDetected;

    if (!bmsConnected) {
      bmsData.totalVoltage = 0.0;
      bmsData.current = 0.0;
      bmsData.soc = 0;
      bmsData.cellCount = 0;
      bmsData.tempSensorCount = 0;
      bmsData.protectionStatus = 0;
      bmsData.balanceStatus = 0;
      bmsData.balanceStatusHigh = 0;
      bmsData.fetStatus = 0;
      bmsData.capacityRemaining = 0.0;
      bmsData.capacityTotal = 0.0;
      bmsData.cycleCount = 0;
      bmsData.softwareVersion = 0;
      memset(bmsData.cellVoltages, 0, sizeof(bmsData.cellVoltages));
      memset(bmsData.temperatures, 0, sizeof(bmsData.temperatures));
      bmsData.productionDate = "";
      bmsData.hardwareVersion = "";
    }

    Serial.println("[Web] 4 - BMS scalar data");
    // BMS scalar data
    doc["voltage"]        = bmsData.totalVoltage;
    doc["current"]        = bmsData.current;
    doc["soc"]            = bmsData.soc;
    doc["capacity"]       = bmsData.capacityRemaining;
    doc["totalCapacity"]  = bmsData.capacityTotal;
    doc["cellCount"]      = bmsData.cellCount;
    doc["fetStatus"]      = bmsData.fetStatus;
    doc["cycleCount"]     = bmsData.cycleCount;
    doc["tempSensorCount"]= bmsData.tempSensorCount;
    doc["balanceStatus"]  = bmsData.balanceStatus;
    doc["balanceStatusHigh"] = bmsData.balanceStatusHigh;
    doc["protectionStatus"]  = bmsData.protectionStatus;
    doc["protectionDecoded"] = sanitizeJsonString(decodeProtectionState(bmsData.protectionStatus));
    if (!bmsConnected) {
      doc["protectionDecoded"] = "BMS DISCONNECTED";
    }
    doc["softwareVersion"]   = sanitizeJsonString(String(bmsData.softwareVersion));
    doc["productionDate"]    = sanitizeJsonString(bmsData.productionDate);

    Serial.println("[Web] 6 - Temperatures array");
    // Temperatures array
    if (bmsConnected && bmsData.tempSensorCount > 0) {
      JsonArray temps = doc.createNestedArray("temperatures");
      for (uint8_t i = 0; i < bmsData.tempSensorCount && i < 6; i++) {
        temps.add(bmsData.temperatures[i]);
      }
    }

    Serial.println("[Web] 7 - Cell voltages array");
    // Cell voltages array
    if (bmsConnected && bmsData.cellCount > 0) {
      JsonArray cells = doc.createNestedArray("cellVoltages");
      for (uint8_t i = 0; i < bmsData.cellCount && i < 24; i++) {
        cells.add(bmsData.cellVoltages[i]);
      }
    }

    // Unlock mutex
    xSemaphoreGive(bmsDataMutex);
  } else {
    Serial.println("[Web] Помилка захоплення мутексу - повертаємо порожні дані");
    doc["wifiConnected"]  = wifiConnected;
    doc["wifiInfo"]       = wifiConnected ? localIP.toString() : "";
    doc["bmsConnected"]   = false;
    doc["bmsInfo"]        = "";
    doc["displayActive"]  = displayActive;
    doc["displayDetected"]= displayDetected;
  }

  Serial.println("[Web] 5 - ESP32-S3 System info");
  // Uptime as formatted string to avoid extra 'с' in frontend
  unsigned long totalSeconds = millis() / 1000;
  unsigned long hours = totalSeconds / 3600;
  unsigned long minutes = (totalSeconds % 3600) / 60;
  unsigned long seconds = totalSeconds % 60;
  char uptime_str[32];
  sprintf(uptime_str, "%lu год, %lu хв, %lu сек", hours, minutes, seconds);
  doc["espUptime"] = uptime_str;

  // Heap memory
  doc["espHeap"]        = ESP.getFreeHeap();           // free heap in bytes
  doc["espTotalHeap"]   = ESP.getHeapSize();           // total heap in bytes

  // PSRAM memory (2MB on our board) — guarded against uninitialized PSRAM
  if (ESP.getPsramSize() > 0) {
    doc["espTotalPsram"]  = ESP.getPsramSize();
    doc["espFreePsram"]   = ESP.getFreePsram();
  } else {
    doc["espTotalPsram"]  = 0;
    doc["espFreePsram"]   = 0;
  }

  // CPU frequency
  doc["espCpuFreq"]     = ESP.getCpuFreqMHz();

  // CPU temperature (ESP32-S3 core)
  doc["espTemp"]        = temperatureRead();

  // Flash size
  doc["flashSize"]      = ESP.getFlashChipSize();

  // Only add WiFi RSSI if connected (avoid blocking)
  if (wifiConnected) {
    doc["wifiRssi"]     = WiFi.RSSI();
  }
  doc["apClients"]      = WiFi.softAPgetStationNum();

  Serial.println("[Web] 8 - serializeJson");
  String response;
  serializeJson(doc, response);

  Serial.println("[Web] RAW JSON:");
  Serial.println(response);

  Serial.println("[Web] 9 - Serial.printf");
  Serial.printf("[Web] /data V=%.2f A=%.2f SOC=%d%% cells=%d prot=0x%04X\n",
    bmsData.totalVoltage, bmsData.current, bmsData.soc,
    bmsData.cellCount, bmsData.protectionStatus);

  Serial.println("[Web] 10 - server.send");
  server.send(200, "application/json", response);
  Serial.println("[Web] 11 - Done");
}

// ─── handleControl — GET /control?cmd=mos&state=0-3 ──────
inline void handleControl() {
  String cmd   = server.arg("cmd");
  server.sendHeader("Access-Control-Allow-Origin", "*");

  if (cmd == "mos") {
    int state = server.arg("state").toInt();
    if (state >= 0 && state <= 3) {
      controlMOSFET((uint8_t)state);
      String r = "{\"status\":\"ok\",\"state\":" + String(state) + "}";
      server.send(200, "application/json", r);
      Serial.printf("[Web] MOSFET state=%d\n", state);
    } else {
      server.send(400, "application/json",
        "{\"status\":\"error\",\"message\":\"state must be 0-3\"}");
    }
  } else {
    server.send(400, "application/json",
      "{\"status\":\"error\",\"message\":\"unknown cmd\"}");
  }
}

// ─── handleScan — GET /scan → {devices:[{mac,name}]} ────
inline void handleScan() {
  Serial.println("[Web] GET /scan — BLE scan start");
  server.sendHeader("Access-Control-Allow-Origin", "*");

  startBLEScan();

  StaticJsonDocument<1024> doc;
  JsonArray devices = doc.createNestedArray("devices");

  for (int i = 0; i < bmsCount; i++) {
    int pipe = availableBMS[i].indexOf('|');
    JsonObject d = devices.createNestedObject();
    d["mac"]  = availableBMS[i].substring(0, pipe);
    d["name"] = availableBMS[i].substring(pipe + 1);
    yield();
  }

  String response;
  serializeJson(doc, response);
  server.send(200, "application/json", response);
  Serial.printf("[Web] /scan returned %d devices\n", bmsCount);
}

// ─── handleWiFiScan — GET /wifiscan → {networks:[{ssid,rssi,open}]}
inline void handleWiFiScan() {
  Serial.println("[Web] GET /wifiscan — WiFi scan start");
  server.sendHeader("Access-Control-Allow-Origin", "*");

  int n = WiFi.scanNetworks();
  StaticJsonDocument<1024> doc;
  JsonArray networks = doc.createNestedArray("networks");

  for (int i = 0; i < n && i < 20; i++) {
    JsonObject net = networks.createNestedObject();
    net["ssid"] = WiFi.SSID(i);
    net["rssi"] = WiFi.RSSI(i);
    net["open"] = (WiFi.encryptionType(i) == WIFI_AUTH_OPEN);
    yield();
  }
  WiFi.scanDelete();

  String response;
  serializeJson(doc, response);
  server.send(200, "application/json", response);
  Serial.printf("[Web] /wifiscan returned %d networks\n", n);
}

// ─── handleConnect — POST /connect {mac,name} ─────────────
inline void handleConnect() {
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Access-Control-Allow-Methods", "POST, OPTIONS");
  server.sendHeader("Access-Control-Allow-Headers", "Content-Type");

  if (server.method() == HTTP_OPTIONS) {
    server.send(204);
    return;
  }

  StaticJsonDocument<128> doc;
  deserializeJson(doc, server.arg("plain"));
  String mac = doc["mac"];
  String name = doc["name"];

  Serial.printf("[Web] POST /connect mac=%s name=%s\n", mac.c_str(), name.c_str());

  if (connectToBMS(mac, name)) {
    // Save BMS MAC to Flash after successful connection
    saveBMSConfig(mac);
    server.send(200, "application/json", "{\"status\":\"connected\"}");
  } else {
    server.send(400, "application/json", "{\"status\":\"failed\"}");
  }
}

// ─── handleDisconnect — GET /disconnect ─────────────────
inline void handleDisconnect() {
  server.sendHeader("Access-Control-Allow-Origin", "*");
  disconnectBMS();
  server.send(200, "application/json", "{\"status\":\"disconnected\"}");
  Serial.println("[Web] GET /disconnect");
}

// ─── handleResetBMS — GET /reset-bms ───────────────────────
inline void handleResetBMS() {
  server.sendHeader("Access-Control-Allow-Origin", "*");
  resetBMSConfig();
  server.send(200, "application/json", "{\"status\":\"ok\",\"message\":\"BMS config reset\"}");
  Serial.println("[Web] GET /reset-bms - BMS configuration cleared");
}

// ─── handleDisplay — GET /display?cmd=toggle ─────────────
inline void handleDisplayWeb() {
  server.sendHeader("Access-Control-Allow-Origin", "*");
  String cmd = server.arg("cmd");

  if (!displayDetected) {
    server.send(200, "application/json",
      "{\"status\":\"ok\",\"displayActive\":false,\"displayDetected\":false}");
    return;
  }

  if (cmd == "toggle") {
    setDisplayPower(!displayActive);
  }

  String r = "{\"status\":\"ok\",\"displayActive\":" +
             String(displayActive ? "true" : "false") + "}";
  server.send(200, "application/json", r);
  Serial.printf("[Web] /display?cmd=%s → %s\n", cmd.c_str(),
                displayActive ? "ON" : "OFF");
}

// ─── handleWiFiConnect — POST /wificonnect ───────────────
inline void handleWiFiConnect() {
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Access-Control-Allow-Methods", "POST, OPTIONS");
  server.sendHeader("Access-Control-Allow-Headers", "Content-Type");

  if (server.method() == HTTP_OPTIONS) {
    server.send(204);
    return;
  }

  StaticJsonDocument<256> doc;
  deserializeJson(doc, server.arg("plain"));
  String ssid = doc["ssid"];
  String password = doc["password"];

  Serial.printf("[Web] POST /wificonnect ssid=%s password_len=%d\n",
                ssid.c_str(), password.length());

  bool ok = saveWiFiConfig(ssid, password);
  if (ok) {
    Serial.println("[Web] WiFi конфігурація збережена, спроба підключення...");
    shouldRestart = true;
    server.send(200, "application/json", "{\"status\":\"ok\",\"message\":\"WiFi saved, restarting...\",\"connected\":false}");
  } else {
    Serial.println("[Web] Помилка збереження WiFi конфігурації");
    server.send(500, "application/json", "{\"status\":\"error\",\"message\":\"Failed to save WiFi config\"}");
  }
}

// ─── handleSaveWiFi — POST /save_wifi (save without restart) ─────
inline void handleSaveWiFi() {
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Access-Control-Allow-Methods", "POST, OPTIONS");
  server.sendHeader("Access-Control-Allow-Headers", "Content-Type");

  if (server.method() == HTTP_OPTIONS) {
    server.send(204);
    return;
  }

  StaticJsonDocument<256> doc;
  deserializeJson(doc, server.arg("plain"));
  String ssid = doc["ssid"];
  String password = doc["password"];

  Serial.printf("[Web] POST /save_wifi ssid=%s password_len=%d\n",
                ssid.c_str(), password.length());

  bool ok = saveWiFiConfig(ssid, password);
  if (ok) {
    Serial.println("[Web] WiFi конфігурація збережена успішно");
    server.send(200, "application/json", "{\"status\":\"ok\",\"message\":\"WiFi saved successfully\"}");
  } else {
    Serial.println("[Web] Помилка збереження WiFi конфігурації");
    server.send(500, "application/json", "{\"status\":\"error\",\"message\":\"Failed to save WiFi config\"}");
  }
}

// ─── handleBalance — GET /balance?state=0-1 ─────────────
inline void handleBalance() {
  server.sendHeader("Access-Control-Allow-Origin", "*");
  int state = server.arg("state").toInt();
  Serial.printf("[Web] GET /balance?state=%d\n", state ? 1 : 0);
  server.send(200, "text/plain", state ? "Балансування ввімкнено" : "Балансування вимкнено");
}

// ─── handleAutoPoll — GET/POST /autopoll ─────────────────
inline void handleAutoPoll() {
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
  server.sendHeader("Access-Control-Allow-Headers", "Content-Type");

  if (server.method() == HTTP_OPTIONS) {
    server.send(204);
    return;
  }

  if (server.method() == HTTP_POST) {
    StaticJsonDocument<64> doc;
    deserializeJson(doc, server.arg("plain"));
    bool enabled = doc["enabled"] | true;
    Serial.printf("[Web] POST /autopoll enabled=%d\n", enabled ? 1 : 0);
    server.send(200, "application/json", "{\"status\":\"ok\",\"autoPolling\":" + String(enabled ? "true" : "false") + "}");
  } else {
    server.send(200, "application/json", "{\"status\":\"ok\",\"autoPolling\":true}");
  }
}

// ─── handleCloudSettings — GET /cloud-settings ───────────
inline void handleCloudSettings() {
  server.sendHeader("Access-Control-Allow-Origin", "*");
  StaticJsonDocument<256> doc;
  doc["device_id"] = deviceId;
  doc["enabled"] = false;
  doc["server"] = apiServer;
  String response;
  serializeJson(doc, response);
  server.send(200, "application/json", response);
}

// ─── handleCloudSave — GET /cloud-save?device_id=...&enabled=...&server=...
inline void handleCloudSave() {
  server.sendHeader("Access-Control-Allow-Origin", "*");
  String devId = server.arg("device_id");
  bool enabled = server.arg("enabled") == "true";
  String serverUrl = server.arg("server");
  Serial.printf("[Web] GET /cloud-save device_id=%s enabled=%d server=%s\n", devId.c_str(), enabled ? 1 : 0, serverUrl.c_str());
  deviceId = devId;
  if (serverUrl.length() > 0) {
    apiServer = serverUrl;
  }
  saveAppConfig(apiServer, deviceId);
  server.send(200, "application/json", "{\"status\":\"ok\"}");
}

// ─── handleSystem — GET /system ─────────────────────────
inline void handleSystem() {
  server.sendHeader("Access-Control-Allow-Origin", "*");

  StaticJsonDocument<512> doc;

  // Причина рестарту
  esp_reset_reason_t reason = esp_reset_reason();
  const char* reasonStr = "Unknown";
  switch (reason) {
    case ESP_RST_POWERON: reasonStr = "Power On / Reset"; break;
    case ESP_RST_EXT:     reasonStr = "External Reset"; break;
    case ESP_RST_SW:      reasonStr = "Software Reset"; break;
    case ESP_RST_PANIC:   reasonStr = "Panic Exception"; break;
    case ESP_RST_INT_WDT: reasonStr = "Internal Watchdog"; break;
    case ESP_RST_TASK_WDT:reasonStr = "Task Watchdog"; break;
    case ESP_RST_DEEPSLEEP:reasonStr = "Deep Sleep Wake"; break;
    case ESP_RST_BROWNOUT:reasonStr = "Brownout"; break;
    default:              reasonStr = "Unknown"; break;
  }
  doc["reset_reason"] = reasonStr;

  // Chip ID
  uint64_t chipmac = ESP.getEfuseMac();
  char chipIdStr[20];
  snprintf(chipIdStr, sizeof(chipIdStr), "%04X%08X", (uint16_t)(chipmac>>32), (uint32_t)chipmac);
  doc["chip_id"] = chipIdStr;
  doc["mac_address"] = WiFi.macAddress();
  doc["chip_revision"] = ESP.getChipRevision();

  // Flash
  doc["flash_size_mb"] = ESP.getFlashChipSize() / 1024 / 1024;
  doc["flash_speed_mhz"] = ESP.getFlashChipSpeed() / 1000000;
  doc["free_sketch_space"] = ESP.getFreeSketchSpace();

  // PSRAM
  doc["psram_size"] = ESP.getPsramSize();
  doc["psram_free"] = ESP.getFreePsram();

  // Heap
  doc["heap_free"] = ESP.getFreeHeap();
  doc["heap_total"] = ESP.getHeapSize();
  doc["cpu_freq_mhz"] = ESP.getCpuFreqMHz();

  String response;
  serializeJson(doc, response);
  server.send(200, "application/json", response);
  Serial.println("[Web] /system - System audit sent");
}

// ─── handleSave — POST/GET /save ─────────────────────────
inline void handleSave() {
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "text/plain", "Not implemented");
}

// ─── handleOptions — CORS preflight ───────────────────────
inline void handleOptions() {
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
  server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
  server.send(204);
}

// ─── handleNotFound ───────────────────────────────────────
inline void handleNotFound() {
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(404, "text/plain", "Not Found");
  Serial.printf("[Web] 404: %s\n", server.uri().c_str());
}

// ─── handleUpdate — POST /update (OTA firmware upload) ───
inline void handleUpdate() {
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Access-Control-Allow-Methods", "POST, OPTIONS");
  server.sendHeader("Access-Control-Allow-Headers", "Content-Type");

  if (server.method() == HTTP_OPTIONS) {
    server.send(204);
    return;
  }

  HTTPUpload& upload = server.upload();
  static size_t full_file_size = 0;

  switch (upload.status) {
    case UPLOAD_FILE_START:
      full_file_size = server.header("Content-Length").toInt();
      Serial.printf("[OTA] Upload start: %s (%u bytes)\n", upload.filename.c_str(), full_file_size);
      otaUpdateStart();
      if (!Update.begin(UPDATE_SIZE_UNKNOWN)) {
        Update.end();
        server.send(500, "application/json", "{\"status\":\"error\",\"message\":\"Not enough space for firmware\"}");
        return;
      }
      break;
    case UPLOAD_FILE_WRITE:
      if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
        Update.end();
        server.send(500, "application/json", "{\"status\":\"error\",\"message\":\"Firmware write error\"}");
        return;
      }
      if (full_file_size > 0) {
        int percent = (upload.totalSize * 100) / full_file_size;
        otaUpdateProgress(percent);
      }
      break;
    case UPLOAD_FILE_END:
      if (Update.end(true)) {
        Serial.printf("[OTA] Firmware updated successfully. Size: %u bytes\n", upload.totalSize);
        otaUpdateComplete();
        server.send(200, "application/json", "{\"status\":\"ok\",\"message\":\"Firmware updated. Restarting...\"}");
        delay(1000);
        ESP.restart();
      } else {
        Update.end();
        server.send(500, "application/json", "{\"status\":\"error\",\"message\":\"Firmware update failed\"}");
      }
      break;
  }
}

// ─── initWebServer ────────────────────────────────────────
inline void initWebServer() {
  Serial.println("[Web] initWebServer()");

  server.on("/",           HTTP_GET,  handleRoot);
  server.on("/data",       HTTP_GET,  handleData);
  server.on("/control",    HTTP_GET,  handleControl);
  server.on("/scan",       HTTP_GET,  handleScan);
  server.on("/wifiscan",   HTTP_GET,  handleWiFiScan);
  server.on("/wificonnect", HTTP_POST, handleWiFiConnect);
  server.on("/save_wifi",  HTTP_POST, handleSaveWiFi);
  server.on("/connect",    HTTP_POST, handleConnect);
  server.on("/disconnect", HTTP_GET,  handleDisconnect);
  server.on("/reset-bms",  HTTP_GET,  handleResetBMS);
  server.on("/display",    HTTP_GET,  handleDisplayWeb);
  server.on("/balance",    HTTP_GET,  handleBalance);
  server.on("/autopoll",   HTTP_ANY,  handleAutoPoll);
  server.on("/cloud-settings", HTTP_GET, handleCloudSettings);
  server.on("/cloud-save", HTTP_GET,  handleCloudSave);
  server.on("/system",     HTTP_GET,  handleSystem);
  server.on("/save",       HTTP_ANY,  handleSave);
  server.on("/update",     HTTP_POST, handleUpdate);
  server.onNotFound(handleNotFound);

  const char * headerkeys[] = {"Content-Length"};
  server.collectHeaders(headerkeys, 1);

  server.begin();
  Serial.println("[Web] Server started — AP: http://192.168.4.1  STA: http://" + localIP.toString());
}
