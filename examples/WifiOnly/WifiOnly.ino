/*
 * WifiOnly - the smallest possible sketch using ZanWifiSetup.
 * -------------------------------------------------------------
 * Flash this once to get a device onto Wi-Fi and nothing else. It never
 * needs to be re-uploaded - it opens a "ZAN-Setup-XXXX" hotspot with a web
 * form the first time (or whenever it can't connect), and just sits
 * connected once it succeeds.
 *
 * When you're ready to build an actual project on top of this, don't add
 * to this file - start a new sketch, #include <ZanWifiSetup.h> there too,
 * and see examples/BlinkWhileConnected for how to add your own code
 * alongside it.
 */
#include <ZanWifiSetup.h>

void setup() {
  ZanWifiSetup.begin();
}

void loop() {
  ZanWifiSetup.loop();
}
