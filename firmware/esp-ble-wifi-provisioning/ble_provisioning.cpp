#include "ble_provisioning.h"
#include "config.h"

#include <ArduinoJson.h>
#include <NimBLEDevice.h>
#include <WiFi.h>
#include <esp_mac.h>

BleProvisioning bleProvisioning;

namespace {

NimBLECharacteristic *gDeviceInfoChar = nullptr;
NimBLECharacteristic *gCommandChar = nullptr;
NimBLECharacteristic *gNetworkChar = nullptr;
NimBLECharacteristic *gCredentialsChar = nullptr;
NimBLECharacteristic *gStatusChar = nullptr;

// WiFi.macAddress() can return all-zeros if called before the WiFi driver
// has fully started (esp_wifi_start() hasn't run yet at this point in
// boot - we go straight to BLE provisioning without ever calling
// WiFi.begin()/scan first). Read the factory-programmed MAC straight from
// eFuse instead, which needs no driver state at all.
void readStationMac(uint8_t mac[6]) {
  esp_read_mac(mac, ESP_MAC_WIFI_STA);
}

String macToString(const uint8_t mac[6]) {
  char buf[18];
  snprintf(buf, sizeof(buf), "%02X:%02X:%02X:%02X:%02X:%02X",
           mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  return String(buf);
}

String macSuffix() {
  uint8_t mac[6];
  readStationMac(mac);
  char buf[5];
  snprintf(buf, sizeof(buf), "%02X%02X", mac[4], mac[5]);
  return String(buf);
}

class ServerCallbacks : public NimBLEServerCallbacks {
  void onConnect(NimBLEServer *server, NimBLEConnInfo &connInfo) override {
    Serial.println("[BLE] client connected");
  }
  void onDisconnect(NimBLEServer *server, NimBLEConnInfo &connInfo, int reason) override {
    Serial.println("[BLE] client disconnected, resuming advertising");
    NimBLEDevice::startAdvertising();
  }
};

class CommandCallbacks : public NimBLECharacteristicCallbacks {
  void onWrite(NimBLECharacteristic *characteristic, NimBLEConnInfo &connInfo) override {
    const NimBLEAttValue &value = characteristic->getValue();
    if (value.length() == 0) return;

    JsonDocument doc;
    if (deserializeJson(doc, value.c_str(), value.length()) != DeserializationError::Ok) {
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

class CredentialsCallbacks : public NimBLECharacteristicCallbacks {
  void onWrite(NimBLECharacteristic *characteristic, NimBLEConnInfo &connInfo) override {
    const NimBLEAttValue &value = characteristic->getValue();
    if (value.length() == 0) return;

    JsonDocument doc;
    if (deserializeJson(doc, value.c_str(), value.length()) != DeserializationError::Ok) {
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

  NimBLEDevice::init(deviceName.c_str());
  NimBLEServer *server = NimBLEDevice::createServer();
  server->setCallbacks(&gServerCallbacks);

  NimBLEService *service = server->createService(SERVICE_UUID);

  gDeviceInfoChar = service->createCharacteristic(
      CHAR_DEVICE_INFO_UUID, NIMBLE_PROPERTY::READ);

  gCommandChar = service->createCharacteristic(
      CHAR_COMMAND_UUID, NIMBLE_PROPERTY::WRITE);
  gCommandChar->setCallbacks(&gCommandCallbacks);

  gNetworkChar = service->createCharacteristic(
      CHAR_NETWORK_UUID, NIMBLE_PROPERTY::NOTIFY);

  gCredentialsChar = service->createCharacteristic(
      CHAR_CREDENTIALS_UUID, NIMBLE_PROPERTY::WRITE);
  gCredentialsChar->setCallbacks(&gCredentialsCallbacks);

  gStatusChar = service->createCharacteristic(
      CHAR_STATUS_UUID, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY);

  {
    JsonDocument doc;
    doc["device"] = DEVICE_NAME_PREFIX;
    doc["fw"] = FIRMWARE_VERSION;
    uint8_t mac[6];
    readStationMac(mac);
    doc["mac"] = macToString(mac);
    doc["chip"] = CHIP_LABEL;
    String out;
    serializeJson(doc, out);
    gDeviceInfoChar->setValue(out.c_str());
  }

  // Set the initial value directly (no client is connected yet, so there is
  // nothing to notify) - notify() must not be called before service->start(),
  // or the BLE stack rejects it / misbehaves.
  gStatusChar->setValue("{\"status\":\"idle\"}");

  service->start();

  NimBLEAdvertising *advertising = NimBLEDevice::getAdvertising();
  advertising->addServiceUUID(SERVICE_UUID);
  advertising->enableScanResponse(true);
  advertising->start();

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
