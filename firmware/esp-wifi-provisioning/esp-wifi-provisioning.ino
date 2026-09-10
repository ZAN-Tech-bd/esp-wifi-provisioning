/*
 * ZAN Tech - ESP32 Wi-Fi Setup (base template)
 * -----------------------------------------------
 * Lets anyone connect this device to their own Wi-Fi without ever touching
 * the code: on boot, it tries the last Wi-Fi network it was told about. If
 * that fails (or it's never been set up before), it opens its own hotspot.
 * Connect a phone to that hotspot, browse to the shown IP address, and
 * enter the real Wi-Fi name and password - the device saves it and
 * restarts straight onto that network.
 *
 * Made by ZAN Tech - https://github.com/ZAN-Tech-bd/esp-ble-wifi-provisioning
 * Licensed under the MIT License.
 *
 * Add your own project's code in loop() below, guarded by a
 * WiFi.status() == WL_CONNECTED check.
 */

#include <WiFi.h>
#include <WebServer.h>
#include <Preferences.h>
#include <esp_mac.h>

#define AP_SSID_PREFIX "ZAN-Setup-"     // hotspot name = prefix + last 2 bytes of the MAC
#define NVS_NAMESPACE "wifi-config"
#define WIFI_CONNECT_TIMEOUT_MS 10000   // how long to try the saved network before giving up

Preferences preferences;
WebServer server(80);

String ssid = "";
String password = "";

// Unique-ish hotspot name so more than one of these devices can be told
// apart if several are being set up near each other.
String hotspotName() {
  uint8_t mac[6];
  esp_read_mac(mac, ESP_MAC_WIFI_STA);
  char suffix[5];
  snprintf(suffix, sizeof(suffix), "%02X%02X", mac[4], mac[5]);
  return String(AP_SSID_PREFIX) + suffix;
}

void handleRoot() {
  String html = "<h2>Wi-Fi Setup</h2>"
                "<form action='/save' method='POST'>"
                "<input name='ssid' placeholder='Wi-Fi name'><br>"
                "<input name='pass' type='password' placeholder='Wi-Fi password'><br>"
                "<input type='submit' value='Connect'>"
                "</form>";
  server.send(200, "text/html", html);
}

void handleSave() {
  ssid = server.arg("ssid");
  password = server.arg("pass");

  preferences.begin(NVS_NAMESPACE, false);
  preferences.putString("ssid", ssid);
  preferences.putString("pass", password);
  preferences.end();

  server.send(200, "text/html", "<h3>Saved! Restarting...</h3>");
  delay(2000);
  ESP.restart();
}

void setup() {
  Serial.begin(115200);

  preferences.begin(NVS_NAMESPACE, true);
  ssid = preferences.getString("ssid", "");
  password = preferences.getString("pass", "");
  preferences.end();

  if (ssid != "") {
    WiFi.begin(ssid.c_str(), password.c_str());
    Serial.print("Connecting to saved Wi-Fi");
    for (int i = 0; i < WIFI_CONNECT_TIMEOUT_MS / 500; i++) {
      if (WiFi.status() == WL_CONNECTED) break;
      delay(500);
      Serial.print(".");
    }
    Serial.println();
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("Connected! IP: ");
    Serial.println(WiFi.localIP());
  } else {
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
}

void loop() {
  server.handleClient();

  // YOUR CODE HERE - runs all the time; check `WiFi.status() ==
  // WL_CONNECTED` first if what you're doing needs the network.
}
