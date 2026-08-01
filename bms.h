// bms.h - BMS BLE Communication Module (Header-only)
#pragma once

#include "GLOBAL_STATE.h"
#include <Preferences.h>

// BLE Callbacks - No filtering, show all devices
class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    String deviceName = advertisedDevice.getName().c_str();
    String deviceAddress = advertisedDevice.getAddress().toString().c_str();
    
    // Use MAC address as name if device has no name
    String displayName = deviceName.length() > 0 ? deviceName : "Unknown_" + deviceAddress.substring(12);
    
    Serial.print("Знайдено BLE пристрій: ");
    Serial.print(deviceAddress);
    Serial.print(" - ");
    Serial.println(displayName);
    
    // Add all devices, no filtering
    if (bmsCount < 20) {
      availableBMS[bmsCount] = deviceAddress + "|" + displayName;
      bmsCount++;
    }
  }
};

class MyClientCallback : public BLEClientCallbacks {
  void onConnect(BLEClient* pclient) {
    Serial.println("[BLE] Підключено до сервера");
  }

  void onDisconnect(BLEClient* pclient) {
    Serial.println("[BLE] Відключено від сервера");
    bmsConnected = false;
  }
};

// BLE Notification Callback with proper fragment assembly
inline void notifyCallback(BLERemoteCharacteristic* pBLERemoteCharacteristic,
                    uint8_t* pData, size_t length, bool isNotify) {

  Serial.print("[BLE] Отримано фрагмент: ");
  for (size_t i = 0; i < length; i++) {
    Serial.printf("%02X ", pData[i]);
  }
  Serial.println();

  // If buffer is empty and first byte is NOT 0xDD, ignore this fragment (old packet leftovers)
  if (bleRxLen == 0 && (length == 0 || pData[0] != 0xDD)) {
    Serial.println("[BLE] Ігноруємо фрагмент без start byte 0xDD");
    return;
  }

  // Check buffer overflow
  if (bleRxLen + length > sizeof(bleRxBuffer)) {
    Serial.println("[BLE] Переповнення буфера - скидання");
    bleRxLen = 0;
    memset(bleRxBuffer, 0, sizeof(bleRxBuffer));
    return;
  }

  // Append bytes sequentially
  memcpy(&bleRxBuffer[bleRxLen], pData, length);
  bleRxLen += length;

  Serial.printf("[BLE] Буфер: %d байтів\n", bleRxLen);

  // Check for complete packet: must have minimum length and end byte 0x77
  if (bleRxLen >= 7 && bleRxBuffer[bleRxLen - 1] == 0x77) {
    // Verify start byte is 0xDD
    if (bleRxBuffer[0] == 0xDD) {
      Serial.print("[BLE] Повний пакет зібрано: ");
      for (size_t i = 0; i < bleRxLen; i++) {
        Serial.printf("%02X ", bleRxBuffer[i]);
      }
      Serial.println();

      // Copy to global response buffer
      memcpy(bmsResponse, bleRxBuffer, bleRxLen);
      bmsResponseLength = bleRxLen;
      newDataReceived = true;

      // Clear buffer after successful packet assembly
      bleRxLen = 0;
      memset(bleRxBuffer, 0, sizeof(bleRxBuffer));
    } else {
      Serial.println("[BLE] Пакет не починається з 0xDD - скидання");
      bleRxLen = 0;
      memset(bleRxBuffer, 0, sizeof(bleRxBuffer));
    }
  }
}

// JBD Checksum calculation according to protocol documentation
// Sum bytes from Length field (byte 3) through end of Data payload
// Start byte (0xDD) and Command byte (0xA5/0x5A) are NOT included
inline uint16_t jbdChecksum(const uint8_t* data, size_t len) {
  uint16_t sum = 0;
  for (size_t i = 0; i < len; i++) {
    sum += data[i];
  }
  return ((~sum) + 1) & 0xFFFF;
}

// Decode protection state according to JBD protocol
inline String decodeProtectionState(uint16_t protectionStatus) {
  String result = "";

  if (protectionStatus & 0x0001) result += "Cell OVP, ";
  if (protectionStatus & 0x0002) result += "Cell UVP, ";
  if (protectionStatus & 0x0004) result += "Pack OVP, ";
  if (protectionStatus & 0x0008) result += "Pack UVP, ";
  if (protectionStatus & 0x0010) result += "Over-temp Charge, ";
  if (protectionStatus & 0x0020) result += "Under-temp Charge, ";
  if (protectionStatus & 0x0040) result += "Over-temp Discharge, ";
  if (protectionStatus & 0x0080) result += "Under-temp Discharge, ";
  if (protectionStatus & 0x0100) result += "Over-current Charge, ";
  if (protectionStatus & 0x0200) result += "Over-current Discharge, ";
  if (protectionStatus & 0x0400) result += "Short Circuit, ";
  if (protectionStatus & 0x0800) result += "Front-end IC Error, ";
  if (protectionStatus & 0x1000) result += "MOS Software Lock, ";

  // Remove trailing comma and space
  if (result.length() > 2) {
    result = result.substring(0, result.length() - 2);
  }

  return result;
}

