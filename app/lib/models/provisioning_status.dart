enum ProvisioningState {
  /// Not connected to any device over BLE yet.
  disconnected,
  idle,
  scanning,
  scanComplete,
  connecting,
  connected,
  failed,
}

class ProvisioningStatus {
  final ProvisioningState state;
  final String? ssid;
  final String? ip;
  final int? rssi;
  final String? failureReason;

  const ProvisioningStatus({
    required this.state,
    this.ssid,
    this.ip,
    this.rssi,
    this.failureReason,
  });

  static const disconnected =
      ProvisioningStatus(state: ProvisioningState.disconnected);
  static const idle = ProvisioningStatus(state: ProvisioningState.idle);

  factory ProvisioningStatus.fromJson(Map<String, dynamic> json) {
    switch (json['status'] as String? ?? 'idle') {
      case 'scanning':
        return const ProvisioningStatus(state: ProvisioningState.scanning);
      case 'scan_complete':
        return const ProvisioningStatus(state: ProvisioningState.scanComplete);
      case 'connecting':
        return ProvisioningStatus(
          state: ProvisioningState.connecting,
          ssid: json['ssid'] as String?,
        );
      case 'connected':
        return ProvisioningStatus(
          state: ProvisioningState.connected,
          ssid: json['ssid'] as String?,
          ip: json['ip'] as String?,
          rssi: (json['rssi'] as num?)?.toInt(),
        );
      case 'failed':
        return ProvisioningStatus(
          state: ProvisioningState.failed,
          ssid: json['ssid'] as String?,
          failureReason: json['reason'] as String?,
        );
      default:
        return idle;
    }
  }

  /// Human-readable text for the failure reason reported by the firmware.
  String get friendlyFailureReason {
    switch (failureReason) {
      case 'wrong_password':
        return 'Incorrect Wi-Fi password.';
      case 'not_found':
        return 'Network not found — it may be out of range.';
      case 'timeout':
        return 'Connection timed out.';
      default:
        return 'Could not connect to that network.';
    }
  }
}
