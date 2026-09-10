import 'dart:async';
import 'dart:convert';

import 'package:flutter/foundation.dart';
import 'package:flutter_blue_plus/flutter_blue_plus.dart';
import 'package:permission_handler/permission_handler.dart';

import '../constants/ble_constants.dart';
import '../models/device_info.dart';
import '../models/provisionable_device.dart';
import '../models/provisioning_status.dart';
import '../models/wifi_network.dart';

/// The entire BLE state machine for talking to a ZAN Tech provisioning
/// device, implementing docs/BLE_PROTOCOL.md. Framework-agnostic — the UI
/// layer just listens via [ChangeNotifier] and calls the public methods.
class BleProvisioningService extends ChangeNotifier {
  BluetoothDevice? _connectedDevice;
  BluetoothCharacteristic? _commandChar;
  BluetoothCharacteristic? _credentialsChar;
  BluetoothCharacteristic? _statusChar;
  BluetoothCharacteristic? _networkChar;
  BluetoothCharacteristic? _deviceInfoChar;

  StreamSubscription<List<ScanResult>>? _scanSub;
  StreamSubscription<BluetoothConnectionState>? _connectionSub;
  StreamSubscription<List<int>>? _statusSub;
  StreamSubscription<List<int>>? _networkSub;

  final Map<String, ProvisionableDevice> _discovered = {};
  bool _isScanningForDevices = false;
  ProvisioningStatus _status = ProvisioningStatus.disconnected;
  ZanDeviceInfo? _deviceInfo;
  final List<WifiNetwork> _networks = [];
  String? _lastError;

  List<ProvisionableDevice> get discoveredDevices =>
      _discovered.values.toList()..sort((a, b) => b.rssi.compareTo(a.rssi));
  bool get isScanningForDevices => _isScanningForDevices;
  bool get isConnected => _connectedDevice != null;
  ProvisioningStatus get status => _status;
  ZanDeviceInfo? get deviceInfo => _deviceInfo;
  List<WifiNetwork> get networks => List.unmodifiable(_networks);
  String? get lastError => _lastError;
  String? get connectedDeviceName => _connectedDevice?.platformName;

  // ---------------------------------------------------------------------
  // Permissions
  // ---------------------------------------------------------------------

  /// Requests the runtime permissions BLE scanning/connecting needs.
  /// Safe to call every time before scanning; a no-op once granted.
  Future<bool> ensurePermissions() async {
    final statuses = await [
      Permission.bluetoothScan,
      Permission.bluetoothConnect,
      Permission.locationWhenInUse,
    ].request();

    return statuses.values.every(
      (s) => s.isGranted || s.isLimited || s.isRestricted,
    );
  }

  // ---------------------------------------------------------------------
  // Device discovery
  // ---------------------------------------------------------------------

  Future<void> startDeviceScan() async {
    _discovered.clear();
    _lastError = null;
    notifyListeners();

    final granted = await ensurePermissions();
    if (!granted) {
      _lastError = 'Bluetooth/location permission was denied.';
      notifyListeners();
      return;
    }

    await _scanSub?.cancel();
    _isScanningForDevices = true;
    notifyListeners();

    _scanSub = FlutterBluePlus.scanResults.listen((results) {
      for (final result in results) {
        final name = result.device.platformName;
        final advertisedServices = result.advertisementData.serviceUuids;
        final matchesService = advertisedServices.contains(BleConstants.serviceUuid);
        final matchesName = name.startsWith(BleConstants.devicenamePrefix);

        if (matchesService || matchesName) {
          _discovered[result.device.remoteId.str] = ProvisionableDevice(
            device: result.device,
            name: name.isEmpty ? result.device.remoteId.str : name,
            rssi: result.rssi,
          );
        }
      }
      notifyListeners();
    });

    try {
      await FlutterBluePlus.startScan(timeout: BleConstants.scanTimeout);
    } catch (e) {
      _lastError = 'Failed to start scan: $e';
    }

    // startScan's timeout resolves the future when the OS-level scan stops;
    // reflect that in our own flag too.
    _isScanningForDevices = false;
    notifyListeners();
  }

  Future<void> stopDeviceScan() async {
    await FlutterBluePlus.stopScan();
    _isScanningForDevices = false;
    notifyListeners();
  }

  // ---------------------------------------------------------------------
  // Connection lifecycle
  // ---------------------------------------------------------------------