// Verify checksum according to JBD protocol documentation
// Sum bytes from Length field (byte 3) through end of Data payload
// Start byte (0xDD) and Command byte (0xA5/0x5A) are NOT included
inline bool verifyChecksum(const uint8_t* data, size_t length) {
  if (length < 6) {
    Serial.println("Занадто коротка відповідь для checksum");
    return false;
  }

  if (data[length - 1] != 0x77) {
    Serial.println("Відсутній кінцевий байт 0x77");
    return false;
  }

  // Debug logging
  Serial.printf("[Checksum] Length=%u\n", length);
  Serial.print("[Checksum] Data: ");
  for (size_t i = 0; i < length; i++) {
    Serial.printf("%02X ", data[i]);
  }
  Serial.println();

  uint16_t receivedChecksum = (data[length - 3] << 8) | data[length - 2];

  // Calculate checksum according to protocol: skip bytes 0 (0xDD) and 1 (0xA5/0x5A)
  // Sum from byte 3 (Length) through end of Data payload (exclude last 3 bytes: checksum + end)
  uint16_t calculatedChecksum = jbdChecksum(data + 3, length - 6);

  Serial.printf("[Checksum] Calculated=%04X  Received=%04X\n", calculatedChecksum, receivedChecksum);

  bool result = (calculatedChecksum == receivedChecksum);
  if (!result) {
    Serial.println("Checksum помилка - ПАКЕТ ВІДХИЛЕНО");
    return false;
  } else {
    Serial.println("Checksum OK");
  }

  return result;
}

// Send BMS command
inline bool sendBMSCommand(uint8_t* cmd, size_t cmdLen, uint32_t timeout) {
  if (!pClient || !pClient->isConnected() || !pRxCharacteristic) {
    Serial.println("BLE не підключено");
    return false;
  }

  Serial.print("[BMS] Відправка команди: ");
  for (size_t i = 0; i < cmdLen; i++) {
    Serial.printf("%02X ", cmd[i]);
  }
  Serial.println();

  // Clear buffers before new request
  newDataReceived = false;
  bmsResponseLength = 0;
  memset(bmsResponse, 0, sizeof(bmsResponse));
  bleRxLen = 0;  // Clear BLE fragment buffer
  memset(bleRxBuffer, 0, sizeof(bleRxBuffer));

  pRxCharacteristic->writeValue(cmd, cmdLen, false);

  uint32_t startTime = millis();
  while (!newDataReceived && (millis() - startTime) < timeout) {
    yield();  // Critical: allow background tasks to run
  }

  if (!newDataReceived) {
    Serial.println("Таймаут очікування відповіді");
    return false;
  }

  // Small delay for WiFi stability
  delay(100);

  // Strict checksum verification - no fallback for bad packets
  if (!verifyChecksum(bmsResponse, bmsResponseLength)) {
    Serial.println("Помилка checksum в відповіді - ПАКЕТ ВІДХИЛЕНО");
    return false;
  } else {
    Serial.println("Checksum пройдено успішно");
  }

  return true;
}

