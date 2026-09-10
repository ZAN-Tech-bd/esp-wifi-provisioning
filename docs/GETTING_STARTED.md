# Getting Started

This walks through flashing the firmware, running the app, and provisioning
one ESP32 board end to end.

## 1. Flash the firmware

1. Install the **ESP32 board package** in Arduino IDE (File → Preferences →
   Additional Board Manager URLs →
   `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json`,
   then Tools → Board → Boards Manager → search "esp32" → install).
2. Install the **ArduinoJson** library (Tools → Manage Libraries → search
   "ArduinoJson" by Benoit Blanchon → install v7.x).
3. Open `firmware/esp-ble-wifi-provisioning/esp-ble-wifi-provisioning.ino`.
4. Select your board (Tools → Board → your ESP32 variant) and the correct
   port.
5. Upload.
6. Open the Serial Monitor at `115200` baud — on boot you'll see whether the
   device found stored credentials or dropped into provisioning mode, plus
   the exact BLE name it's advertising as (`ZAN-Prov-XXXX`).

The firmware works on any ESP32 (original, S2, S3, C3) that has both Wi-Fi
and BLE — see `firmware/README.md` for per-variant notes.

## 2. Run the app

No pre-built binary is shipped — the repo ships the app's source under
`app/`. Since this is a source template meant to be dropped into your own
project, you first scaffold the native Android/iOS shells with the Flutter
CLI, then copy in the code from this repo:

```bash
cd app
flutter create --org com.zantech --project-name zan_wifi_provisioning .
```

`flutter create` will not overwrite `lib/`, `pubspec.yaml`, or `README.md`
that already exist in this repo — it only fills in the missing `android/`,
`ios/`, `web/`, etc. folders. Then:

1. Add the permissions from [`app/README.md`](../app/README.md#platform-permissions)
   to the generated `android/app/src/main/AndroidManifest.xml` and
   `ios/Runner/Info.plist`.
2. `flutter pub get`
3. `flutter run` with a phone connected (BLE doesn't work in most emulators —
   use a real device).

## 3. Provision a device

1. Power the ESP32. If it has no saved Wi-Fi credentials it starts
   advertising as `ZAN-Prov-XXXX`.
2. Open the app → **Scan for devices** → tap your ESP32 in the list.
3. The app connects over BLE and requests a Wi-Fi scan; pick your network
   from the list, enter the password, tap **Connect**.
4. The app shows live status (`connecting` → `connected`) as the ESP32
   reports it. On success you'll see the IP address the device was assigned.
5. Disconnect the app — the ESP32 keeps the credentials in flash and will
   reconnect to that Wi-Fi network on every future boot without BLE.

## 4. Re-provisioning / forgetting a network

Two ways to clear stored credentials and go back into BLE provisioning mode:

- **From the app**: while connected to the device over BLE, use the
  "Forget network" action (sends `{"cmd":"reset"}`).
- **From the hardware**: hold the BOOT button (GPIO0 on most dev boards —
  configurable via `RESET_BUTTON_PIN` in `config.h`) for 3+ seconds. The
  device erases NVS and reboots into provisioning mode.

## 5. Where to go from here

- [`docs/BLE_PROTOCOL.md`](BLE_PROTOCOL.md) — full wire protocol if you're
  porting either side to a different platform/language.
- `firmware/esp-ble-wifi-provisioning/config.h` — every tunable constant
  (timeouts, pins, UUIDs) in one place.
- `app/lib/services/ble_provisioning_service.dart` — the entire BLE state
  machine on the app side, framework-agnostic from the UI.
