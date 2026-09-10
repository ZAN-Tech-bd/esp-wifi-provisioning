import 'package:flutter_test/flutter_test.dart';

import 'package:zan_wifi_provisioning/main.dart';

void main() {
  testWidgets('App boots to the device scan screen', (WidgetTester tester) async {
    await tester.pumpWidget(const ZanWifiProvisioningApp());
    await tester.pump();

    expect(find.text('Find your device'), findsOneWidget);
  });
}
