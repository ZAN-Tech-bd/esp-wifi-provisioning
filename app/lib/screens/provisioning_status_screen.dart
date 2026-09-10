import 'package:flutter/material.dart';

import '../models/provisioning_status.dart';
import '../services/ble_provisioning_service.dart';

class ProvisioningStatusScreen extends StatefulWidget {
  final BleProvisioningService service;
  final String ssid;

  const ProvisioningStatusScreen({
    super.key,
    required this.service,
    required this.ssid,
  });

  @override
  State<ProvisioningStatusScreen> createState() =>
      _ProvisioningStatusScreenState();
}

class _ProvisioningStatusScreenState extends State<ProvisioningStatusScreen> {
  @override
  void initState() {
    super.initState();
    widget.service.addListener(_onServiceChanged);
  }

  @override
  void dispose() {
    widget.service.removeListener(_onServiceChanged);
    super.dispose();
  }

  void _onServiceChanged() {
    if (mounted) setState(() {});
  }

  Future<void> _forgetAndRetry() async {
    await widget.service.resetDevice();
    if (!mounted) return;
    Navigator.of(context).pop();
  }

  Future<void> _finish() async {
    await widget.service.disconnect();
    if (!mounted) return;
    Navigator.of(context).popUntil((route) => route.isFirst);
  }

  @override
  Widget build(BuildContext context) {
    final status = widget.service.status;

    return Scaffold(
      appBar: AppBar(title: const Text('Provisioning')),
      body: Center(
        child: Padding(
          padding: const EdgeInsets.all(32),
          child: switch (status.state) {
            ProvisioningState.connecting => _Connecting(ssid: widget.ssid),
            ProvisioningState.connected => _Connected(
                status: status,
                onDone: _finish,
              ),
            ProvisioningState.failed => _Failed(
                status: status,
                onRetry: () => Navigator.of(context).pop(),
                onForget: _forgetAndRetry,
              ),
            _ => _Connecting(ssid: widget.ssid),
          },
        ),
      ),
    );
  }
}

class _Connecting extends StatelessWidget {
  final String ssid;

  const _Connecting({required this.ssid});

  @override
  Widget build(BuildContext context) {
    return Column(
      mainAxisSize: MainAxisSize.min,
      children: [
        const CircularProgressIndicator(),
        const SizedBox(height: 24),
        Text(
          'Connecting the device to "$ssid"…',
          textAlign: TextAlign.center,
          style: Theme.of(context).textTheme.titleMedium,
        ),
        const SizedBox(height: 8),
        Text(
          'Keep the app open and stay near the device.',
          textAlign: TextAlign.center,
          style: Theme.of(context).textTheme.bodySmall,
        ),
      ],
    );
  }
}

class _Connected extends StatelessWidget {
  final ProvisioningStatus status;
  final VoidCallback onDone;

  const _Connected({required this.status, required this.onDone});

  @override
  Widget build(BuildContext context) {
    return Column(
      mainAxisSize: MainAxisSize.min,
      children: [
        Icon(Icons.check_circle,
            color: Colors.green.shade600, size: 72),
        const SizedBox(height: 24),
        Text(
          'Connected to "${status.ssid}"',
          textAlign: TextAlign.center,
          style: Theme.of(context).textTheme.titleLarge,
        ),
        if (status.ip != null) ...[
          const SizedBox(height: 8),
          Text('Device IP: ${status.ip}',
              style: Theme.of(context).textTheme.bodyMedium),
        ],
        const SizedBox(height: 32),
        SizedBox(
          width: double.infinity,
          child: ElevatedButton(onPressed: onDone, child: const Text('Done')),
        ),
      ],
    );
  }
}

class _Failed extends StatelessWidget {
  final ProvisioningStatus status;
  final VoidCallback onRetry;
  final VoidCallback onForget;

  const _Failed({
    required this.status,
    required this.onRetry,
    required this.onForget,
  });

  @override
  Widget build(BuildContext context) {
    return Column(
      mainAxisSize: MainAxisSize.min,
      children: [
        Icon(Icons.error, color: Theme.of(context).colorScheme.error, size: 72),
        const SizedBox(height: 24),
        Text(
          'Couldn\'t connect to "${status.ssid}"',
          textAlign: TextAlign.center,
          style: Theme.of(context).textTheme.titleLarge,
        ),
        const SizedBox(height: 8),
        Text(
          status.friendlyFailureReason,
          textAlign: TextAlign.center,
          style: Theme.of(context).textTheme.bodyMedium,
        ),
        const SizedBox(height: 32),
        SizedBox(
          width: double.infinity,
          child: ElevatedButton(
            onPressed: onRetry,
            child: const Text('Choose a different network'),
          ),
        ),
        const SizedBox(height: 8),
        SizedBox(
          width: double.infinity,
          child: OutlinedButton(
            onPressed: onForget,
            child: const Text('Forget device credentials'),
          ),
        ),
      ],
    );
  }
}