  Future<bool> connect(ProvisionableDevice target) async {
    await stopDeviceScan();
    _lastError = null;
    _networks.clear();
    notifyListeners();

    try {
      await target.device.connect(timeout: BleConstants.connectTimeout);
      _connectedDevice = target.device;

      await _connectionSub?.cancel();
      _connectionSub = target.device.connectionState.listen((state) {
        if (state == BluetoothConnectionState.disconnected) {
          _handleDisconnect();
        }
      });

      if (defaultTargetPlatform == TargetPlatform.android) {
        try {
          await target.device.requestMtu(BleConstants.requestMtu);
        } catch (_) {
          // Non-fatal: falls back to the negotiated default MTU.
        }
      }

      final services = await target.device.discoverServices();
      final service = services.firstWhere(
        (s) => s.uuid == BleConstants.serviceUuid,
        orElse: () => throw Exception('ZAN Tech provisioning service not found on device'),
      );

      for (final c in service.characteristics) {
        if (c.uuid == BleConstants.deviceInfoCharUuid) _deviceInfoChar = c;
        if (c.uuid == BleConstants.commandCharUuid) _commandChar = c;
        if (c.uuid == BleConstants.networkCharUuid) _networkChar = c;
        if (c.uuid == BleConstants.credentialsCharUuid) _credentialsChar = c;
        if (c.uuid == BleConstants.statusCharUuid) _statusChar = c;
      }

      if (_commandChar == null || _credentialsChar == null || _statusChar == null) {
        throw Exception('Device is missing required characteristics');
      }

      await _subscribeToNotifications();
      await _readDeviceInfo();

      _status = ProvisioningStatus.idle;
      notifyListeners();
      return true;
    } catch (e) {
      _lastError = 'Could not connect: $e';
      await disconnect();
      return false;
    }
  }

  Future<void> _subscribeToNotifications() async {
    await _statusChar!.setNotifyValue(true);
    _statusSub = _statusChar!.lastValueStream.listen((bytes) {
      if (bytes.isEmpty) return;
      final json = _decodeJson(bytes);
      if (json == null) return;
      _status = ProvisioningStatus.fromJson(json);
      if (_status.state == ProvisioningState.scanning) {
        _networks.clear();
      }
      notifyListeners();
    });

    if (_networkChar != null) {
      await _networkChar!.setNotifyValue(true);
      _networkSub = _networkChar!.lastValueStream.listen((bytes) {
        if (bytes.isEmpty) return;
        final json = _decodeJson(bytes);
        if (json == null || json['type'] != 'network') return;
        final network = WifiNetwork.fromJson(json);
        _networks
          ..remove(network)
          ..add(network);
        _networks.sort((a, b) => b.rssi.compareTo(a.rssi));
        notifyListeners();
      });
    }
  }

  Future<void> _readDeviceInfo() async {
    if (_deviceInfoChar == null) return;
    try {
      final bytes = await _deviceInfoChar!.read();
      final json = _decodeJson(bytes);
      if (json != null) {
        _deviceInfo = ZanDeviceInfo.fromJson(json);
      }
    } catch (_) {
      // Non-fatal — device info is informational only.
    }
  }

  void _handleDisconnect() {
    _connectedDevice = null;
    _commandChar = null;
    _credentialsChar = null;
    _statusChar = null;
    _networkChar = null;
    _deviceInfoChar = null;
    _deviceInfo = null;
    _status = ProvisioningStatus.disconnected;
    _networks.clear();
    notifyListeners();
  }

  Future<void> disconnect() async {
    await _statusSub?.cancel();
    await _networkSub?.cancel();
    await _connectionSub?.cancel();
    try {
      await _connectedDevice?.disconnect();
    } catch (_) {
      // already gone
    }
    _handleDisconnect();
  }

  // ---------------------------------------------------------------------
  // Provisioning actions
  // ---------------------------------------------------------------------

  Future<void> requestWifiScan() async {
    await _writeJson(_commandChar, {'cmd': 'scan'});
  }

  Future<void> cancelConnection() async {
    await _writeJson(_commandChar, {'cmd': 'cancel'});
  }

  /// Erases the device's stored Wi-Fi credentials ("forget this network").
  Future<void> resetDevice() async {
    await _writeJson(_commandChar, {'cmd': 'reset'});
  }

  Future<void> sendCredentials(String ssid, String password) async {
    await _writeJson(_credentialsChar, {'ssid': ssid, 'password': password});
  }

  Future<void> _writeJson(
    BluetoothCharacteristic? characteristic,
    Map<String, dynamic> payload,
  ) async {
    if (characteristic == null) {
      _lastError = 'Not connected to a device.';
      notifyListeners();
      return;
    }
    try {
      final bytes = utf8.encode(jsonEncode(payload));
      await characteristic.write(bytes, withoutResponse: false);
    } catch (e) {
      _lastError = 'Write failed: $e';
      notifyListeners();
    }
  }

  Map<String, dynamic>? _decodeJson(List<int> bytes) {
    try {
      return jsonDecode(utf8.decode(bytes)) as Map<String, dynamic>;
    } catch (_) {
      return null;
    }
  }

  @override
  void dispose() {
    _scanSub?.cancel();
    _statusSub?.cancel();
    _networkSub?.cancel();
    _connectionSub?.cancel();
    super.dispose();
  }
}
