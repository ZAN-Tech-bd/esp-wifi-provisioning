#include "ble_provisioning.h"
#include "config.h"

#include <ArduinoJson.h>
#include <BLE2902.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <WiFi.h>
#include <string.h>

BleProvisioning bleProvisioning;

namespace {

BLECharacteristic *gDeviceInfoChar = nullptr;
BLECharacteristic *gCommandChar = nullptr;
BLECharacteristic *gNetworkChar = nullptr;
BLECharacteristic *gCredentialsChar = nullptr;
BLECharacteristic *gStatusChar = nullptr;

String macSuffix() {
  String mac = WiFi.macAddress();  // "AA:BB:CC:DD:EE:FF"
  mac.replace(":", "");
  return mac.substring(mac.length() - 4);  // last 2 bytes
}

class ServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer *server) override {
    Serial.println("[BLE] client connected");
  }
  void onDisconnect(BLEServer *server) override {
    Serial.println("[BLE] client disconnected, resuming advertising");
    BLEDevice::startAdvertising();
  }
};

class CommandCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *characteristic) override {
    std::string raw = characteristic->getValue();
    if (raw.empty()) return;

    JsonDocument doc;
    if (deserializeJson(doc, raw) != DeserializationError::Ok) {
      Serial.println("[BLE] command: bad JSON");
      return;
    }

    const char *cmd = doc["cmd"] | "";
    if (strcmp(cmd, "scan") == 0) {
      bleProvisioning.requestScan();
    } else if (strcmp(cmd, "reset") == 0) {
      bleProvisioning.requestReset();
    } else if (strcmp(cmd, "cancel") == 0) {
      bleProvisioning.requestCancel();
    } else {
      Serial.printf("[BLE] unknown command: %s\n", cmd);
    }
  }
};

class CredentialsCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *characteristic) override {
    std::string raw = characteristic->getValue();
    if (raw.empty()) return;

    JsonDocument doc;
    if (deserializeJson(doc, raw) != DeserializationError::Ok) {
      Serial.println("[BLE] credentials: bad JSON");
      return;
    }

    const char *ssid = doc["ssid"] | "";
    const char *password = doc["password"] | "";
    if (strlen(ssid) == 0) {
      Serial.println("[BLE] credentials: missing ssid");
      return;
    }

    bleProvisioning.requestConnect(String(ssid), String(password));
  }
};

ServerCallbacks gServerCallbacks;
CommandCallbacks gCommandCallbacks;
CredentialsCallbacks gCredentialsCallbacks;

}  // namespace

void BleProvisioning::begin(WifiManager *wifiManager) {
  wifi_ = wifiManager;

  String deviceName = String(DEVICE_NAME_PREFIX) + macSuffix();
  Serial.printf("[BLE] advertising as %s\n", deviceName.c_str());

  BLEDevice::init(deviceName.c_str());
  BLEServer *server = BLEDevice::createServer();
  server->setCallbacks(&gServerCallbacks);

  BLEService *service = server->createService(SERVICE_UUID);

  gDeviceInfoChar = service->createCharacteristic(
      CHAR_DEVICE_INFO_UUID, BLECharacteristic::PROPERTY_READ);

  gCommandChar = service->createCharacteristic(
      CHAR_COMMAND_UUID, BLECharacteristic::PROPERTY_WRITE);
  gCommandChar->setCallbacks(&gCommandCallbacks);

  gNetworkChar = service->createCharacteristic(
      CHAR_NETWORK_UUID, BLECharacteristic::PROPERTY_NOTIFY);
  gNetworkChar->addDescriptor(new BLE2902());

  gCredentialsChar = service->createCharacteristic(
      CHAR_CREDENTIALS_UUID, BLECharacteristic::PROPERTY_WRITE);
  gCredentialsChar->setCallbacks(&gCredentialsCallbacks);

  gStatusChar = service->createCharacteristic(
      CHAR_STATUS_UUID,
      BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);
  gStatusChar->addDescriptor(new BLE2902());

  {
    JsonDocument doc;
    doc["device"] = DEVICE_NAME_PREFIX;
    doc["fw"] = FIRMWARE_VERSION;
    doc["mac"] = WiFi.macAddress();
    doc["chip"] = CHIP_LABEL;
    String out;
    serializeJson(doc, out);
    gDeviceInfoChar->setValue(out.c_str());
  }

  publishStatus("{\"status\":\"idle\"}");

  service->start();

  BLEAdvertising *advertising = BLEDevice::getAdvertising();
  advertising->addServiceUUID(SERVICE_UUID);
  advertising->setScanResponse(true);
  advertising->setMinPreferred(0x06);
  advertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();

  state_ = State::Idle;
  stateEnteredMs_ = millis();
}