// Parse basic BMS info
inline bool readBasicInfo() {
  uint8_t cmd[] = {0xDD, 0xA5, 0x03, 0x00, 0xFF, 0xFD, 0x77};

  if (!sendBMSCommand(cmd, sizeof(cmd), 350)) {  // Increased timeout to 350ms for BLE fragment assembly
    return false;
  }

  if (bmsResponseLength < 0x1D) {
    Serial.println("Неповна відповідь для базової інформації");
    return false;
  }

  // Lock mutex to protect BMS data during write
  if (bmsDataMutex != nullptr && xSemaphoreTake(bmsDataMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
    // Parse data according to JBD protocol specification
    // Payload starts at data[4] (after header: 0xDD, 0xA5, command, length)
    bmsData.totalVoltage = ((bmsResponse[4] << 8) | bmsResponse[5]) / 100.0;
    bmsData.current = (int16_t)((bmsResponse[6] << 8) | bmsResponse[7]) / 100.0;
    bmsData.capacityRemaining = ((bmsResponse[8] << 8) | bmsResponse[9]) / 100.0;
    bmsData.capacityTotal = ((bmsResponse[10] << 8) | bmsResponse[11]) / 100.0;
    bmsData.cycleCount = (bmsResponse[12] << 8) | bmsResponse[13];
    bmsData.productionDate = String("20") + String(bmsResponse[14]) + "/" +
                            String(bmsResponse[15]) + "/" + String(bmsResponse[16]);
    bmsData.balanceStatus = (bmsResponse[17] << 8) | bmsResponse[18];
    bmsData.balanceStatusHigh = (bmsResponse[19] << 8) | bmsResponse[20];
    bmsData.protectionStatus = (bmsResponse[20] << 8) | bmsResponse[21];  // Fixed: bytes 20-21
    bmsData.softwareVersion = bmsResponse[22];  // Hardware version
    bmsData.soc = bmsResponse[23];  // Fixed: RSOC in byte 23
    bmsData.fetStatus = bmsResponse[24];  // Fixed: FET Status in byte 24
    bmsData.cellCount = bmsResponse[25];  // Fixed: Cell Count in byte 25
    bmsData.tempSensorCount = bmsResponse[26];  // Fixed: NTC Count in byte 26

    // Parse temperatures (0.1K -> °C: (value - 2731) / 10)
    // Temperatures start at byte 27
    for (int i = 0; i < bmsData.tempSensorCount && i < 6; i++) {
      uint16_t tempRaw = (bmsResponse[27 + i * 2] << 8) | bmsResponse[27 + i * 2 + 1];
      bmsData.temperatures[i] = (tempRaw - 2731) / 10.0;
    }
    // Clear unused temperature sensors
    for (int i = bmsData.tempSensorCount; i < 6; i++) {
      bmsData.temperatures[i] = 0;
    }

    Serial.printf("[BMS] Напруга: %.2fV, Струм: %.2fA, SOC: %d%%\n",
                  bmsData.totalVoltage, bmsData.current, bmsData.soc);
    Serial.printf("[BMS] Температурні датчики: %d, T1: %.1f°C\n",
                  bmsData.tempSensorCount, bmsData.temperatures[0]);

    // Decode and log protection status
    String protectionDecoded = decodeProtectionState(bmsData.protectionStatus);
    if (protectionDecoded.length() > 0) {
      Serial.printf("[BMS] Захисти: %s (0x%04X)\n", protectionDecoded.c_str(), bmsData.protectionStatus);
    }

    // Unlock mutex
    xSemaphoreGive(bmsDataMutex);
  } else {
    Serial.println("[BMS] Помилка захоплення мутексу в readBasicInfo");
    return false;
  }

  yield();  // Allow background processing

  return true;
}

// Parse cell voltages
inline bool readCellVoltages() {
  uint8_t cmd[] = {0xDD, 0xA5, 0x04, 0x00, 0xFF, 0xFC, 0x77};

  if (!sendBMSCommand(cmd, sizeof(cmd), 350)) {  // Increased timeout for BLE fragment assembly
    return false;
  }

  // Calculate number of cells from payload length (byte 3)
  // Each cell takes 2 bytes, so cells = data[3] / 2
  uint8_t cellsInPacket = bmsResponse[3] / 2;
  size_t expectedLength = 4 + (cellsInPacket * 2) + 2;

  if (bmsResponseLength < expectedLength) {
    Serial.printf("Неповна відповідь для напруг банок: очікується %d, отримано %d\n",
                  expectedLength, bmsResponseLength);
    return false;
  }

  // Lock mutex to protect BMS data during write
  if (bmsDataMutex != nullptr && xSemaphoreTake(bmsDataMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
    // Parse cell voltages according to JBD protocol
    // Cell voltages start at byte 4, each cell is 2 bytes (Big-Endian)
    for (int i = 0; i < cellsInPacket && i < 24; i++) {
      uint16_t voltage = (bmsResponse[4 + i * 2] << 8) | bmsResponse[4 + i * 2 + 1];
      bmsData.cellVoltages[i] = voltage / 1000.0;  // Convert mV to V
    }

    // Update cell count from packet
    bmsData.cellCount = cellsInPacket;

    Serial.printf("[BMS] Напруги банок прочитано: %d комірок\n", cellsInPacket);

    // Unlock mutex
    xSemaphoreGive(bmsDataMutex);
  } else {
    Serial.println("[BMS] Помилка захоплення мутексу в readCellVoltages");
    return false;
  }

  yield();

  return true;
}

// MOSFET control
inline void controlMOSFET(uint8_t state) {
  uint8_t cmd[] = {0xDD, 0xA5, 0x94, 0x01, state, 0x00, 0x77};
  uint16_t checksum = jbdChecksum(cmd, 5);
  
  cmd[5] = (checksum >> 8) & 0xFF;
  cmd[6] = checksum & 0xFF;
  
  Serial.printf("[BMS] Керування MOSFET: %02X\n", state);
  
  if (sendBMSCommand(cmd, sizeof(cmd), 1000)) {
    Serial.println("[BMS] MOSFET команда відправлена успішно");
  } else {
    Serial.println("[BMS] Помилка відправки MOSFET команди");
  }
}

// Initialize BMS
inline void initBMS() {
  Serial.println("[BMS] initBMS() called");
  memset(&bmsData, 0, sizeof(bmsData));
  BLEDevice::init("BMS-Controller");
  Serial.println("[BMS] BLE initialized");
  
  // Завантажити BMS MAC з Preferences
  Preferences prefs;
  prefs.begin("bms_app", true);  // read-only
  BMS_MAC = prefs.getString("bms_mac", "");
  BMS_NAME = prefs.getString("name", "");
  prefs.end();
  
  if (BMS_MAC.length() > 0) {
    Serial.printf("[BMS] Завантажено збережену конфігурацію: %s (%s)\n", BMS_MAC.c_str(), BMS_NAME.c_str());
  }
  Serial.println("[BMS] initBMS() completed");
}

// Start BLE scan
inline void startBLEScan() {
  // Prohibit scanning if already connected to BMS
  if (bmsConnected) {
    Serial.println("[BLE] Сканування заборонено - вже підключено до BMS");
    return;
  }

  Serial.println("=== ЗАПУСК СКАНУВАННЯ BLE ===");
  bmsCount = 0;
  isScanning = true;

  // Pause WiFi to prevent radio conflict
  WiFi.disconnect();
  delay(100);

  BLEScan* pBLEScan = BLEDevice::getScan();
  pBLEScan->stop();
  delay(100);

  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true);
  pBLEScan->start(3, false); // Reduced to 3 seconds

  // Non-blocking wait for scan
  static unsigned long scanStartTime = 0;
  scanStartTime = millis();
  while (millis() - scanStartTime < 3000) {
    yield();
  }
  Serial.println("=== СКАНУВАННЯ ЗАВЕРШЕНО ===");
  Serial.print("Знайдено пристроїв: ");
  Serial.println(bmsCount);

  isScanning = false;

  // Restore WiFi mode
  WiFi.mode(WIFI_AP_STA);
  delay(100);
}

// Connect to BMS
inline bool connectToBMS(const String& mac, const String& name) {
  if (mac.length() == 0) {
    Serial.println("MAC адреса не вказана");
    return false;
  }
  
  Serial.printf("Підключення до BMS: %s (%s)\n", mac.c_str(), name.c_str());
  
  BLEAddress address(mac.c_str());
  if (!pClient) {
    pClient = BLEDevice::createClient();
    pClient->setClientCallbacks(new MyClientCallback());
  }
  
  if (pClient->isConnected()) {
    pClient->disconnect();
  }
  
  if (!pClient->connect(address)) {
    Serial.println("Не вдалося підключитися до BMS");
    return false;
  }
  
  BLERemoteService* pRemoteService = pClient->getService(SERVICE_UUID);
  if (!pRemoteService) {
    Serial.println("Сервіс не знайдено");
    pClient->disconnect();
    return false;
  }
  
  pTxCharacteristic = pRemoteService->getCharacteristic(CHAR_TX_UUID);
  pRxCharacteristic = pRemoteService->getCharacteristic(CHAR_RX_UUID);
  
  if (!pTxCharacteristic || !pRxCharacteristic) {
    Serial.println("Характеристики не знайдено");
    pClient->disconnect();
    return false;
  }
  
  if (pTxCharacteristic->canNotify()) {
    pTxCharacteristic->registerForNotify(notifyCallback);
  }
  
  bmsConnected = true;
  BMS_MAC = mac;
  BMS_NAME = name;
  
  // Збереження BMS MAC в Preferences
  Preferences prefs;
  prefs.begin("bms_app", false);
  prefs.putString("bms_mac", mac);
  prefs.putString("name", name);
  prefs.end();
  Serial.println("[BMS] Збережено BMS конфігурацію");
  
  Serial.println("Підключено до BMS успішно!");
  return true;
}

// Disconnect BMS
inline void disconnectBMS() {
  if (pClient) {
    pClient->disconnect();
    bmsConnected = false;
  }
}

// Update BMS data
inline bool updateBMSData() {
  if (!bmsConnected) {
    return false;
  }

  Serial.println("[BMS] Оновлення даних...");

  // Read basic info
  if (!readBasicInfo()) {
    Serial.println("[BMS] Помилка читання базової інформації");
    return false;
  }

  // Delay between commands for BLE/WiFi stability on ESP32-C3
  delay(100);  // Keep this delay for radio coexistence
  yield();

  // Read cell voltages
  if (!readCellVoltages()) {
    Serial.println("[BMS] Помилка читання напруг банок");
    return false;
  }

  Serial.println("[BMS] Дані успішно оновлені");
  yield();
  return true;
}
