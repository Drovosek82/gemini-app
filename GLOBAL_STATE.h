// GLOBAL_STATE.h - Centralized Global State (Header-only, Local Version)
#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <BLEDevice.h>
#include <BLEClient.h>
#include <ArduinoJson.h>
#include <Preferences.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

// BMS Data Structure
struct BMSData {
  float totalVoltage;
  float current;
  float capacityRemaining;
  float capacityTotal;
  uint16_t cycleCount;
  uint8_t soc;
  uint8_t cellCount;
  float cellVoltages[24];
  float temperatures[6];
  uint16_t balanceStatus;
  uint16_t balanceStatusHigh;
  uint16_t protectionStatus;
  uint8_t fetStatus;
  uint8_t softwareVersion;
  uint8_t tempSensorCount;
  String productionDate;
  String hardwareVersion;
};

// BLE Configuration (inline definitions)
inline const BLEUUID SERVICE_UUID("0000ff00-0000-1000-8000-00805f9b34fb");
inline const BLEUUID CHAR_TX_UUID("0000ff01-0000-1000-8000-00805f9b34fb");
inline const BLEUUID CHAR_RX_UUID("0000ff02-0000-1000-8000-00805f9b34fb");

// WiFi Configuration (inline definitions)
inline const char* DEFAULT_AP_PREFIX = "BMS_Setup";
inline const char* DEFAULT_AP_PASSWORD = "12345678";
inline String wifiSSID = "";
inline String wifiPassword = "";
inline bool wifiConfigured = false;
inline bool wifiConnected = false;
inline IPAddress localIP;

// Display State (inline definitions)
inline bool displayDetected = false;
inline bool displayActive = false;

// API Configuration (inline definitions)
inline String apiServer = "https://461983bf-ecc9-4747-8db2-e82b27bd579d-00-1m6pgpxvvfymx-cyebw660.worf.replit.dev/api/bms/push";
inline String deviceId = "bms_001";

// BLE Objects (inline definitions)
inline BLEClient* pClient = nullptr;
inline BLERemoteCharacteristic* pTxCharacteristic = nullptr;
inline BLERemoteCharacteristic* pRxCharacteristic = nullptr;

// BMS State (inline definitions)
inline BMSData bmsData;
inline bool bmsConnected = false;
inline String BMS_MAC = "";
inline String BMS_NAME = "";
inline uint8_t bmsResponse[512];
inline size_t bmsResponseLength = 0;
inline bool newDataReceived = false;
inline String availableBMS[20];
inline int bmsCount = 0;
inline bool isScanning = false;

// BLE fragment buffer for JBD/Xiaoxiang BMS
inline uint8_t bleRxBuffer[64];
inline uint8_t bleRxLen = 0;

// Mutex for protecting BMS data access
inline SemaphoreHandle_t bmsDataMutex = nullptr;


// Timing Variables (inline definitions)
inline unsigned long lastBMSUpdate = 0;
inline unsigned long lastDisplayUpdate = 0;
inline unsigned long lastWiFiCheck = 0;
inline unsigned long wifiConnectStartTime = 0;
inline unsigned long lastReconnectAttempt = 0;

// Constants (inline definitions)
inline const unsigned long BMS_UPDATE_INTERVAL = 5000;  // Збільшити до 5 секунд замість 2
inline const unsigned long DISPLAY_UPDATE_INTERVAL = 3000;  // Збільшити до 3 секунд замість 1
inline const unsigned long WIFI_CONNECT_TIMEOUT = 15000;  // 15 seconds timeout - fallback to AP mode if no connection
inline const unsigned long WIFI_CHECK_INTERVAL = 10000;

// WiFi Manager State (dual-mode AP+STA always active)
inline bool shouldRestart = false;

// API Client State (inline definitions)
inline bool apiSaved = false;

// Function Declarations
void controlMOSFET(uint8_t state);
bool readBasicInfo();
bool readCellVoltages();
bool sendBMSCommand(const uint8_t* cmd, size_t cmdLen, uint32_t timeout = 1000);
bool verifyChecksum(const uint8_t* data, size_t length);
uint16_t jbdChecksum(const uint8_t* data, size_t length);
String decodeProtectionState(uint16_t protectionStatus);

// WiFi Manager Functions (dual-mode)
bool startSTAConnection();
bool checkSTAConnection();
bool initAPMode();
void handleWiFiManager();
bool saveWiFiConfig(const String& ssid, const String& password);
void loadWiFiConfig();
void resetWiFiConfig();
String getWiFiStatusString();
bool isWiFiReady();
IPAddress getAPIP();
IPAddress getSTAIP();

// BMS Config Functions (Preferences)
bool saveBMSConfig(const String& mac);
void loadBMSConfig();
void resetBMSConfig();

// Web Server Functions
void initWebServer();
void handleRoot();
void handleData();
void handleControl();
void handleScan();
void handleWiFiScan();
void handleConnect();
void handleDisconnect();
void handleDisplayWeb();
void handleBalance();
void handleAutoPoll();
void handleCloudSettings();
void handleCloudSave();
void handleSystem();
void handleSave();
void handleOptions();
void handleNotFound();
