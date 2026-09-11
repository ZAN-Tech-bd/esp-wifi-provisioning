# Troubleshooting

## "It won't connect to my Wi-Fi" — check this first

**ESP32 only supports 2.4GHz Wi-Fi, not 5GHz.** This is, by far, the most
common reason a network name typed correctly into the setup form still
never connects. Most home routers broadcast both bands — sometimes under
the *same* network name, sometimes as two separate names (e.g. `Home-WiFi`
and `Home-WiFi-5G`). If you typed the 5GHz one, the ESP32 will sit there
retrying and eventually fall back to the setup hotspot, with no more
specific error than "timeout."

Fix: check your router's admin page for a 2.4GHz-band SSID, or look for a
second network name in your phone's Wi-Fi list — that's almost always the
2.4GHz one. Type that one into the form instead.

## Compiling

### "Family Device" / `Invalid value for '--chip': '{build.mcu}'`

You picked (or arduino-cli auto-selected) a generic placeholder board
entry instead of your actual chip. Run:

```bash
esptool.py --port COM7 chip_id
```

to find the real chip, then pick that specific board in Arduino IDE
(Tools → Board), or use its specific FQBN with arduino-cli
(`esp32:esp32:esp32c3`, `esp32:esp32:esp32s3`, etc. — run
`arduino-cli board listall esp32` to see every option).

### `fatal error: ZanWifiSetup.h: No such file or directory`

The library isn't installed where your sketch can find it. Either:

- Arduino IDE: Sketch → Include Library → Add .ZIP Library... and select
  this repo's folder (or a .ZIP of it), then restart the IDE if it still
  doesn't show up.
- arduino-cli: add `--library /path/to/esp-wifi-provisioning` to your
  `compile`/`upload` command.

See [`GETTING_STARTED.md`](GETTING_STARTED.md#1-install-the-library-one-time-ever)
for the full install steps.

### `text section exceeds available space in board` / sketch too big

This library alone uses well under the default partition size on every
supported chip (71-77% on the tightest ones, see the README). If you hit
this, it's your own added code pushing the total over — check Tools →
Partition Scheme in Arduino IDE and pick one with a bigger app partition
(e.g. "Minimum SPIFFS" or "Huge APP"), or trim what you've added.

### Random compile errors that don't match the code at all

Usually a stale build cache, especially after switching boards/FQBNs
repeatedly. Close Arduino IDE completely, delete the cache folder
(`%LOCALAPPDATA%\arduino\sketches\` on Windows), reopen, and rebuild.

## Uploading

### `Could not open port` / `port is busy or doesn't exist`

- Confirm the board is actually connected and shows up (Windows: Device
  Manager → Ports; `arduino-cli board list`).
- Close the Serial Monitor and any other program (including a second
  Arduino IDE window) that might be holding the port open.
- Some boards need you to hold BOOT while upload starts, especially the
  very first time or after a deep sleep.

## The setup hotspot

### It never shows up

- Give it a few seconds after power-up.
- If this device connected to Wi-Fi successfully before, it won't open the
  hotspot again — that's by design (see below). Hold the reset button for
  3 seconds to force it back into setup mode.
- Confirm your `setStatusLedPin()` (if you changed it) isn't wired to
  something that's actually failing silently — check the Serial Monitor at
  115200 baud for `[ZanWifiSetup]` log lines to see what it's actually
  doing.

### The page won't load at 192.168.4.1

- Make sure your phone actually joined the `ZAN-Setup-XXXX` network —
  phones sometimes auto-reconnect to a stronger known network in the
  background. Check your Wi-Fi settings to confirm which network you're on.
- There's no captive-portal auto-popup in this version, so nothing will
  prompt you automatically — you have to type the address in yourself.
- Try `http://192.168.4.1/` (with the trailing slash and `http://`, not
  `https://`) if your browser is being clever about auto-completing it.

### I submitted the form but nothing happens / it just hangs

The device is trying to connect and will restart in a couple of seconds
either way (success or failure) — this isn't instant. If it never
restarts, check Serial for what's actually happening; a full USB power
cut mid-write is about the only way this gets stuck, and power-cycling
fixes it.

## After connecting

### The LED doesn't do anything

- Some boards don't define `LED_BUILTIN` at all, or it's wired to a pin
  used for something else (SPI flash, USB) — check your board's pinout.
  Call `ZanWifiSetup.setStatusLedPin(<pin>)` with a known-good GPIO before
  `begin()`.
- If you intentionally disabled it (`setStatusLedPin(-1)`), that's
  expected — nothing to fix.

### The reset button doesn't do anything

- Make sure you're holding it for the full 3 seconds, continuously.
- Confirm you're pressing the actual pin `ZanWifiSetup` is watching —
  default is GPIO0 (BOOT), but if you called `setResetButtonPin()` with a
  different pin, use that one, and make sure it's wired to GND when
  pressed (it's configured `INPUT_PULLUP`, so no external resistor needed).
- If you called `setResetButtonPin(-1)`, this feature is disabled —
  expected.

### I want to change Wi-Fi networks but it's still connected fine

There's no in-browser way to do this while already connected — hold the
reset button for 3 seconds to force the setup hotspot back open, then
reconfigure from there. See
[`GETTING_STARTED.md`](GETTING_STARTED.md#4-reconfiguring-later-new-wi-fi-network-or-moved-somewhere-new).

## Still stuck?

Open an issue with your board (e.g. ESP32-DevKitC, ESP32-S3), the exact
error or symptom, and the Serial Monitor log (115200 baud) from power-on —
see [`CONTRIBUTING.md`](../CONTRIBUTING.md).
