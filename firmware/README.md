# Firmware — `esp-ble-wifi-provisioning`

Arduino sketch for any ESP32 variant with both Wi-Fi and BLE (original
ESP32, S3, C3 — **not** the S2, which has no Bluetooth radio).

## Files

| File | Responsibility |
|---|---|
| `esp-ble-wifi-provisioning.ino` | `setup()`/`loop()`, boot-time connect attempt, reset button, hand-off point for your own app code |
| `config.h` | Every constant: UUIDs, timeouts, pins, NVS keys |
| `wifi_manager.h/.cpp` | NVS-backed credential storage, connect, scan — no BLE knowledge, reusable on its own |
| `ble_provisioning.h/.cpp` | GATT server + state machine implementing [`docs/BLE_PROTOCOL.md`](../docs/BLE_PROTOCOL.md) |

## Requirements

- Arduino IDE 2.x (or `arduino-cli` / PlatformIO if you prefer — see below)
- **esp32** board package (Espressif) — tested against core 3.x
- **ArduinoJson** library v7.x (Benoit Blanchon)

## Build & flash (Arduino IDE)

1. Boards Manager → install `esp32` by Espressif Systems.
2. Library Manager → install `ArduinoJson`.
3. Open this folder's `.ino` file, pick your board + port, Upload.

## Build & flash (arduino-cli)

```bash
arduino-cli core install esp32:esp32
arduino-cli lib install ArduinoJson

arduino-cli board list          # find your port + exact chip
cd firmware/esp-ble-wifi-provisioning
arduino-cli compile --fqbn esp32:esp32:esp32c3 .
arduino-cli upload  --fqbn esp32:esp32:esp32c3 --port COM7 .
```

Use the specific chip FQBN (`esp32:esp32:esp32c3`, `esp32:esp32:esp32s3`,
`esp32:esp32:esp32`, ...) — see the callout in
[`docs/GETTING_STARTED.md`](../docs/GETTING_STARTED.md#option-b--arduino-cli)
if `board list` only reports a generic "Family Device" entry.

## Build & flash (PlatformIO)

No `platformio.ini` is committed (this repo targets Arduino IDE users by
default), but PlatformIO works fine with this exact source tree. Minimal
`platformio.ini` to drop next to the `.ino`:

```ini
[env:esp32dev]
platform = espressif32
board = esp32dev
framework = arduino
lib_deps = bblanchon/ArduinoJson@^7
monitor_speed = 115200
```

## Adding your own product logic

The `.ino` file's `loop()` has a marked block (`YOUR CODE HERE`) that only
runs once Wi-Fi is actually connected — provisioning has already happened
by then, so you can safely start MQTT clients, HTTP polling, sensor
publishing, etc. there without worrying about BLE/Wi-Fi state at all.

## Changing pins / timeouts / UUIDs

Everything tunable lives in `config.h` — nothing else in this folder should
need editing for basic customization (device name prefix, button pin,
timeouts). Only touch the UUIDs in `config.h` if you're also updating the
app side to match (see [`docs/BLE_PROTOCOL.md`](../docs/BLE_PROTOCOL.md)).

## Common pitfalls

**Never call `notify()` on a characteristic before its service's
`service->start()` has run.** The Bluedroid BLE library asserts
(`getService() != nullptr`) and reboots the chip if you do — and because it
crashes again on every subsequent boot, this manifests as a silent boot
loop where the device never advertises and never shows up in the app, with
no obvious error unless you happen to catch the crash dump over serial
(see the native-USB serial note in
[`docs/GETTING_STARTED.md`](../docs/GETTING_STARTED.md#confirm-its-alive)).
`ble_provisioning.cpp` sets the initial Status value with `setValue()`
(no client is connected yet, so there's nothing to notify) and only calls
`notify()` after `service->start()` — keep that ordering if you touch
`begin()`.

**`BLECharacteristic::getValue()`'s return type depends on your esp32 core
version.** Older core releases return `std::string`; core 3.x (tested:
3.3.11) returns Arduino `String`. `ble_provisioning.cpp` uses `String` — if
you're on an older core and see a "conversion from 'String' to non-scalar
type 'std::string'" compile error, swap the type in
`CommandCallbacks::onWrite` / `CredentialsCallbacks::onWrite` accordingly.

## Memory footprint

The Bluedroid BLE stack that ships with the standard `esp32` Arduino core is
used here for zero extra dependencies. If flash/RAM is tight on your board
(e.g. running alongside a large application), consider swapping to
[NimBLE-Arduino](https://github.com/h2zero/NimBLE-Arduino) — the GATT
layout and JSON payloads in `ble_provisioning.cpp` stay the same, only the
`BLEDevice`/`BLEServer`/`BLECharacteristic` calls need adapting to NimBLE's
API.
