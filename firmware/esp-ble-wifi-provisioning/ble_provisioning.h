/*
 * ZAN Tech - BLE Wi-Fi Provisioning
 * ble_provisioning.h - GATT server + state machine implementing
 * docs/BLE_PROTOCOL.md. BLE characteristic callbacks only ever set a flag
 * or copy a small string (see requestScan/requestReset/requestCancel/
 * requestConnect) - all the actual work (Wi-Fi scanning, connecting)
 * happens from loop() on the main task, never inside a BLE callback.
 */
#pragma once

#include <Arduino.h>
#include "wifi_manager.h"

class BleProvisioning {
 public:
  void begin(WifiManager *wifiManager);
  void loop();

  // Safe to call from BLE callback context (a different FreeRTOS task).
  void requestScan();
  void requestReset();
  void requestCancel();
  void requestConnect(const String &ssid, const String &password);

 private:
  enum class State { Idle, Scanning, Connecting, Connected };

  WifiManager *wifi_ = nullptr;
  State state_ = State::Idle;

  volatile bool pendingScan_ = false;
  volatile bool pendingReset_ = false;
  volatile bool pendingCancel_ = false;
  volatile bool pendingCredentials_ = false;
  String pendingSsid_;
  String pendingPassword_;

  uint32_t stateEnteredMs_ = 0;

  void handleScanRequest();
  void handleCredentialsRequest();
  void publishStatus(const String &json);
};

extern BleProvisioning bleProvisioning;