void BleProvisioning::requestScan() { pendingScan_ = true; }
void BleProvisioning::requestReset() { pendingReset_ = true; }
void BleProvisioning::requestCancel() { pendingCancel_ = true; }

void BleProvisioning::requestConnect(const String &ssid, const String &password) {
  pendingSsid_ = ssid;
  pendingPassword_ = password;
  pendingCredentials_ = true;
}

void BleProvisioning::publishStatus(const String &json) {
  gStatusChar->setValue(json.c_str());
  gStatusChar->notify();
}

void BleProvisioning::handleScanRequest() {
  state_ = State::Scanning;
  stateEnteredMs_ = millis();
  publishStatus("{\"status\":\"scanning\"}");

  std::vector<WifiNetwork> networks = wifi_->scanNetworks();
  size_t total = networks.size();

  for (size_t i = 0; i < total; i++) {
    JsonDocument doc;
    doc["type"] = "network";
    doc["index"] = (int)i;
    doc["total"] = (int)total;
    doc["ssid"] = networks[i].ssid;
    doc["rssi"] = networks[i].rssi;
    doc["secure"] = networks[i].secure;
    String out;
    serializeJson(doc, out);
    gNetworkChar->setValue(out.c_str());
    gNetworkChar->notify();
    delay(20);  // give the central time to drain its notify queue
  }

  publishStatus("{\"status\":\"scan_complete\"}");
  state_ = State::Idle;
  stateEnteredMs_ = millis();
}

void BleProvisioning::handleCredentialsRequest() {
  state_ = State::Connecting;
  stateEnteredMs_ = millis();

  {
    JsonDocument doc;
    doc["status"] = "connecting";
    doc["ssid"] = pendingSsid_;
    String out;
    serializeJson(doc, out);
    publishStatus(out);
  }

  WifiConnectResult result = wifi_->connectWithCredentials(
      pendingSsid_, pendingPassword_, WIFI_CONNECT_TIMEOUT_MS);

  JsonDocument doc;
  if (result == WifiConnectResult::Success) {
    wifi_->saveCredentials(pendingSsid_, pendingPassword_);
    doc["status"] = "connected";
    doc["ssid"] = pendingSsid_;
    doc["ip"] = wifi_->localIp();
    doc["rssi"] = WiFi.RSSI();
    state_ = State::Connected;
  } else {
    doc["status"] = "failed";
    doc["ssid"] = pendingSsid_;
    switch (result) {
      case WifiConnectResult::WrongPassword: doc["reason"] = "wrong_password"; break;
      case WifiConnectResult::NotFound:      doc["reason"] = "not_found"; break;
      case WifiConnectResult::Timeout:       doc["reason"] = "timeout"; break;
      default:                               doc["reason"] = "unknown"; break;
    }
    state_ = State::Idle;
  }
  stateEnteredMs_ = millis();

  String out;
  serializeJson(doc, out);
  publishStatus(out);
}

void BleProvisioning::loop() {
  if (pendingReset_) {
    pendingReset_ = false;
    wifi_->clearCredentials();
    wifi_->disconnect();
    publishStatus("{\"status\":\"idle\"}");
    state_ = State::Idle;
    stateEnteredMs_ = millis();
    Serial.println("[BLE] credentials cleared by app request");
  }

  if (pendingCancel_) {
    pendingCancel_ = false;
    if (state_ == State::Connecting) {
      wifi_->disconnect();
      publishStatus("{\"status\":\"idle\"}");
      state_ = State::Idle;
      stateEnteredMs_ = millis();
    }
  }

  if (pendingScan_) {
    pendingScan_ = false;
    handleScanRequest();
  }

  if (pendingCredentials_) {
    pendingCredentials_ = false;
    handleCredentialsRequest();
  }

  if (PROVISIONING_TIMEOUT_MS > 0 && state_ == State::Idle &&
      millis() - stateEnteredMs_ > PROVISIONING_TIMEOUT_MS) {
    Serial.println("[BLE] provisioning timeout reached, rebooting");
    ESP.restart();
  }
}
