// wifi_manager.h - Smart WiFi Manager Module (Header-only, Local Version)
#pragma once

#include "GLOBAL_STATE.h"
#include <Preferences.h>

// Preferences namespace
inline Preferences preferences;

// BMS Config Functions (Preferences)
inline bool saveBMSConfig(const String& mac) {
  Serial.printf("[Config] Збереження BMS MAC: %s\n", mac.c_str());

  preferences.begin("bms_app", false);

  bool success = preferences.putString("bms_mac", mac);
  bool successHasBMS = preferences.putBool("has_bms", true);
  success &= successHasBMS;

  preferences.end();

  delay(100); // Ensure NVS write completes

  if (success) {
    BMS_MAC = mac;
    Serial.println("[Config] MAC-адресу BMS успішно збережено в Flash!");
    return true;
  } else {
    Serial.println("[Config] Помилка збереження MAC-адреси BMS");
    return false;
  }
}

inline void loadBMSConfig() {
  Serial.println("[Config] Завантаження BMS конфігурації з NVS...");

  preferences.begin("bms_app", true);

  String mac = preferences.getString("bms_mac", "");
  bool has_bms = preferences.getBool("has_bms", false);

  preferences.end();

  if (has_bms && mac.length() > 0) {
    BMS_MAC = mac;
    Serial.printf("[Config] Знайдено збережену BMS: %s\n", BMS_MAC.c_str());
  } else {
    Serial.println("[Config] Збережена BMS не знайдена");
  }
}

inline void resetBMSConfig() {
  preferences.begin("bms_app", false);
  preferences.remove("bms_mac");
  preferences.remove("has_bms");
  preferences.end();

  BMS_MAC = "";
  Serial.println("[Config] BMS конфігурація скинута");
}

// Load WiFi configuration from preferences
inline void loadWiFiConfig() {
  Serial.println("[WiFi] Спроба завантаження конфігурації з NVS...");

  bool opened = preferences.begin("bms_app", true);

  if (!opened) {
    Serial.println("[WiFi] Помилка відкриття Preferences для завантаження");
    wifiSSID = "";
    wifiPassword = "";
    wifiConfigured = false;
    return;
  }

  wifiSSID = preferences.getString("ssid", "");
  wifiPassword = preferences.getString("pass", "");
  wifiConfigured = wifiSSID.length() > 0;

  preferences.end();

  Serial.println("[WiFi] Завантаження конфігурації:");
  Serial.printf("[WiFi] SSID: '%s' (довжина: %d)\n", wifiSSID.c_str(), wifiSSID.length());
  Serial.printf("[WiFi] Password довжина: %d\n", wifiPassword.length());
  Serial.printf("[WiFi] Налаштовано: %s\n", wifiConfigured ? "Так" : "Ні");
}

// Save WiFi configuration to preferences
inline bool saveWiFiConfig(const String& ssid, const String& password) {
  Serial.printf("[WiFi] Спроба збереження SSID: %s, довжина пароля: %d\n",
                ssid.c_str(), password.length());

  preferences.begin("bms_app", false);

  bool success = preferences.putString("ssid", ssid);
  bool successPass = preferences.putString("pass", password);
  success &= successPass;

  preferences.end();

  // Add delay to ensure NVS write completes
  delay(100);

  Serial.printf("[WiFi] putString SSID: %s, putString pass: %s\n",
                success ? "OK" : "FAIL", successPass ? "OK" : "FAIL");

  if (success) {
    wifiSSID = ssid;
    wifiPassword = password;
    wifiConfigured = true;
    Serial.println("[WiFi] Конфігурація збережена успішно");
    return true;
  } else {
    Serial.println("[WiFi] Помилка збереження конфігурації");
    return false;
  }
}

// Load app configuration from preferences
inline void loadAppConfig() {
  preferences.begin("app-config", false);
  apiServer = preferences.getString("api_server", apiServer);
  deviceId = preferences.getString("device_id", deviceId);
  preferences.end();
  Serial.println("[WiFi] Завантажено налаштування API");
  Serial.printf("[WiFi] API Server: %s\n", apiServer.c_str());
  Serial.printf("[WiFi] Device ID: %s\n", deviceId.c_str());
}

