class ZanDeviceInfo {
  final String device;
  final String firmwareVersion;
  final String mac;
  final String chip;

  const ZanDeviceInfo({
    required this.device,
    required this.firmwareVersion,
    required this.mac,
    required this.chip,
  });

  factory ZanDeviceInfo.fromJson(Map<String, dynamic> json) => ZanDeviceInfo(
        device: json['device'] as String? ?? 'Unknown',
        firmwareVersion: json['fw'] as String? ?? '0.0.0',
        mac: json['mac'] as String? ?? '',
        chip: json['chip'] as String? ?? '',
      );
}
