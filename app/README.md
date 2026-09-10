# App — ZAN Tech Wi-Fi Provisioning (Flutter)

Scans for nearby ESP32 devices running the companion firmware, lets the user
pick a Wi-Fi network and enter its password, and hands those credentials to
the device over BLE. Built against [`docs/BLE_PROTOCOL.md`](../docs/BLE_PROTOCOL.md).

## What's in this folder

This repo ships the app's **Dart source only** (`lib/`, `pubspec.yaml`) —
the native `android/`, `ios/`, `web/`, etc. project shells that
`flutter create` normally generates are not committed, since they're
regenerated per-target and go stale fast. Scaffold them once with the
Flutter CLI, then this source drops straight in.

```
lib/
├── main.dart                          # app entry point, wires everything together
├── constants/ble_constants.dart       # UUIDs + timeouts, mirrors docs/BLE_PROTOCOL.md
├── models/                            # WifiNetwork, ProvisioningStatus, DeviceInfo, ...
├── services/ble_provisioning_service.dart  # the entire BLE state machine (framework-agnostic)
├── screens/                           # DeviceScan -> WifiSetup -> ProvisioningStatus
├── widgets/                           # small shared UI pieces
└── theme/app_theme.dart               # ZAN Tech brand color — change this to re-skin
```

## Setup

```bash
cd app
flutter create --org com.zantech --project-name zan_wifi_provisioning .
flutter pub get
```

`flutter create` fills in only the missing platform folders — it will not
touch `lib/`, `pubspec.yaml`, or this README since they already exist.

## Platform permissions

BLE scanning/connecting needs explicit permissions on both platforms. Add
these after running `flutter create`:

### Android — `android/app/src/main/AndroidManifest.xml`

Add inside `<manifest>`, above `<application>`:

```xml
<uses-permission android:name="android.permission.BLUETOOTH_SCAN" android:usesPermissionFlags="neverForLocation" />
<uses-permission android:name="android.permission.BLUETOOTH_CONNECT" />
<uses-permission android:name="android.permission.ACCESS_FINE_LOCATION" android:maxSdkVersion="30" />
<uses-feature android:name="android.hardware.bluetooth_le" android:required="true" />
```

`neverForLocation` is safe here because this app never derives physical
location from BLE scan results — it only matches devices by advertised
service UUID / name. `ACCESS_FINE_LOCATION` is still required by the OS on
Android 11 (API 30) and below for any BLE scan.

`flutter_blue_plus` requires `minSdkVersion` 21+; recent Flutter versions
already default `android/app/build.gradle.kts`'s `minSdk` above that via
`flutter.minSdkVersion`, so this is normally a non-issue — only check it
if you're on an older Flutter/Gradle template.

### iOS — `ios/Runner/Info.plist`

Add:

```xml
<key>NSBluetoothAlwaysUsageDescription</key>
<string>This app uses Bluetooth to find and set up your ZAN Tech device.</string>
<key>NSBluetoothPeripheralUsageDescription</key>
<string>This app uses Bluetooth to find and set up your ZAN Tech device.</string>
```

## Running

BLE does not work in the iOS Simulator or most Android emulators — use a
real phone connected via USB (or wireless debugging). Confirm it's detected,
then run:

```bash
flutter devices   # your phone should be listed
flutter run       # builds, installs, launches, hot-reloads
```

To build once and install separately instead (e.g. scripting it, or you
don't want `flutter run`'s interactive session):

```bash
flutter build apk --debug
adb install -r build/app/outputs/flutter-apk/app-debug.apk
adb shell am start -n com.zantech.zan_wifi_provisioning/.MainActivity
```

## Rebranding for your own product

Everything ZAN-Tech-specific lives in a handful of places:

- `lib/theme/app_theme.dart` — `brandColor`
- `lib/widgets/zan_brand_header.dart` — the small wordmark shown on the scan screen
- `lib/main.dart` — app title, class name
- App name / icon / bundle id — set via `flutter create --org ... --project-name ...` and your platform's usual app icon tooling (e.g. `flutter_launcher_icons`)

The BLE protocol itself (`lib/constants/ble_constants.dart`, `lib/services/ble_provisioning_service.dart`)
has nothing ZAN-Tech-specific in it beyond the UUIDs, so it works unmodified
against any firmware that implements [`docs/BLE_PROTOCOL.md`](../docs/BLE_PROTOCOL.md).

## Dependencies

| Package | Why |
|---|---|
| [`flutter_blue_plus`](https://pub.dev/packages/flutter_blue_plus) | BLE central role: scan, connect, read/write/notify characteristics |
| [`permission_handler`](https://pub.dev/packages/permission_handler) | Runtime Bluetooth/location permission prompts |
