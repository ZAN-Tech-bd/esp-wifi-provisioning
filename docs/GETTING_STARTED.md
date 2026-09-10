# Getting Started

Flash the firmware, connect a device to your Wi-Fi, done. No app to
install, no BLE, no serial cable needed after the first flash.

---

## 0. What you need

- Any ESP32 board (original ESP32, S2, S3, C3, C6, C2 — this only uses
  Wi-Fi, so every variant works).
- A USB cable that carries data (not charge-only).
- A phone or laptop with Wi-Fi, to do the setup.

---

## 1. Flash the firmware

### Arduino IDE

1. Boards Manager → install `esp32` by Espressif Systems (if you haven't
   already).
2. Open `firmware/esp-wifi-provisioning/esp-wifi-provisioning.ino`.
3. Tools → Board → pick your exact chip; Tools → Port → pick its port.
4. Click Upload.

No extra libraries to install — everything this sketch uses ships with the
`esp32` board package.

### arduino-cli

```bash
arduino-cli core install esp32:esp32

arduino-cli board list          # find your port + exact chip
cd firmware/esp-wifi-provisioning
arduino-cli compile --fqbn esp32:esp32:esp32c3 .
arduino-cli upload  --fqbn esp32:esp32:esp32c3 --port COM7 .
```

Swap `esp32c3` for your actual chip (`esp32`, `esp32s2`, `esp32s3`,
`esp32c6`, ...) and `COM7` for your port (`/dev/ttyUSB0` on Linux,
`/dev/cu.usbserial-*` on macOS).

> **Board shows up as a generic "Family Device"?** That's an
> auto-detection placeholder, not a real target — compiling against it
> fails. Run `esptool.py --port COM7 chip_id` to find the actual chip, then
> use its specific FQBN instead.

---

## 2. Connect it to your Wi-Fi

1. Power the board. Open the Serial Monitor at `115200` baud if you want to
   watch this happen — otherwise just wait a few seconds.
2. It has no saved Wi-Fi yet, so it opens its own hotspot named
   **`ZAN-Setup-XXXX`** (`XXXX` = last 2 bytes of its MAC address).
3. On your phone or laptop, connect to that hotspot like any other Wi-Fi
   network (no password by default).
4. Open a browser and go to **`http://192.168.4.1/`**.
5. Enter your real Wi-Fi name and password, tap Connect.
6. The device saves it and restarts. It reconnects to your Wi-Fi
   automatically from now on, every time it boots.

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

## 3. Reconfiguring later (new Wi-Fi network)

This version keeps things to one simple rule: **the setup hotspot only
opens when the device can't connect.** So to change networks:

1. Take the device somewhere its current saved network is out of range (or
   turn that router off), then power-cycle it.
2. It'll fail to connect, fall back to the `ZAN-Setup-XXXX` hotspot
   automatically, and you repeat step 2 above with the new network.

There's no way to change Wi-Fi from a browser while it's still
successfully connected — see
[`firmware/README.md`](../firmware/README.md#a-known-trade-off-by-design)
for why that's a deliberate simplification, not an oversight.

---

## 4. Where to go from here

- [`firmware/README.md`](../firmware/README.md) — full file-by-file
  breakdown and how the sketch is organized.
- `firmware/esp-wifi-provisioning/esp-wifi-provisioning.ino` — the whole
  sketch; look for the two `YOUR ... CODE GOES HERE` comments to see where
  to add your own project's logic.
- `firmware/esp-wifi-provisioning/page.h` — the setup page's HTML, if you
  want to restyle it.
