# API Reference

Everything `ZanWifiSetup` exposes. There's one global instance — you never
construct it yourself, just call methods on `ZanWifiSetup` directly after
`#include <ZanWifiSetup.h>`.

## Core methods

### `ZanWifiSetup.begin()`

Call once, at the top of `setup()`.

- Loads any saved Wi-Fi credentials from flash.
- If there are any, tries connecting for up to `setConnectTimeoutMs()`
  (default 10 seconds).
- If that succeeds, returns with Wi-Fi connected — nothing else happens.
- If it fails (or nothing was saved), opens the setup hotspot and starts
  the web server. Returns immediately; the hotspot keeps running in the
  background via `loop()`.

Blocks for up to the connect timeout if there are saved credentials to try
— that's the only time this library blocks.

```cpp
void setup() {
  ZanWifiSetup.begin();
}
```

### `ZanWifiSetup.loop()`

Call on every iteration of `loop()`, unconditionally. Handles (in order):

1. The setup hotspot's web server, if it's running (near-instant no-op if
   the device is already connected and the hotspot was never opened).
2. The reset button, watching for a 3-second hold.
3. The status LED (solid/blinking).

Safe to call even while connected — steps 1 becomes a no-op, 2 and 3 still
run.

```cpp
void loop() {
  ZanWifiSetup.loop();
  // your code
}
```

### `ZanWifiSetup.isConnected()`

Returns `true` once connected to real Wi-Fi, `false` while the setup
hotspot is active. Equivalent to `WiFi.status() == WL_CONNECTED`, provided
as a small readability convenience. Use it to guard anything in your own
code that needs the network:

```cpp
void loop() {
  ZanWifiSetup.loop();
  if (ZanWifiSetup.isConnected()) {
    // safe to make HTTP requests, publish MQTT, etc.
  }
}
```

## Configuration methods

All optional. Call any of these **before** `begin()` — calling them after
has no effect, since `begin()` is what actually applies them (starting the
LED pin, reading the saved hotspot prefix, etc.).

### `ZanWifiSetup.setHotspotPrefix(const char *prefix)`

**Default:** `"ZAN-Setup-"`

The hotspot's advertised name is this prefix plus a 4-character suffix
derived from the device's own MAC address (e.g. `"ZAN-Setup-8C38"`), so
multiple units being set up near each other don't collide. Change the
prefix to brand it for your own product:

```cpp
ZanWifiSetup.setHotspotPrefix("MyThing-");
```

### `ZanWifiSetup.setStatusLedPin(int pin)`

**Default:** `LED_BUILTIN` (whatever pin your board's core defines as the
onboard LED — often GPIO2, often blue)

- **Solid on** — connected to Wi-Fi.
- **Blinking** (~2.5 times/second) — setup hotspot is active, waiting for
  configuration.

Pass `-1` to disable the LED entirely (e.g. you need that pin for your own
peripheral, or your board has no onboard LED and you haven't wired one up):

```cpp
ZanWifiSetup.setStatusLedPin(-1);   // disable
ZanWifiSetup.setStatusLedPin(4);    // use GPIO4 instead
```

### `ZanWifiSetup.setResetButtonPin(int pin)`

**Default:** `0` (GPIO0 — the BOOT button on essentially every ESP32 dev
board)

Held LOW continuously for 3 seconds, this erases the saved Wi-Fi
credentials and restarts the device straight into the setup hotspot. The
pin is configured `INPUT_PULLUP`, so a plain momentary switch to GND (or
the board's existing BOOT button) is all you need — no external resistor.

Pass `-1` to disable this feature entirely:

```cpp
ZanWifiSetup.setResetButtonPin(-1);  // disable
ZanWifiSetup.setResetButtonPin(14);  // use a dedicated button on GPIO14
```

### `ZanWifiSetup.setConnectTimeoutMs(uint32_t ms)`

**Default:** `10000` (10 seconds)

How long `begin()` waits for a saved network to connect before giving up
and opening the setup hotspot instead. Raise it if your router is slow to
hand out a DHCP lease; lower it if you'd rather fail fast.

```cpp
ZanWifiSetup.setConnectTimeoutMs(20000);  // give it 20 seconds
```

## What's stored, and where

Credentials are saved via the ESP32's [`Preferences`](https://docs.espressif.com/projects/arduino-esp32/en/latest/api/preferences.html)
API (NVS/flash), under the namespace `wifi-config`, keys `ssid` and `pass`.
They survive power loss and reflashing the sketch (NVS is a separate flash
region from the application) but are erased by the reset-button hold, by
calling `esptool.py erase_flash`, or by the ESP32 IDE's "Erase All Flash
Before Sketch Upload" option.

## The setup page's HTTP surface

For reference if you're customizing `src/zan_wifi_setup_page.h` or
building your own client against the hotspot:

| Route | Method | Purpose |
|---|---|---|
| `/` | `GET` | Returns the setup form (`ZAN_WIFI_SETUP_PAGE_HTML`). |
| `/save` | `POST` | Body fields `ssid` (required) and `pass` (optional). Saves to flash, responds with `ZAN_WIFI_SAVED_PAGE_HTML`, then restarts after ~1.5s. Empty `ssid` re-renders the form instead of saving. |

Any other path also returns the setup form (there's no dedicated 404 page)
— there's no captive-portal auto-popup, so a phone won't jump to the form
automatically; browsing to `192.168.4.1` is required.
