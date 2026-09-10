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

## Memory footprint

The Bluedroid BLE stack that ships with the standard `esp32` Arduino core is
used here for zero extra dependencies. If flash/RAM is tight on your board
(e.g. running alongside a large application), consider swapping to
[NimBLE-Arduino](https://github.com/h2zero/NimBLE-Arduino) — the GATT
layout and JSON payloads in `ble_provisioning.cpp` stay the same, only the
`BLEDevice`/`BLEServer`/`BLECharacteristic` calls need adapting to NimBLE's
API.
