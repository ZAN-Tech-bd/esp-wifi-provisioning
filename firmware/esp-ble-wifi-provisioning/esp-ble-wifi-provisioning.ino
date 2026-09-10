/*
 * ZAN Tech - BLE Wi-Fi Provisioning (firmware)
 * ---------------------------------------------
 * Baseline template: boot, try stored Wi-Fi credentials, and if there are
 * none (or they fail) fall back to advertising over BLE so a companion app
 * can scan for networks and hand over new credentials.
 *
 * Protocol: docs/BLE_PROTOCOL.md
 * Tunables: config.h
 *
 * Made by ZAN Tech - https://github.com/ZAN-Tech/esp-ble-wifi-provisioning
 * Licensed under the MIT License.
 *
 * -------------------------------------------------------------------------
 * This is a baseline you are meant to build on: add your own application
 * logic where marked "YOUR CODE HERE" below, once `WiFi.status() ==
 * WL_CONNECTED` this sketch stops touching Wi-Fi/BLE and just calls your
 * loop code every iteration.
 * -------------------------------------------------------------------------
 */

#include "config.h"
#include "wifi_manager.h"
#include "ble_provisioning.h"

WifiManager wifiManager;

static bool provisioningActive = false;
static uint32_t resetButtonPressedAtMs = 0;

static void setStatusLed(bool on) {
  if (STATUS_LED_PIN < 0) return;
  digitalWrite(STATUS_LED_PIN, on ? HIGH : LOW);
}

// Long-press the reset button (default: BOOT / GPIO0) to erase stored
// credentials and reboot into provisioning mode, even if the app isn't
// around to send {"cmd":"reset"}.
static void checkResetButton() {
  bool pressed = digitalRead(RESET_BUTTON_PIN) == LOW;

  if (pressed && resetButtonPressedAtMs == 0) {
    resetButtonPressedAtMs = millis();
  } else if (!pressed) {
    resetButtonPressedAtMs = 0;
  } else if (millis() - resetButtonPressedAtMs > RESET_BUTTON_HOLD_MS) {
    Serial.println("[MAIN] reset button held - erasing credentials");
    wifiManager.clearCredentials();
    delay(200);
    ESP.restart();
  }
}

static void startProvisioning() {
  provisioningActive = true;
  bleProvisioning.begin(&wifiManager);
}

void setup() {
  Serial.begin(DEBUG_BAUD);
  delay(200);
  Serial.println();
  Serial.println("=== ZAN Tech BLE Wi-Fi Provisioning ===");
  Serial.printf("Firmware v%s\n", FIRMWARE_VERSION);

  if (RESET_BUTTON_PIN >= 0) {
    pinMode(RESET_BUTTON_PIN, INPUT_PULLUP);
  }
  if (STATUS_LED_PIN >= 0) {
    pinMode(STATUS_LED_PIN, OUTPUT);
    setStatusLed(false);
  }

  wifiManager.begin();

  if (wifiManager.hasStoredCredentials()) {
    Serial.println("[MAIN] stored credentials found, attempting connection...");
    WifiConnectResult result = wifiManager.connectStored(BOOT_CONNECT_TIMEOUT_MS);

    if (result == WifiConnectResult::Success) {
      Serial.printf("[MAIN] connected, IP: %s\n", wifiManager.localIp().c_str());
      setStatusLed(true);
    } else if (WIFI_RETRY_ON_FAIL) {
      Serial.println("[MAIN] stored credentials failed, starting BLE provisioning");
      startProvisioning();
    } else {
      Serial.println("[MAIN] stored credentials failed, WIFI_RETRY_ON_FAIL is false - halting");
    }
  } else {
    Serial.println("[MAIN] no stored credentials, starting BLE provisioning");
    startProvisioning();
  }
}

void loop() {
  checkResetButton();

  if (provisioningActive) {
    bleProvisioning.loop();

    if (wifiManager.isConnected()) {
      setStatusLed(true);
    }
    return;
  }

  // ---------------------------------------------------------------------
  // YOUR CODE HERE - the device is on Wi-Fi (either from a fresh
  // provisioning session above, or from stored credentials at boot).
  // This is where you'd add MQTT, HTTP polling, sensor publishing, etc.
  // for your actual product.
  // ---------------------------------------------------------------------
}
