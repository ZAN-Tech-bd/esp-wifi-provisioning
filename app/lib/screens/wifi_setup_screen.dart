import 'package:flutter/material.dart';

import '../models/provisioning_status.dart';
import '../models/wifi_network.dart';
import '../services/ble_provisioning_service.dart';
import '../widgets/wifi_signal_icon.dart';
import 'provisioning_status_screen.dart';

class WifiSetupScreen extends StatefulWidget {
  final BleProvisioningService service;

  const WifiSetupScreen({super.key, required this.service});

  @override
  State<WifiSetupScreen> createState() => _WifiSetupScreenState();
}

class _WifiSetupScreenState extends State<WifiSetupScreen> {
  @override
  void initState() {
    super.initState();
    widget.service.addListener(_onServiceChanged);
    widget.service.requestWifiScan();
  }

  @override
  void dispose() {
    widget.service.removeListener(_onServiceChanged);
    super.dispose();
  }

  void _onServiceChanged() {
    if (mounted) setState(() {});
  }

  Future<void> _selectNetwork(WifiNetwork network) async {
    final password = await _promptForPassword(network);
    if (password == null) return; // cancelled

    if (!mounted) return;
    await widget.service.sendCredentials(network.ssid, password);
    if (!mounted) return;

    Navigator.of(context).push(
      MaterialPageRoute(
        builder: (_) => ProvisioningStatusScreen(
          service: widget.service,
          ssid: network.ssid,
        ),
      ),
    );
  }

  Future<String?> _promptForPassword(WifiNetwork network) {
    final controller = TextEditingController();
    bool obscure = true;

    return showModalBottomSheet<String>(
      context: context,
      isScrollControlled: true,
      builder: (context) {
        return StatefulBuilder(
          builder: (context, setModalState) {
            return Padding(
              padding: EdgeInsets.only(
                left: 20,
                right: 20,
                top: 20,
                bottom: MediaQuery.of(context).viewInsets.bottom + 20,
              ),
              child: Column(
                mainAxisSize: MainAxisSize.min,
                crossAxisAlignment: CrossAxisAlignment.stretch,
                children: [
                  Text(
                    network.ssid,
                    style: Theme.of(context).textTheme.titleLarge,
                  ),
                  const SizedBox(height: 4),
                  Text(
                    network.secure ? 'Secured network' : 'Open network',
                    style: Theme.of(context).textTheme.bodySmall,
                  ),
                  const SizedBox(height: 20),
                  if (network.secure)
                    TextField(
                      controller: controller,
                      obscureText: obscure,
                      autofocus: true,
                      decoration: InputDecoration(
                        labelText: 'Wi-Fi password',
                        suffixIcon: IconButton(
                          icon: Icon(obscure
                              ? Icons.visibility_off
                              : Icons.visibility),
                          onPressed: () =>
                              setModalState(() => obscure = !obscure),
                        ),
                      ),
                      onSubmitted: (v) => Navigator.of(context).pop(v),
                    ),
                  const SizedBox(height: 16),
                  ElevatedButton(
                    onPressed: () =>
                        Navigator.of(context).pop(controller.text),
                    child: const Text('Connect'),
                  ),
                ],
              ),
            );
          },
        );
      },
    );
  }

  @override
  Widget build(BuildContext context) {
    final networks = widget.service.networks;
    final scanning = widget.service.status.state == ProvisioningState.scanning;

    return Scaffold(
      appBar: AppBar(
        title: Text(widget.service.connectedDeviceName ?? 'Set up Wi-Fi'),
        actions: [
          IconButton(
            icon: const Icon(Icons.refresh),
            onPressed: scanning ? null : widget.service.requestWifiScan,
          ),
        ],
      ),
      body: Column(
        children: [
          if (scanning) const LinearProgressIndicator(minHeight: 2),
          Expanded(
            child: networks.isEmpty
                ? Center(
                    child: Text(
                      scanning
                          ? 'Scanning for Wi-Fi networks…'
                          : 'No networks found. Tap refresh to scan again.',
                      style: Theme.of(context).textTheme.bodyLarge,
                    ),
                  )
                : ListView.separated(
                    padding: const EdgeInsets.all(16),
                    itemCount: networks.length,
                    separatorBuilder: (_, __) => const SizedBox(height: 4),
                    itemBuilder: (context, index) {
                      final network = networks[index];
                      return Card(
                        child: ListTile(
                          leading: WifiSignalIcon(
                            bars: network.signalBars,
                            secure: network.secure,
                          ),
                          title: Text(network.ssid),
                          subtitle: Text('${network.rssi} dBm'),
                          trailing: const Icon(Icons.chevron_right),
                          onTap: () => _selectNetwork(network),
                        ),
                      );
                    },
                  ),
          ),
        ],
      ),
    );
  }
}
