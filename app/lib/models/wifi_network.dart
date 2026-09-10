class WifiNetwork {
  final String ssid;
  final int rssi;
  final bool secure;

  const WifiNetwork({
    required this.ssid,
    required this.rssi,
    required this.secure,
  });

  factory WifiNetwork.fromJson(Map<String, dynamic> json) => WifiNetwork(
        ssid: json['ssid'] as String? ?? '',
        rssi: (json['rssi'] as num?)?.toInt() ?? -100,
        secure: json['secure'] as bool? ?? true,
      );

  /// 1-4, for a signal-strength icon.
  int get signalBars {
    if (rssi >= -55) return 4;
    if (rssi >= -65) return 3;
    if (rssi >= -75) return 2;
    return 1;
  }

  @override
  bool operator ==(Object other) => other is WifiNetwork && other.ssid == ssid;

  @override
  int get hashCode => ssid.hashCode;
}
