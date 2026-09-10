# Getting Started

A complete, verified walkthrough: flash the firmware, install the app on a
phone, and provision one ESP32 board end to end. Every command below was
run against real hardware (an ESP32-C3) and a real Android phone while
building this repo — including the gotchas, which are called out inline
instead of hidden.

Two ways to flash are covered: **Arduino IDE** (easiest if you're new to
ESP32) and **arduino-cli** (faster, scriptable, what these steps were
actually tested with). Pick one.

---

## 0. What you need

- An ESP32 board with both Wi-Fi and BLE (original ESP32, S3, or C3 —
  **not** the S2, which has no Bluetooth radio).
- A USB cable that carries data (not charge-only).
- A phone (Android or iOS) — BLE does not work in most emulators/simulators,
  so a real device is required for the app.
- [Flutter SDK](https://docs.flutter.dev/get-started/install) installed and
  on your `PATH`.

---

## 1. Flash the firmware

### Option A — Arduino IDE

1. Install the **ESP32 board package**: File → Preferences → Additional
   Board Manager URLs →
   `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json`,
   then Tools → Board → Boards Manager → search "esp32" → install.
2. Install the **ArduinoJson** library: Tools → Manage Libraries → search
   "ArduinoJson" (by Benoit Blanchon) → install v7.x.
3. Open `firmware/esp-ble-wifi-provisioning/esp-ble-wifi-provisioning.ino`.
4. Tools → Board → pick your **exact chip** (see the callout below — don't
   pick a generic "Family Device" entry if one shows up).
5. Tools → Port → pick the port your board enumerated as.
6. Click Upload.

### Option B — arduino-cli

```bash
# One-time setup
arduino-cli core install esp32:esp32
arduino-cli lib install ArduinoJson

# Find your board's port and exact chip
arduino-cli board list
```

> **Don't compile against a generic "Family Device" board.** If
> `board list` reports something like `ESP32 Family Device` with an FQBN
> ending in `..._family`, that entry is a placeholder Arduino IDE uses for
> auto-detection — compiling directly against it fails with an
> `Invalid value for '--chip': '{build.mcu}'` error, because that FQBN has
> no chip target baked in. Find the *real* chip instead:
>
> ```bash
> esptool.py --port COM7 chip_id
> # -> "Detecting chip type... ESP32-C3"
> ```
>
> Then use the specific FQBN: `esp32:esp32:esp32c3` (or `esp32:esp32:esp32`,
> `esp32:esp32:esp32s3`, etc. — run `arduino-cli board listall esp32` to see
> every option).

```bash
cd firmware/esp-ble-wifi-provisioning
arduino-cli compile --fqbn esp32:esp32:esp32c3 .
arduino-cli upload --fqbn esp32:esp32:esp32c3 --port COM7 .
```

(Replace `esp32c3` and `COM7` with what you found above — on macOS/Linux
the port looks like `/dev/ttyUSB0` or `/dev/cu.usbserial-*`.)

### Confirm it's alive

The board should now be advertising over BLE as `ZAN-Prov-XXXX` (last 2
bytes of its MAC address). The most reliable way to confirm this is to
scan for it from the app (step 3) — but if you want a serial sanity check
first:

```bash
arduino-cli monitor -p COM7 -c baudrate=115200
```

> **Boards with native USB (ESP32-C3, -S3) can show an empty Serial
> Monitor right after a reset.** The chip's USB peripheral itself restarts
> on reset, and the very first `Serial.println()` calls happen before the
> host has finished re-enumerating the virtual COM port — so those early
> lines are silently dropped. This is a timing quirk of native USB-CDC, not
> a crash. If the monitor stays empty but the board still shows up in the
> app's scan, everything is working correctly.
>
> What genuinely indicates a problem is the board's port **repeatedly
> disappearing and reappearing every second or two** — that's a boot loop
> (a crash → panic handler → reboot → crash again cycle), and it means the
> firmware never reaches the BLE-advertising state. If you hit this after
> modifying `ble_provisioning.cpp`, check that no `notify()` call happens
> before `service->start()` — see the callout in
> [`firmware/README.md`](../firmware/README.md#common-pitfalls).

---

## 2. Set up the app

The repo ships the app's **Dart source only** — you scaffold the native
Android/iOS project shells once with the Flutter CLI, then this source
drops straight in:

```bash
cd app
flutter create --org com.zantech --project-name zan_wifi_provisioning .
```

`flutter create` fills in only the missing `android/`, `ios/`, `web/`, etc.
folders — it will not overwrite `lib/`, `pubspec.yaml`, or this repo's
`README.md`.

1. Add the permissions from
   [`app/README.md`](../app/README.md#platform-permissions) to the
   generated `android/app/src/main/AndroidManifest.xml` and
   `ios/Runner/Info.plist`.
2. `flutter pub get`

---

## 3. Install the app on your phone

Connect your phone via USB with USB debugging enabled (Android: Settings →
About phone → tap "Build number" 7 times → Developer options → USB
debugging; iOS: trust the computer when prompted, and you'll need Xcode
signing set up).

```bash
flutter devices        # confirm your phone shows up
flutter run            # builds, installs, launches, and hot-reloads
```

If you'd rather build once and install separately (e.g. scripting a CI
step, or `flutter run`'s interactive session isn't convenient):

```bash
flutter build apk --debug
adb install -r build/app/outputs/flutter-apk/app-debug.apk
adb shell am start -n com.zantech.zan_wifi_provisioning/.MainActivity
```

On first launch, grant the Bluetooth/location permission prompt — the app
needs it to scan for BLE devices at all. If you accidentally deny it, the
scan silently returns zero results; clear the app's permissions in Android
Settings → Apps and relaunch to get the prompt again.

---

## 4. Provision a device

1. Power the ESP32. If it has no saved Wi-Fi credentials it starts
   advertising as `ZAN-Prov-XXXX`.
2. Open the app → it scans automatically (tap the refresh icon to scan
   again) → tap your device in the list.
3. The app connects over BLE and requests a Wi-Fi scan; pick your network
   from the list, enter the password, tap **Connect**.
4. The app shows live status (`connecting` → `connected`) as the ESP32
   reports it. On success you'll see the IP address the device was assigned.
5. Tap **Done** — the ESP32 keeps the credentials in flash and will
   reconnect to that Wi-Fi network on every future boot without BLE.

### If the app shows "No devices found"

- Confirm the ESP32 is actually powered and past its boot sequence (give
  it 2-3 seconds after power-up).
- Confirm it doesn't already have working stored credentials — a device
  that connects to Wi-Fi successfully on boot skips BLE entirely by design
  (see [`docs/BLE_PROTOCOL.md`](BLE_PROTOCOL.md#4-state-machine-firmware)).
  Long-press the reset button (step 5) to force it back into provisioning
  mode.
- Confirm Bluetooth is actually on and the permission prompt was granted
  (previous section).
- Try moving the phone within a meter or two of the board — BLE range with
  default TX power is short.

---

## 5. Re-provisioning / forgetting a network

Two ways to clear stored credentials and go back into BLE provisioning mode:

- **From the app**: while connected to the device over BLE, use the
  "Forget device credentials" action (sends `{"cmd":"reset"}`).
- **From the hardware**: hold the BOOT button (GPIO0 on most dev boards —
  configurable via `RESET_BUTTON_PIN` in `config.h`) for 3+ seconds. The
  device erases NVS and reboots into provisioning mode.

---

## 6. Where to go from here

- [`docs/BLE_PROTOCOL.md`](BLE_PROTOCOL.md) — full wire protocol if you're
  porting either side to a different platform/language.
- `firmware/esp-ble-wifi-provisioning/config.h` — every tunable constant
  (timeouts, pins, UUIDs) in one place.
- `app/lib/services/ble_provisioning_service.dart` — the entire BLE state
  machine on the app side, framework-agnostic from the UI.
