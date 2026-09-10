#include "wifi_manager.h"
#include "config.h"

#include <WiFi.h>
#include <Preferences.h>
#include <algorithm>
#include <map>

static Preferences prefs;

void WifiManager::begin() {
  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);
  WiFi.persistent(false);  // we manage persistence ourselves via NVS namespace below
}

bool WifiManager::hasStoredCredentials() {
  prefs.begin(NVS_NAMESPACE, true);
  bool has = prefs.isKey(NVS_KEY_SSID) && prefs.getString(NVS_KEY_SSID, "").length() > 0;
  prefs.end();
  return has;
}

void WifiManager::loadStoredCredentials(String &ssid, String &password) {
  prefs.begin(NVS_NAMESPACE, true);
  ssid = prefs.getString(NVS_KEY_SSID, "");
  password = prefs.getString(NVS_KEY_PASSWORD, "");
  prefs.end();
}

void WifiManager::saveCredentials(const String &ssid, const String &password) {
  prefs.begin(NVS_NAMESPACE, false);
  prefs.putString(NVS_KEY_SSID, ssid);
  prefs.putString(NVS_KEY_PASSWORD, password);
  prefs.end();
}

void WifiManager::clearCredentials() {
  prefs.begin(NVS_NAMESPACE, false);
  prefs.clear();
  prefs.end();
}

WifiConnectResult WifiManager::connectStored(uint32_t timeoutMs) {
  String ssid, password;
  loadStoredCredentials(ssid, password);
  if (ssid.length() == 0) {
    return WifiConnectResult::NotFound;
  }
  return connectWithCredentials(ssid, password, timeoutMs);
}

WifiConnectResult WifiManager::connectWithCredentials(const String &ssid,
                                                       const String &password,
                                                       uint32_t timeoutMs) {
  WiFi.disconnect(true, false);
  delay(100);
  WiFi.begin(ssid.c_str(), password.c_str());
  return waitForConnection(timeoutMs);
}

WifiConnectResult WifiManager::waitForConnection(uint32_t timeoutMs) {
  uint32_t start = millis();
  while (millis() - start < timeoutMs) {
    wl_status_t status = WiFi.status();
    if (status == WL_CONNECTED) {
      return WifiConnectResult::Success;
    }
    if (status == WL_CONNECT_FAILED) {
      return WifiConnectResult::WrongPassword;
    }
    if (status == WL_NO_SSID_AVAIL) {
      return WifiConnectResult::NotFound;
    }
    delay(200);
  }
  return WifiConnectResult::Timeout;
}

void WifiManager::disconnect() {
  WiFi.disconnect(true, false);
}

bool WifiManager::isConnected() {
  return WiFi.status() == WL_CONNECTED;
}

String WifiManager::localIp() {
  return WiFi.localIP().toString();
}

std::vector<WifiNetwork> WifiManager::scanNetworks() {
  std::vector<WifiNetwork> result;

  int count = WiFi.scanNetworks(/*async=*/false, /*show_hidden=*/false);
  if (count <= 0) {
    return result;
  }

  // Deduplicate by SSID, keeping the strongest RSSI seen for each.
  std::map<String, WifiNetwork> bySsid;
  for (int i = 0; i < count; i++) {
    String ssid = WiFi.SSID(i);
    if (ssid.length() == 0) continue;

    WifiNetwork net;
    net.ssid = ssid;
    net.rssi = WiFi.RSSI(i);
    net.secure = WiFi.encryptionType(i) != WIFI_AUTH_OPEN;

    auto it = bySsid.find(ssid);
    if (it == bySsid.end() || net.rssi > it->second.rssi) {
      bySsid[ssid] = net;
    }
  }

  WiFi.scanDelete();

  for (auto &pair : bySsid) {
    result.push_back(pair.second);
  }
  std::sort(result.begin(), result.end(), [](const WifiNetwork &a, const WifiNetwork &b) {
    return a.rssi > b.rssi;
  });

  return result;
}
