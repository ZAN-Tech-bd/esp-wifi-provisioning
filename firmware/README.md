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
- **NimBLE-Arduino** library v2.x (h2zero) — see "Why NimBLE" below

## Build & flash (Arduino IDE)

1. Boards Manager → install `esp32` by Espressif Systems.
2. Library Manager → install `ArduinoJson` and `NimBLE-Arduino`.
3. Open this folder's `.ino` file, pick your board + port, Upload.

No partition scheme changes, no board-specific settings — this fits the
default partition table on every supported chip (original ESP32 included).

## Build & flash (arduino-cli)

```bash
arduino-cli core install esp32:esp32
arduino-cli lib install ArduinoJson
arduino-cli lib install "NimBLE-Arduino"

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
lib_deps =
    bblanchon/ArduinoJson@^7
    h2zero/NimBLE-Arduino@^2
monitor_speed = 115200
```

## Why NimBLE (not the stock BLE library)

This firmware is meant to be **one part** of a bigger project — you drop it
into your own product's sketch alongside your own sensors, actuators,
MQTT/HTTP client, etc. That means flash headroom matters: the less this
module uses, the more room is left for everything else you add on top.

The ESP32 Arduino core ships a built-in BLE library (`BLEDevice.h`, backed
by Bluedroid) that needs no extra install, but on the **original ESP32**
chip it links in Bluedroid's full classic-Bluetooth-plus-BLE combo stack
even if you only call BLE APIs — that alone is roughly 400-500KB of flash
this firmware doesn't need. Combined with Wi-Fi and ArduinoJson, that
pushed a build past the default 1.2MB app partition entirely (127% used,
"text section exceeds available space") and would only get worse once you
add your own code on top.

[NimBLE-Arduino](https://github.com/h2zero/NimBLE-Arduino) is a from-scratch
BLE-only host stack with no classic Bluetooth code at all, on any chip. The
same firmware compiles to:

| Chip | Bluedroid (stock BLE lib) | NimBLE-Arduino |
|---|---|---|
| Original ESP32 | 1,665,059 bytes (127% — **doesn't fit**) | 1,153,072 bytes (87%) |
| ESP32-C3 | 1,253,889 bytes (95%) | smaller still (no classic BT on this chip either way) |

One `#include` swap (`NimBLEDevice.h` instead of `BLEDevice.h`/`BLEServer.h`/
etc.) and it fits the default partition scheme on every supported chip,
with real headroom left for your own application code — no special
Arduino IDE settings, no custom partition table, just Upload.

If you'd rather use the stock Bluedroid library anyway (e.g. you're already
depending on it elsewhere and don't want a second BLE stack in the build),
swapping back is a contained change — only `ble_provisioning.cpp`'s
`NimBLE*` types/calls need to change back to `BLE*`; the GATT layout and
JSON payloads stay identical. Just budget for the partition scheme change
on original ESP32 boards if you do.

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
`service->start()` has run.** Set the initial value with `setValue()`
instead (no client is connected yet, so there's nothing to notify), and
only call `notify()` after `service->start()` — that's what
`ble_provisioning.cpp` does in `begin()`. This matters more than it sounds:
on the stock Bluedroid BLE library, getting this order wrong asserts and
reboots the chip, and because it crashes again on every subsequent boot it
manifests as a silent boot loop where the device never advertises and never
shows up in the app, with no obvious error unless you happen to catch the
crash dump over serial (see the native-USB serial note in
[`docs/GETTING_STARTED.md`](../docs/GETTING_STARTED.md#confirm-its-alive)).

**`WiFi.macAddress()` can return `00:00:00:00:00:00` if called before the
Wi-Fi driver has actually started.** `WiFi.mode(WIFI_STA)` alone doesn't
start it — that happens lazily on the first `WiFi.begin()`/scan — and this
firmware goes straight to BLE provisioning without either when there are no
stored credentials, so the device would otherwise advertise as
`ZAN-Prov-0000` every time. `ble_provisioning.cpp` reads the factory MAC
directly via `esp_read_mac(mac, ESP_MAC_WIFI_STA)` (`<esp_mac.h>`) instead,
which needs no driver state at all — use that (not `WiFi.macAddress()`) for
anything that needs the MAC before Wi-Fi actually connects.

**`NimBLECharacteristic::getValue()` returns a `NimBLEAttValue`, not a
`String` or `std::string`.** Use `.c_str()` / `.length()` on it (see
`CommandCallbacks::onWrite` / `CredentialsCallbacks::onWrite`) rather than
assigning it directly to either string type.
