import 'package:flutter/material.dart';

class WifiSignalIcon extends StatelessWidget {
  final int bars; // 1-4
  final bool secure;

  const WifiSignalIcon({super.key, required this.bars, required this.secure});

  @override
  Widget build(BuildContext context) {
    final IconData icon = switch (bars) {
      4 => Icons.wifi,
      3 => Icons.wifi,
      2 => Icons.wifi_2_bar,
      _ => Icons.wifi_1_bar,
    };
    return Stack(
      alignment: Alignment.bottomRight,
      children: [
        Icon(icon, size: 26),
        if (secure)
          Container(
            padding: const EdgeInsets.all(1),
            decoration: BoxDecoration(
              color: Theme.of(context).colorScheme.surface,
              shape: BoxShape.circle,
            ),
            child: const Icon(Icons.lock, size: 12),
          ),
      ],
    );
  }
}
