import 'package:flutter/material.dart';

import 'screens/device_scan_screen.dart';
import 'services/ble_provisioning_service.dart';
import 'theme/app_theme.dart';

void main() {
  runApp(const ZanWifiProvisioningApp());
}

/// ZAN Tech BLE Wi-Fi Provisioning — open-source baseline app.
///
/// One long-lived [BleProvisioningService] instance is created here and
/// passed down through constructors; see that class for the entire BLE
/// state machine. Fork this app, swap the theme/branding, and build your
/// own onboarding flow on top of the same service.
class ZanWifiProvisioningApp extends StatefulWidget {
  const ZanWifiProvisioningApp({super.key});

  @override
  State<ZanWifiProvisioningApp> createState() => _ZanWifiProvisioningAppState();
}

class _ZanWifiProvisioningAppState extends State<ZanWifiProvisioningApp> {
  final BleProvisioningService _service = BleProvisioningService();

  @override
  void dispose() {
    _service.dispose();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: 'ZAN Tech Wi-Fi Provisioning',
      debugShowCheckedModeBanner: false,
      theme: AppTheme.light(),
      darkTheme: AppTheme.dark(),
      home: DeviceScanScreen(service: _service),
    );
  }
}
