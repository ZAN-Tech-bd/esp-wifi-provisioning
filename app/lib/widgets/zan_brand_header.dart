import 'package:flutter/material.dart';

/// Small "Made by ZAN Tech" wordmark used across screens so the product's
/// origin stays visible without a dedicated splash screen.
class ZanBrandHeader extends StatelessWidget {
  const ZanBrandHeader({super.key});

  @override
  Widget build(BuildContext context) {
    final scheme = Theme.of(context).colorScheme;
    return Row(
      mainAxisSize: MainAxisSize.min,
      children: [
        Container(
          width: 10,
          height: 10,
          decoration: BoxDecoration(color: scheme.primary, shape: BoxShape.circle),
        ),
        const SizedBox(width: 8),
        Text(
          'ZAN Tech',
          style: TextStyle(
            fontWeight: FontWeight.w700,
            fontSize: 13,
            letterSpacing: 0.5,
            color: scheme.onSurfaceVariant,
          ),
        ),
      ],
    );
  }
}