// Save app configuration to preferences
inline bool saveAppConfig(const String& server, const String& id) {
  preferences.begin("app-config", false);
  bool success = preferences.putString("api_server", server);
  success &= preferences.putString("device_id", id);
  preferences.end();

  if (success) {
    apiServer = server;
    deviceId = id;
    Serial.println("[WiFi] Налаштування API збережено");
    Serial.printf("[WiFi] API Server: %s\n", apiServer.c_str());
    Serial.printf("[WiFi] Device ID: %s\n", deviceId.c_str());
    return true;
  } else {
    Serial.println("[WiFi] Помилка збереження налаштувань API");
    return false;
  }
}

// Reset WiFi configuration
inline void resetWiFiConfig() {
  preferences.begin("wifi-config", false);
  preferences.clear();
  preferences.end();
  
  wifiSSID = "";
  wifiPassword = "";
  wifiConfigured = false;
  wifiConnected = false;
  
  Serial.println("[WiFi] Конфігурація скинута");
}

// Start STA connection to saved WiFi (non-blocking)
inline bool startSTAConnection() {
  if (!wifiConfigured || wifiSSID.length() == 0) {
    Serial.println("[WiFi] Немає збереженої конфігурації WiFi");
    return false;
  }

  Serial.printf("[WiFi] Спроба підключення до %s (пароль: %s)...\n",
                wifiSSID.c_str(), wifiPassword.length() > 0 ? "***" : "empty");

  // Disconnect first to clear any previous connection state
  WiFi.disconnect();

  // Small delay to ensure disconnect completes
  delay(100);

  // Start connection (AP already active in AP+STA mode)
  WiFi.begin(wifiSSID.c_str(), wifiPassword.c_str());
  wifiConnectStartTime = millis();

  Serial.printf("[WiFi] WiFi.begin() викликано, статус: %d\n", WiFi.status());

  return true;
}

// Check STA connection status
inline bool checkSTAConnection() {
  wl_status_t status = WiFi.status();

  if (status == WL_CONNECTED) {
    if (!wifiConnected) {
      wifiConnected = true;
      localIP = WiFi.localIP();
      Serial.printf("[WiFi] Підключено успішно! IP: %s\n", localIP.toString().c_str());
    }
    return true;
  } else {
    if (wifiConnected) {
      wifiConnected = false;
      Serial.printf("[WiFi] Втрачено підключення до STA, статус: %d\n", status);
    }
    return false;
  }
}

// Initialize AP mode (always active in dual-mode)
inline bool initAPMode() {
  Serial.println("[WiFi] Ініціалізація точки доступу...");
  
  // Generate AP SSID
  String apSSID = String(DEFAULT_AP_PREFIX);
  
  // Start AP with explicit channel and max connections
  bool success = WiFi.softAP(apSSID.c_str(), DEFAULT_AP_PASSWORD, 1, 0, 4);
  
  if (success) {
    IPAddress apIP = WiFi.softAPIP();
    Serial.printf("[WiFi] Точка доступу запущена\n");
    Serial.printf("[WiFi] SSID: %s\n", apSSID.c_str());
    Serial.printf("[WiFi] Password: %s\n", DEFAULT_AP_PASSWORD);
    Serial.printf("[WiFi] AP IP: %s\n", apIP.toString().c_str());
    Serial.printf("[WiFi] Channel: 1, Max clients: 4\n");
    
    // Verify AP is actually running
    delay(500);
    Serial.printf("[WiFi] AP Status: %s\n", WiFi.softAPgetStationNum() > 0 ? "Clients connected" : "No clients");
    return true;
  } else {
    Serial.println("[WiFi] Помилка запуску точки доступу");
    return false;
  }
}

// Handle WiFi Manager logic (dual-mode AP+STA)
inline void handleWiFiManager() {
  static unsigned long lastWiFiCheck = 0;
  static unsigned long lastSTAAttempt = 0;
  static bool staConnecting = false;
  
  // Check WiFi status periodically
  if (millis() - lastWiFiCheck > 3000) { // Check every 3 seconds
    lastWiFiCheck = millis();
    
    // Check STA connection status
    checkSTAConnection();
    
    // Handle STA connection attempts
    if (wifiConfigured && !wifiConnected && !staConnecting) {
      // Try to connect every 30 seconds
      if (millis() - lastSTAAttempt > 30000) {
        Serial.println("[WiFi] Спроба підключення до збереженої мережі...");
        if (startSTAConnection()) {
          staConnecting = true;
          lastSTAAttempt = millis();
        }
      }
    }
    
    // Handle STA connection timeout
    if (staConnecting) {
      if (WiFi.status() == WL_CONNECTED) {
        staConnecting = false;
        Serial.println("[WiFi] STA підключено успішно");
      } else if (millis() - lastSTAAttempt > 20000) { // 20 second timeout
        staConnecting = false;
        Serial.println("[WiFi] STA підключення не вдалося, AP залишається активним");
        // AP stays active - no action needed
      }
    }
  }
}

