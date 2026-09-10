# Firmware — `esp-wifi-provisioning`

A single Arduino sketch for any ESP32 (or ESP32-S2/S3/C3/C6/C2 — this uses
only Wi-Fi, no Bluetooth, so **every** ESP32 variant works, including the S2).

## Files

| File | Responsibility |
|---|---|
| `esp-wifi-provisioning.ino` | All the logic: load/save credentials, connect, fall back to a setup hotspot. Read this top to bottom — it's short. |
| `page.h` | The setup page's HTML. Kept separate so the `.ino` doesn't get cluttered with markup — this is the only file you need to edit to change how the page looks. |

## How it works

1. On boot, if a Wi-Fi name/password was saved before, it tries connecting
   to it (up to `WIFI_CONNECT_TIMEOUT_MS`).
2. If that works, it's just... connected. Nothing else happens.
3. If it doesn't (wrong password, network out of range, or nothing was ever
   saved), the device opens its own Wi-Fi hotspot named `ZAN-Setup-XXXX`
   (`XXXX` = last 2 bytes of its MAC address, so multiple units nearby don't
   collide).
4. Connect a phone to that hotspot, open a browser, and go to the IP
   address printed on Serial (`192.168.4.1` by default). That loads a small
   form — enter the real Wi-Fi name and password there.
5. Submitting the form saves it to flash and restarts the device, which
   repeats step 1 with the new details.

No app, no BLE, no serial cable needed after the initial flash — just a
phone's browser.

## Build & flash

Works in the Arduino IDE with **zero extra libraries** — `WiFi.h`,
`WebServer.h`, `Preferences.h`, and `esp_mac.h` all ship with the standard
`esp32` board package (Espressif).

1. Boards Manager → install `esp32` by Espressif Systems (if you haven't
   already).
2. Open `esp-wifi-provisioning.ino`, pick your board + port, Upload.

Or with `arduino-cli`:

```bash
arduino-cli core install esp32:esp32
cd firmware/esp-wifi-provisioning
arduino-cli compile --fqbn esp32:esp32:esp32c3 .
arduino-cli upload  --fqbn esp32:esp32:esp32c3 --port COM7 .
```

Swap `esp32c3` for whatever chip you actually have (`esp32`, `esp32s2`,
`esp32s3`, `esp32c6`, ...).

## Adding your own project logic

Two clearly marked spots in `esp-wifi-provisioning.ino`:

- Inside `setup()`, near the bottom — one-time init (`pinMode()`, sensor
  setup, etc.). Runs whether or not Wi-Fi connected.
- Inside `loop()`, near the bottom — your project's actual logic. Runs
  continuously regardless of connection state; check
  `WiFi.status() == WL_CONNECTED` first for anything that needs the network.

## Changing the setup page

See [`../README.md#what-it-looks-like`](../README.md#what-it-looks-like)
for a screenshot of the current form and confirmation pages.

Edit `page.h` — it's plain HTML in a raw string, nothing ESP-specific about
it. The form must keep posting to `/save` with fields named `ssid` and
`pass` (or update `handleSave()` in the `.ino` to match if you rename them).

## Changing the hotspot name, timeouts, or NVS keys

They're `#define`s at the top of `esp-wifi-provisioning.ino`
(`AP_SSID_PREFIX`, `WIFI_CONNECT_TIMEOUT_MS`, `NVS_NAMESPACE`).

## A known trade-off (by design)

There's no way to change the Wi-Fi network from a browser once the device
has already connected — the setup hotspot and web server only start when
`WiFi.status() != WL_CONNECTED`. To reconfigure a device that's already
online (moved to a new house, router replaced, etc.), just make sure it
*can't* reach the old network when it boots (e.g. take it to the new
location, or turn the old router off) — it'll fail to connect, fall back to
the setup hotspot automatically, and you re-enter new details from there.
This keeps the sketch to one simple path (connect, or don't and open the
hotspot) instead of also handling "connected but want to switch" as a third
state.

## Why this needs no external libraries

Every earlier version of this project used Bluetooth (first the stock
Bluedroid BLE library, then NimBLE-Arduino to shrink it) so a phone app
could hand over Wi-Fi credentials over BLE. Bluetooth is unnecessary
complexity for this job — a plain Wi-Fi hotspot + web form does the same
thing with a phone's ordinary browser, no app to install, and works on
every ESP32 variant (Wi-Fi-only chips like the S2 included, which couldn't
run the BLE-based version at all). This version is modeled directly on the
common "ESP32 Wi-Fi setup portal" pattern used across the Arduino/ESP32
community.
