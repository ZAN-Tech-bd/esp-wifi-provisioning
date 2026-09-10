/*
 * ZAN Tech - ESP32 Wi-Fi Setup (base template)
 * -----------------------------------------------
 * Lets anyone connect this device to their own Wi-Fi without ever touching
 * the code. How it works:
 *
 *   1. On boot, if a Wi-Fi name/password was saved before, try connecting
 *      to it.
 *   2. If that works: done, the device is on the network.
 *   3. If it doesn't (or nothing was saved yet): the device opens its own
 *      Wi-Fi hotspot instead (see hotspotName() below for the name).
 *   4. Connect a phone to that hotspot, open a browser, and go to the IP
 *      address printed on Serial (also shown as "Couldn't connect - setup
 *      hotspot started" below it). That loads a small form (see page.h) -
 *      enter the real Wi-Fi name and password there.
 *   5. Submitting the form saves it to flash and restarts the device,
 *      which then does step 1 again with the new details.
 *
 * The web page's HTML lives in page.h, not here, so this file stays
 * readable. Everything Wi-Fi/web-server related is handled below - your
 * own project code goes in ONE place, clearly marked near the bottom of
 * loop().
 *
 * Made by ZAN Tech - https://github.com/ZAN-Tech-bd/esp-wifi-provisioning
 * Licensed under the MIT License.
 */

#include <WiFi.h>
#include <WebServer.h>
#include <Preferences.h>
#include <esp_mac.h>

#include "page.h"

#define AP_SSID_PREFIX "ZAN-Setup-"     // hotspot name = prefix + last 2 bytes of the MAC
#define NVS_NAMESPACE "wifi-config"     // where credentials are saved in flash
#define WIFI_CONNECT_TIMEOUT_MS 10000   // how long to try the saved network before giving up

Preferences preferences;
WebServer server(80);

// Builds a hotspot name like "ZAN-Setup-8C38" from the chip's own MAC
// address, so more than one of these devices can be told apart if several
// are being set up near each other. Reads the MAC straight from the chip
// (not WiFi.macAddress()) because that call can return all-zeros this
// early in boot, before Wi-Fi has actually started.
String hotspotName() {
  uint8_t mac[6];
  esp_read_mac(mac, ESP_MAC_WIFI_STA);
  char suffix[5];
  snprintf(suffix, sizeof(suffix), "%02X%02X", mac[4], mac[5]);
  return String(AP_SSID_PREFIX) + suffix;
}

// GET / - shows the setup form (see page.h).
void handleRoot() {
  server.send(200, "text/html", SETUP_PAGE_HTML);
}

// POST /save - the form in page.h submits here. Saves whatever was typed
// in and restarts; setup() below then tries connecting with it on the
// next boot.
void handleSave() {
  String newSsid = server.arg("ssid");
  newSsid.trim();  // mobile keyboards love adding a trailing space
  String newPassword = server.arg("pass");

  if (newSsid.length() == 0) {
    server.send(200, "text/html", SETUP_PAGE_HTML);  // nothing to save, just re-show the form
    return;
  }

  preferences.begin(NVS_NAMESPACE, false);
  preferences.putString("ssid", newSsid);
  preferences.putString("pass", newPassword);
  preferences.end();

  server.send(200, "text/html", SAVED_PAGE_HTML);
  delay(2000);
  ESP.restart();
}

void setup() {
  Serial.begin(115200);

  // --- Step 1: load whatever Wi-Fi details were saved last time --------
  preferences.begin(NVS_NAMESPACE, true);
  String savedSsid = preferences.getString("ssid", "");
  String savedPassword = preferences.getString("pass", "");
  preferences.end();

  // --- Step 2: try connecting with them, if there were any -------------
  if (savedSsid != "") {
    WiFi.begin(savedSsid.c_str(), savedPassword.c_str());
    Serial.print("Connecting to saved Wi-Fi");
    for (int i = 0; i < WIFI_CONNECT_TIMEOUT_MS / 500; i++) {
      if (WiFi.status() == WL_CONNECTED) break;
      delay(500);
      Serial.print(".");
    }
    Serial.println();
  }

  // --- Step 3: connected, or not - act accordingly ----------------------
  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("Connected! IP: ");
    Serial.println(WiFi.localIP());
  } else {
    // No saved Wi-Fi, or it didn't work - open the setup hotspot instead.
    String apName = hotspotName();
    WiFi.softAP(apName.c_str());
    Serial.print("Couldn't connect - setup hotspot started: ");
    Serial.println(apName);
    Serial.print("Connect to it, then open a browser to: http://");
    Serial.println(WiFi.softAPIP());

    server.on("/", handleRoot);
    server.on("/save", HTTP_POST, handleSave);
    server.begin();
  }

  // -----------------------------------------------------------------------
  // YOUR ONE-TIME SETUP CODE GOES HERE (e.g. pinMode(), sensor init).
  // Runs whether or not Wi-Fi connected - guard anything that needs the
  // network with `if (WiFi.status() == WL_CONNECTED)`.
  // -----------------------------------------------------------------------
}

void loop() {
  server.handleClient();  // no-op if the setup hotspot was never started

  // -----------------------------------------------------------------------
  // YOUR MAIN CODE GOES HERE. This is where the rest of your project's
  // logic lives - reading sensors, publishing to MQTT, etc. Check
  // `WiFi.status() == WL_CONNECTED` first if it needs the network; this
  // runs continuously either way, whether the device is online or still
  // sitting in setup-hotspot mode.
  // -----------------------------------------------------------------------
}