// Initialize WiFi Manager (dual-mode AP+STA)
inline void initWiFiManager() {
  Serial.println("[WiFi] Ініціалізація WiFi Manager...");
  
  // Load saved configuration
  loadWiFiConfig();
  loadBMSConfig();
  loadAppConfig();
  
  // Set WiFi mode to AP+STA (dual mode)
  WiFi.mode(WIFI_AP_STA);
  Serial.println("[WiFi] Режим AP+STA встановлено");
  
  // Start AP mode (always active)
  if (initAPMode()) {
    Serial.println("[WiFi] Точка доступу успішно запущена");
  } else {
    Serial.println("[WiFi] Помилка запуску точки доступу");
  }
  
  // Try to connect to saved WiFi (non-blocking)
  if (wifiConfigured) {
    Serial.println("[WiFi] Запуск спроби підключення до збереженої мережі...");
    startSTAConnection();
  } else {
    Serial.println("[WiFi] Конфігурація WiFi відсутня, лише AP режим активний");
  }
  
  Serial.println("[WiFi] Ініціалізація завершена");
  Serial.println("[WiFi] Перехід до initWebServer()...");
}

// Scan WiFi networks
inline void scanWiFiNetworks(String networks[], int& count, int maxNetworks = 20) {
  count = 0;

  Serial.println("[WiFi] Початок сканування мереж...");

  // Pause BLE to prevent radio conflict
  BLEDevice::getScan()->stop();
  delay(100);

  // Set WiFi mode to STA+AP for BLE coexistence if not already
  if (WiFi.getMode() == WIFI_OFF) {
    WiFi.mode(WIFI_AP_STA);
  }

  // Disable WiFi sleep mode for better scan reliability
  WiFi.setSleep(false);

  // Use synchronous scan with short timeout to prevent watchdog
  int numNetworks = WiFi.scanNetworks(false, false, false, 3000); // 3 second timeout

  if (numNetworks == 0 || numNetworks == WIFI_SCAN_FAILED) {
    Serial.println("[WiFi] Мереж не знайдено або помилка сканування");
    count = 0;
    WiFi.scanDelete();
    return;
  }

  if (numNetworks < 0) {
    Serial.println("[WiFi] Помилка сканування");
    count = 0;
    WiFi.scanDelete();
    return;
  }

  Serial.printf("[WiFi] Знайдено %d мереж\n", numNetworks);

  // Copy network info
  for (int i = 0; i < numNetworks && i < maxNetworks; i++) {
    String ssid = WiFi.SSID(i);
    int rssi = WiFi.RSSI(i);
    int encryption = WiFi.encryptionType(i);

    // Format: SSID|RSSI|Encryption
    networks[count] = ssid + "|" + String(rssi) + "|" + String(encryption);
    count++;

    Serial.printf("[WiFi] %d: %s (%d dBm) %s\n", i + 1, ssid.c_str(), rssi,
                  encryption == WIFI_AUTH_OPEN ? "[Open]" : "[Secured]");

    yield(); // Prevent watchdog during loop
  }

  // Delete scan results to free memory
  WiFi.scanDelete();

  // Re-enable WiFi sleep mode
  WiFi.setSleep(true);
}

// Get WiFi status string (dual-mode)
inline String getWiFiStatusString() {
  String status = "AP: " + String(DEFAULT_AP_PREFIX);
  
  if (wifiConnected) {
    status += " | STA: " + wifiSSID + " (" + localIP.toString() + ")";
  } else if (wifiConfigured) {
    status += " | STA: Connecting...";
  } else {
    status += " | STA: Not configured";
  }
  
  return status;
}

// Check if WiFi is ready for operations (dual-mode)
inline bool isWiFiReady() {
  // Always ready in dual-mode since AP is always active
  return true;
}

// Get AP IP address
inline IPAddress getAPIP() {
  return WiFi.softAPIP();
}

// Get STA IP address (0.0.0.0 if not connected)
inline IPAddress getSTAIP() {
  return wifiConnected ? WiFi.localIP() : IPAddress(0, 0, 0, 0);
}
