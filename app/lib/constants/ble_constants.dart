import 'package:flutter_blue_plus/flutter_blue_plus.dart';

/// ZAN Tech BLE Wi-Fi Provisioning Protocol v1.
///
/// Mirrors `docs/BLE_PROTOCOL.md` exactly. If you change a UUID here, change
/// it in `firmware/esp-ble-wifi-provisioning/config.h` too, or the two sides
/// stop talking to each other.
class BleConstants {
  BleConstants._();

  static final Guid serviceUuid =
      Guid('b8a10000-2d4c-4a1e-9f3a-000000000000');

  static final Guid deviceInfoCharUuid =
      Guid('b8a10001-2d4c-4a1e-9f3a-000000000000');

  static final Guid commandCharUuid =
      Guid('b8a10002-2d4c-4a1e-9f3a-000000000000');

  static final Guid networkCharUuid =
      Guid('b8a10003-2d4c-4a1e-9f3a-000000000000');

  static final Guid credentialsCharUuid =
      Guid('b8a10004-2d4c-4a1e-9f3a-000000000000');

  static final Guid statusCharUuid =
      Guid('b8a10005-2d4c-4a1e-9f3a-000000000000');

  /// Devices advertise as this prefix + 4 hex chars (last 2 MAC bytes).
  static const devicenamePrefix = 'ZAN-Prov-';

  static const scanTimeout = Duration(seconds: 8);
  static const connectTimeout = Duration(seconds: 10);
  static const requestMtu = 217; // Android only; comfortably fits a 63-char password
}
