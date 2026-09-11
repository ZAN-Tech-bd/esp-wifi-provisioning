/*
 * CustomConfiguration - every ZanWifiSetup option, in one place.
 * ------------------------------------------------------------------
 * All four setters are optional - skip any of them to keep that default.
 * Call whichever ones you want BEFORE ZanWifiSetup.begin(), never after.
 */
#include <ZanWifiSetup.h>

void setup() {
  // Hotspot name prefix. Default is "ZAN-Setup-"; the device's MAC suffix
  // is always appended automatically (e.g. "MyThing-8C38").
  ZanWifiSetup.setHotspotPrefix("MyThing-");

  // Which pin shows connection status (solid = connected, blinking =
  // waiting for setup). Default is LED_BUILTIN. Use -1 to disable the LED
  // entirely, e.g. if you need that pin for your own project.
  ZanWifiSetup.setStatusLedPin(2);

  // Which pin, held LOW for 3 seconds, erases saved Wi-Fi and reopens the
  // setup hotspot. Default is 0 (the BOOT button on most dev boards). Use
  // -1 to disable the reset button entirely.
  ZanWifiSetup.setResetButtonPin(0);

  // How long (milliseconds) to try a saved network before giving up and
  // opening the setup hotspot instead. Default is 10000 (10 seconds).
  ZanWifiSetup.setConnectTimeoutMs(15000);

  ZanWifiSetup.begin();
}

void loop() {
  ZanWifiSetup.loop();
}
