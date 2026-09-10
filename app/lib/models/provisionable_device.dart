import 'package:flutter_blue_plus/flutter_blue_plus.dart';

/// A BLE-discovered device advertising the ZAN Tech provisioning service.
class ProvisionableDevice {
  final BluetoothDevice device;
  final String name;
  final int rssi;

  const ProvisionableDevice({
    required this.device,
    required this.name,
    required this.rssi,
  });

  String get id => device.remoteId.str;

  @override
  bool operator ==(Object other) =>
      other is ProvisionableDevice && other.id == id;

  @override
  int get hashCode => id.hashCode;
}
