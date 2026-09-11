# Contributing

Thanks for considering a contribution to this project — issues, pull
requests, and forks are all welcome.

## Ways to help

- **Bug reports** — open an issue with your board (e.g. ESP32-DevKitC,
  ESP32-S3), Arduino core version, phone/laptop OS, and the Serial Monitor
  log around the failure.
- **Ports** — if you adapt this for ESP8266 or another platform, we'd love
  a link from the README.
- **Fixes / features** — see below.

## Development setup

See [`docs/GETTING_STARTED.md`](docs/GETTING_STARTED.md). This repo is an
Arduino library (`src/`), not a standalone sketch — both example sketches
under `examples/` compile against it directly via `--library .` /
"Add .ZIP Library" pointing at the repo root.

## Before opening a PR

1. The library should still compile with **no external dependencies** —
   that's the whole point of this baseline staying simple. If your change
   needs one, explain why in the PR description.
2. Try to keep both examples compiling for every ESP32 variant
   (S2/S3/C3/C6/C2 and the original chip), not just whichever one you're
   testing on.
3. If you change the public API (`src/ZanWifiSetup.h`), update the inline
   doc comments there and the configuration table in
   [`docs/GETTING_STARTED.md`](docs/GETTING_STARTED.md) to match.
4. Describe what you actually tested (which board, which phone/laptop) in
   the PR description — this is hardware-adjacent code that CI can't fully
   verify.

## Code style

- Match the existing style in the file you're editing rather than
  introducing a new one.
- Prefer small, focused PRs over large ones that mix refactors with
  behavior changes.
- No new dependencies without a reason in the PR description — this repo
  is meant to stay a lightweight baseline other people build products on
  top of.

## License

By contributing, you agree your contributions are licensed under this
repo's [MIT License](LICENSE).
