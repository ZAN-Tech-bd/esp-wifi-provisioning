# Contributing

Thanks for considering a contribution to this project — issues, pull
requests, and forks are all welcome.

## Ways to help

- **Bug reports** — open an issue with your board (e.g. ESP32-DevKitC,
  ESP32-S3), Arduino core version, phone OS/version, and (if firmware-side)
  the Serial Monitor log around the failure.
- **Protocol ports** — if you implement `docs/BLE_PROTOCOL.md` for another
  platform (native iOS/Android, web Bluetooth, a different MCU), we'd love
  a link from the README.
- **Fixes / features** — see below.

## Development setup

- Firmware: see [`firmware/README.md`](firmware/README.md).
- App: see [`app/README.md`](app/README.md).

## Before opening a PR

1. Keep firmware and app changes protocol-compatible, or update
   [`docs/BLE_PROTOCOL.md`](docs/BLE_PROTOCOL.md) in the same PR and explain
   the compatibility impact (see its "Versioning" section).
2. Firmware: the sketch should still compile for a plain `esp32:esp32:esp32`
   board with the default partition scheme (Arduino IDE, esp32 core,
   ArduinoJson, NimBLE-Arduino) — that's the tightest fit of any supported
   chip, so it's the one to check after any change that adds code.
3. App: `flutter analyze` should pass with no new warnings.
4. Describe what you tested (which board, which phone) in the PR
   description — this is hardware-adjacent code that CI can't fully verify.

## Code style

- Match the existing style in the file you're editing rather than
  introducing a new one.
- Prefer small, focused PRs over large ones that mix refactors with
  behavior changes.
- No new dependencies (firmware libraries or Flutter packages) without a
  reason in the PR description — this repo is meant to stay a lightweight
  baseline other people build products on top of.

## License

By contributing, you agree your contributions are licensed under this
repo's [MIT License](LICENSE).
