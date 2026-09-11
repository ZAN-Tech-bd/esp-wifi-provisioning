#include "ZanWifiSetup.h"
#include "zan_wifi_setup_page.h"

#include <WiFi.h>
#include <Preferences.h>
#include <esp_mac.h>

#ifndef LED_BUILTIN
#define LED_BUILTIN -1
#endif

namespace {
constexpr const char *kNvsNamespace = "wifi-config";
constexpr uint32_t kResetHoldMs = 3000;
constexpr uint32_t kBlinkIntervalMs = 400;
}  // namespace

ZanWifiSetupClass ZanWifiSetup;

void ZanWifiSetupClass::setHotspotPrefix(const char *prefix) { hotspotPrefix_ = prefix; }
void ZanWifiSetupClass::setStatusLedPin(int pin) { statusLedPin_ = pin; }
void ZanWifiSetupClass::setResetButtonPin(int pin) { resetButtonPin_ = pin; }
void ZanWifiSetupClass::setConnectTimeoutMs(uint32_t ms) { connectTimeoutMs_ = ms; }

bool ZanWifiSetupClass::isConnected() { return WiFi.status() == WL_CONNECTED; }

String ZanWifiSetupClass::hotspotName() const {
  uint8_t mac[6];
  esp_read_mac(mac, ESP_MAC_WIFI_STA);
  char suffix[5];
  snprintf(suffix, sizeof(suffix), "%02X%02X", mac[4], mac[5]);
  return String(hotspotPrefix_) + suffix;
}

void ZanWifiSetupClass::begin() {
  Serial.begin(115200);  // harmless to call again if your sketch already did

  if (statusLedPin_ == -2) {
    statusLedPin_ = LED_BUILTIN;  // resolve the "use the board default" sentinel
  }
  if (statusLedPin_ >= 0) {
    pinMode(statusLedPin_, OUTPUT);
    digitalWrite(statusLedPin_, LOW);
  }
  if (resetButtonPin_ >= 0) {
    pinMode(resetButtonPin_, INPUT_PULLUP);
  }

  Preferences preferences;
  preferences.begin(kNvsNamespace, true);
  String savedSsid = preferences.getString("ssid", "");
  String savedPassword = preferences.getString("pass", "");
  preferences.end();

  if (savedSsid != "") {
    WiFi.begin(savedSsid.c_str(), savedPassword.c_str());
    Serial.print("[ZanWifiSetup] connecting to saved Wi-Fi");
    for (uint32_t i = 0; i < connectTimeoutMs_ / 500; i++) {
      if (WiFi.status() == WL_CONNECTED) break;
      delay(500);
      Serial.print(".");
    }
    Serial.println();
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("[ZanWifiSetup] connected! IP: ");
    Serial.println(WiFi.localIP());
  } else {
    apMode_ = true;
    startAccessPoint();
    server_.on("/", HTTP_GET, [this]() { handleRoot(); });
    server_.on("/save", HTTP_POST, [this]() { handleSave(); });
    server_.onNotFound([this]() { handleNotFound(); });
    server_.begin();
  }
}

void ZanWifiSetupClass::startAccessPoint() {
  String apName = hotspotName();
  WiFi.softAP(apName.c_str());
  Serial.print("[ZanWifiSetup] couldn't connect - setup hotspot started: ");
  Serial.println(apName);
  Serial.print("[ZanWifiSetup] connect to it, then open a browser to: http://");
  Serial.println(WiFi.softAPIP());
}

void ZanWifiSetupClass::handleRoot() {
  server_.send(200, "text/html", ZAN_WIFI_SETUP_PAGE_HTML);
}

void ZanWifiSetupClass::handleNotFound() {
  // No dedicated 404 - bounce anything else (e.g. a phone requesting
  // /favicon.ico, or someone typing a wrong path) back to the setup form.
  server_.send(200, "text/html", ZAN_WIFI_SETUP_PAGE_HTML);
}

void ZanWifiSetupClass::handleSave() {
  String newSsid = server_.arg("ssid");
  newSsid.trim();  // mobile keyboards love adding a trailing space
  String newPassword = server_.arg("pass");

  if (newSsid.length() == 0) {
    server_.send(200, "text/html", ZAN_WIFI_SETUP_PAGE_HTML);  // nothing to save, just re-show the form
    return;
  }

  Preferences preferences;
  preferences.begin(kNvsNamespace, false);
  preferences.putString("ssid", newSsid);
  preferences.putString("pass", newPassword);
  preferences.end();

  server_.send(200, "text/html", ZAN_WIFI_SAVED_PAGE_HTML);
  server_.client().flush();
  delay(1500);
  ESP.restart();
}

void ZanWifiSetupClass::checkResetButton() {
  if (resetButtonPin_ < 0) return;

  bool pressed = digitalRead(resetButtonPin_) == LOW;
  if (pressed && resetPressedAtMs_ == 0) {
    resetPressedAtMs_ = millis();
  } else if (!pressed) {
    resetPressedAtMs_ = 0;
  } else if (millis() - resetPressedAtMs_ > kResetHoldMs) {
    Serial.println("[ZanWifiSetup] reset button held - erasing saved Wi-Fi");
    Preferences preferences;
    preferences.begin(kNvsNamespace, false);
    preferences.clear();
    preferences.end();
    delay(200);
    ESP.restart();
  }
}

void ZanWifiSetupClass::updateStatusLed() {
  if (statusLedPin_ < 0) return;

  if (isConnected()) {
    digitalWrite(statusLedPin_, HIGH);  // solid on = connected
    return;
  }

  uint32_t now = millis();  // blinking = waiting for setup (hotspot mode)
  if (now - lastBlinkMs_ >= kBlinkIntervalMs) {
    lastBlinkMs_ = now;
    ledState_ = !ledState_;
    digitalWrite(statusLedPin_, ledState_ ? HIGH : LOW);
  }
}

void ZanWifiSetupClass::loop() {
  if (apMode_) {
    server_.handleClient();
  }
  checkResetButton();
  updateStatusLed();
}
