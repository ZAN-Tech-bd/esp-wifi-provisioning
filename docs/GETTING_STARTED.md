# Getting Started

Install the library once, flash the setup example once, connect a device
to your Wi-Fi. Every project after that just includes the same library —
no app, no BLE, no re-uploading Wi-Fi code ever again.

---

## 0. What you need

- Any ESP32 board (original ESP32, S2, S3, C3, C6, C2 — this only uses
  Wi-Fi, so every variant works).
- A USB cable that carries data (not charge-only).
- A phone or laptop with Wi-Fi, to do the setup.

---

## 1. Install the library (one time, ever)

### Arduino IDE

1. Boards Manager → install `esp32` by Espressif Systems (if you haven't
   already).
2. Download this repo (or clone it) as a folder named `ZanWifiSetup` (or
   just use it as-is — the folder name doesn't have to match once it's a
   .ZIP; Arduino reads the name from `library.properties`).
3. Sketch → Include Library → **Add .ZIP Library...** and select it. Or
   copy the whole folder directly into your `Arduino/libraries/` directory
   and restart the IDE.

That's it — every sketch from now on can `#include <ZanWifiSetup.h>`.

### arduino-cli

```bash
arduino-cli core install esp32:esp32
```

Nothing to install for the library itself — point `--library` at wherever
you put this repo for every compile/upload below instead:

```bash
arduino-cli compile --fqbn esp32:esp32:esp32c3 --library /path/to/esp-wifi-provisioning examples/WifiOnly
```

---

## 2. Flash the setup example once

### Arduino IDE

1. File → Examples → ZanWifiSetup → **WifiOnly**.
2. Tools → Board → pick your exact chip; Tools → Port → pick its port.
3. Click Upload.

The entire sketch is:

```cpp
#include <ZanWifiSetup.h>

void setup() { ZanWifiSetup.begin(); }
void loop()  { ZanWifiSetup.loop(); }
```

### arduino-cli

Run these from inside this repo's root folder (`--library .` refers to it):

```bash
arduino-cli board list          # find your port + exact chip
arduino-cli compile --fqbn esp32:esp32:esp32c3 --library . examples/WifiOnly
arduino-cli upload  --fqbn esp32:esp32:esp32c3 --library . --port COM7 examples/WifiOnly
```

Swap `esp32c3` for your actual chip (`esp32`, `esp32s2`, `esp32s3`,
`esp32c6`, ...) and `COM7` for your port (`/dev/ttyUSB0` on Linux,
`/dev/cu.usbserial-*` on macOS).

> **Board shows up as a generic "Family Device"?** That's an
> auto-detection placeholder, not a real target — compiling against it
> fails. Run `esptool.py --port COM7 chip_id` to find the actual chip, then
> use its specific FQBN instead.

---

## 3. Connect it to your Wi-Fi

1. Power the board. Open the Serial Monitor at `115200` baud if you want to
   watch this happen — otherwise just watch the onboard LED: **blinking**
   means it's waiting for setup.
2. It has no saved Wi-Fi yet, so it opens its own hotspot named
   **`ZAN-Setup-XXXX`** (`XXXX` = last 2 bytes of its MAC address).
3. On your phone or laptop, connect to that hotspot like any other Wi-Fi
   network (no password by default).
4. Open a browser and go to **`http://192.168.4.1/`**.
5. Enter your real Wi-Fi name and password, tap Connect.
6. The device saves it and restarts. The LED goes **solid** once it's
   connected, and it reconnects automatically from now on, every boot.

### If the hotspot doesn't show up

- Give it a few seconds after power-up.
- Confirm it doesn't already have working Wi-Fi saved from a previous test
  — a device that connects successfully never opens the hotspot (see
  "reconfiguring" below).
- Try a fresh chip or erase flash once (`esptool.py --port COM7
  erase_flash`) if you're testing repeatedly and want a truly clean slate.

### If the page won't load at 192.168.4.1

- Make sure your phone/laptop actually joined the `ZAN-Setup-XXXX` network
  (some phones auto-reconnect to a stronger known network in the
  background — check your Wi-Fi settings).
- There's no captive-portal auto-popup in this version — you have to type
  the address into the browser yourself.

---

## 4. Reconfiguring later (new Wi-Fi network, or moved somewhere new)

Two ways:

- **Hold the reset button** (BOOT / GPIO0 by default) for **3 seconds**.
  This erases the saved Wi-Fi and reopens the setup hotspot immediately —
  the LED starts blinking again. Use this any time you move a device to a
  new place.
- **Do nothing and just relocate it.** If the saved network genuinely can't
  be reached (out of range, router replaced), the device fails to connect
  on its own and falls back to the hotspot automatically on the next boot.

There's no way to change Wi-Fi from a browser while the device is still
successfully connected — only the reset button or a failed connection
attempt opens the setup hotspot. See
[`src/ZanWifiSetup.h`](../src/ZanWifiSetup.h) if you want to change this
behavior (e.g. `setResetButtonPin()` to move it to a different pin, or
`-1` to disable it entirely).

---

## 5. Building your actual project

**Don't add to the `WifiOnly` example.** Start a **new sketch** and
include the same library there too:

```cpp
#include <ZanWifiSetup.h>

void setup() {
  ZanWifiSetup.begin();
  // your one-time setup code (pinMode(), sensor init, etc.)
}

void loop() {
  ZanWifiSetup.loop();
  // your project code - check ZanWifiSetup.isConnected() first if it
  // needs the network
}
```

See `examples/BlinkWhileConnected` for a working version of this pattern
(blinks an LED once a second, but only once connected). This is the whole
point of it being a library instead of a sketch: the Wi-Fi part is done
once, and every future project — or every new feature in this one — just
builds around it without ever touching `src/ZanWifiSetup.h`/`.cpp` again.

## 6. Configuration reference

Call any of these **before** `ZanWifiSetup.begin()` to change a default:

| Method | Default | Purpose |
|---|---|---|
| `setHotspotPrefix(const char*)` | `"ZAN-Setup-"` | Hotspot name prefix (MAC suffix is appended automatically) |
| `setStatusLedPin(int)` | `LED_BUILTIN` | `-1` disables the LED entirely |
| `setResetButtonPin(int)` | `0` (BOOT button) | `-1` disables the reset button entirely |
| `setConnectTimeoutMs(uint32_t)` | `10000` | How long to try the saved network before opening the hotspot |

Full documentation is inline in [`src/ZanWifiSetup.h`](../src/ZanWifiSetup.h).

---

## 7. Where to go from here

- [`src/ZanWifiSetup.h`](../src/ZanWifiSetup.h) — the entire public API,
  documented inline.
- `src/zan_wifi_setup_page.h` — the setup page's HTML, if you want to
  restyle it (see [`../README.md#what-it-looks-like`](../README.md#what-it-looks-like)
  for a screenshot of the current design).
- `examples/WifiOnly` — the minimal sketch, good as a starting point for
  any new project.
- `examples/BlinkWhileConnected` — shows the pattern for adding your own
  logic alongside the library.
