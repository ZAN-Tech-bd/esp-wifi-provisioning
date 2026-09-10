/*
 * ZAN Tech - BLE Wi-Fi Provisioning
 * config.h - every tunable constant lives here.
 *
 * https://github.com/ZAN-Tech/esp-ble-wifi-provisioning
 */
#pragma once

// ---------------------------------------------------------------------------
// Device identity
// ---------------------------------------------------------------------------
#define DEVICE_NAME_PREFIX      "ZAN-Prov-"   // advertised name = prefix + last 2 MAC bytes
#define FIRMWARE_VERSION        "1.0.0"
#define CHIP_LABEL              "ESP32"

// ---------------------------------------------------------------------------
// BLE UUIDs - ZAN Tech Wi-Fi Provisioning Protocol v1
// See docs/BLE_PROTOCOL.md. Do not change unless you also change the app.
// ---------------------------------------------------------------------------
#define SERVICE_UUID             "b8a10000-2d4c-4a1e-9f3a-000000000000"
#define CHAR_DEVICE_INFO_UUID    "b8a10001-2d4c-4a1e-9f3a-000000000000"
#define CHAR_COMMAND_UUID        "b8a10002-2d4c-4a1e-9f3a-000000000000"
#define CHAR_NETWORK_UUID        "b8a10003-2d4c-4a1e-9f3a-000000000000"
#define CHAR_CREDENTIALS_UUID    "b8a10004-2d4c-4a1e-9f3a-000000000000"
#define CHAR_STATUS_UUID         "b8a10005-2d4c-4a1e-9f3a-000000000000"

// ---------------------------------------------------------------------------
// Behaviour
// ---------------------------------------------------------------------------
#define WIFI_CONNECT_TIMEOUT_MS   15000   // how long to try WiFi.begin() before giving up
#define BOOT_CONNECT_TIMEOUT_MS   10000   // how long to try stored credentials on boot
#define PROVISIONING_TIMEOUT_MS   180000  // reboot if left idle in BLE mode this long (0 = never)
#define WIFI_RETRY_ON_FAIL        true    // if stored credentials fail on boot, still fall back to BLE

// Hold this pin LOW for RESET_BUTTON_HOLD_MS to erase stored credentials.
// GPIO0 is the BOOT button on most ESP32 dev boards.
#define RESET_BUTTON_PIN          0
#define RESET_BUTTON_HOLD_MS      3000

// NVS (flash) storage
#define NVS_NAMESPACE             "zan_wifi"
#define NVS_KEY_SSID              "ssid"
#define NVS_KEY_PASSWORD          "pass"

// Onboard LED feedback (set to -1 to disable)
#ifndef LED_BUILTIN
#define LED_BUILTIN               -1
#endif
#define STATUS_LED_PIN             LED_BUILTIN

// Serial debug output
#define DEBUG_BAUD                115200
