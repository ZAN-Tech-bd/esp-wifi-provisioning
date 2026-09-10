import 'package:flutter/material.dart';

import '../models/provisionable_device.dart';
import '../services/ble_provisioning_service.dart';
import '../widgets/zan_brand_header.dart';
import 'wifi_setup_screen.dart';

class DeviceScanScreen extends StatefulWidget {
  final BleProvisioningService service;

  const DeviceScanScreen({super.key, required this.service});

  @override
  State<DeviceScanScreen> createState() => _DeviceScanScreenState();
}

class _DeviceScanScreenState extends State<DeviceScanScreen> {
  bool _connecting = false;

  @override
  void initState() {
    super.initState();
    widget.service.addListener(_onServiceChanged);
    _startScan();
  }

  @override
  void dispose() {
    widget.service.removeListener(_onServiceChanged);
    widget.service.stopDeviceScan();
    super.dispose();
  }

  void _onServiceChanged() {
    if (mounted) setState(() {});
  }

  Future<void> _startScan() => widget.service.startDeviceScan();

  Future<void> _connect(ProvisionableDevice device) async {
    setState(() => _connecting = true);
    final ok = await widget.service.connect(device);
    if (!mounted) return;
    setState(() => _connecting = false);

    if (ok) {
      Navigator.of(context).push(
        MaterialPageRoute(
          builder: (_) => WifiSetupScreen(service: widget.service),
        ),
      );
    } else if (widget.service.lastError != null) {
      ScaffoldMessenger.of(context)
          .showSnackBar(SnackBar(content: Text(widget.service.lastError!)));
    }
  }

  @override
  Widget build(BuildContext context) {
    final devices = widget.service.discoveredDevices;

    return Scaffold(
      appBar: AppBar(
        title: const Text('Find your device'),
        actions: [
          IconButton(
            icon: const Icon(Icons.refresh),
            onPressed: widget.service.isScanningForDevices ? null : _startScan,
          ),
        ],
      ),
      body: Stack(
        children: [
          Column(
            children: [
              const SizedBox(height: 8),
              const Center(child: ZanBrandHeader()),
              const SizedBox(height: 8),
              if (widget.service.isScanningForDevices)
                const LinearProgressIndicator(minHeight: 2),
              Expanded(
                child: devices.isEmpty
                    ? _EmptyState(scanning: widget.service.isScanningForDevices)
                    : ListView.separated(
                        padding: const EdgeInsets.all(16),
                        itemCount: devices.length,
                        separatorBuilder: (_, __) => const SizedBox(height: 8),
                        itemBuilder: (context, index) {
                          final d = devices[index];
                          return Card(
                            child: ListTile(
                              leading: const Icon(Icons.developer_board),
                              title: Text(d.name),
                              subtitle: Text('Signal: ${d.rssi} dBm'),
                              trailing: const Icon(Icons.chevron_right),
                              onTap: _connecting ? null : () => _connect(d),
                            ),
                          );
                        },
                      ),
              ),
            ],
          ),
          if (_connecting)
            Container(
              color: Colors.black.withValues(alpha: 0.35),
              child: const Center(
                child: Card(
                  child: Padding(
                    padding: EdgeInsets.all(24),
                    child: Column(
                      mainAxisSize: MainAxisSize.min,
                      children: [
                        CircularProgressIndicator(),
                        SizedBox(height: 16),
                        Text('Connecting…'),
                      ],
                    ),
                  ),
                ),
              ),
            ),
        ],
      ),
    );
  }
}

class _EmptyState extends StatelessWidget {
  final bool scanning;

  const _EmptyState({required this.scanning});

  @override
  Widget build(BuildContext context) {
    return Center(
      child: Padding(
        padding: const EdgeInsets.all(32),
        child: Column(
          mainAxisSize: MainAxisSize.min,
          children: [
            Icon(
              scanning ? Icons.bluetooth_searching : Icons.bluetooth_disabled,
              size: 56,
              color: Theme.of(context).colorScheme.outline,
            ),
            const SizedBox(height: 16),
            Text(
              scanning
                  ? 'Looking for nearby ZAN Tech devices…'
                  : 'No devices found yet.',
              textAlign: TextAlign.center,
              style: Theme.of(context).textTheme.bodyLarge,
            ),
            const SizedBox(height: 8),
            Text(
              'Make sure the device is powered on and hasn\'t already been '
              'set up on Wi-Fi. Tap refresh to scan again.',
              textAlign: TextAlign.center,
              style: Theme.of(context).textTheme.bodySmall,
            ),
          ],
        ),
      ),
    );
  }
}
