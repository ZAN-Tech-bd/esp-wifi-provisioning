/*
 * BlinkWhileConnected - shows where your own project code goes.
 * -----------------------------------------------------------------
 * Same Wi-Fi setup as the WifiOnly example (same library, same behavior),
 * plus one small piece of actual project logic: it blinks an LED once a
 * second, but only once Wi-Fi is connected. That's the pattern for
 * everything you build on top of this library - check
 * ZanWifiSetup.isConnected() before doing anything that needs the network,
 * and never touch ZanWifiSetup itself.
 *
 * Swap this blink for your real project: read a sensor, publish to MQTT,
 * poll an API, whatever it needs to be.
 */
#include <ZanWifiSetup.h>

const int PROJECT_LED_PIN = 2;  // pick a pin that isn't your status LED

unsigned long lastBlinkMs = 0;
bool ledOn = false;

void setup() {
  ZanWifiSetup.begin();

  pinMode(PROJECT_LED_PIN, OUTPUT);
}

void loop() {
  ZanWifiSetup.loop();

  if (ZanWifiSetup.isConnected() && millis() - lastBlinkMs > 1000) {
    lastBlinkMs = millis();
    ledOn = !ledOn;
    digitalWrite(PROJECT_LED_PIN, ledOn ? HIGH : LOW);
  }
}
