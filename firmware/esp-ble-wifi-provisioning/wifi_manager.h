/*
 * ZAN Tech - BLE Wi-Fi Provisioning
 * wifi_manager.h - stores/loads credentials in NVS, connects, scans.
 * Deliberately has zero BLE knowledge so it can be reused standalone.
 */
#pragma once

#include <Arduino.h>
#include <vector>

struct WifiNetwork {
  String ssid;
  int32_t rssi;
  bool secure;
};

enum class WifiConnectResult {
  Success,
  WrongPassword,
  NotFound,
  Timeout,
  Unknown
};

class WifiManager {
 public:
  void begin();

  bool hasStoredCredentials();
  void loadStoredCredentials(String &ssid, String &password);
  void saveCredentials(const String &ssid, const String &password);
  void clearCredentials();

  // Blocking connect using currently stored credentials.
  WifiConnectResult connectStored(uint32_t timeoutMs);

  // Blocking connect using explicit credentials (does not persist them -
  // caller decides whether to saveCredentials() after success).
  WifiConnectResult connectWithCredentials(const String &ssid,
                                            const String &password,
                                            uint32_t timeoutMs);

  void disconnect();
  bool isConnected();
  String localIp();

  // Synchronous scan, sorted strongest-first, deduplicated by SSID.
  std::vector<WifiNetwork> scanNetworks();

 private:
  WifiConnectResult waitForConnection(uint32_t timeoutMs);
};
