/*
 * ZanWifiSetup - one-time Wi-Fi provisioning for any ESP32 project.
 * ---------------------------------------------------------------
 * Include this once in a project and never touch it again. Every future
 * project just calls ZanWifiSetup.begin()/loop() and writes its own logic
 * around it - the hotspot, web form, status LED, and reset button are
 * handled entirely inside this library.
 *
 * Minimal usage:
 *
 *   #include <ZanWifiSetup.h>
 *
 *   void setup() {
 *     ZanWifiSetup.begin();
 *     // your one-time setup code
 *   }
 *
 *   void loop() {
 *     ZanWifiSetup.loop();
 *     // your project code - check ZanWifiSetup.isConnected() if it needs
 *     // the network
 *   }
 *
 * See examples/WifiOnly for the smallest possible sketch (literally the
 * above, nothing else) and examples/BlinkWhileConnected for a sketch that
 * adds its own logic alongside it.
 *
 * How it behaves:
 *   - On begin(), tries whatever Wi-Fi was saved last time.
 *   - If that works, isConnected() is true from then on and the onboard
 *     LED lights up solid.
 *   - If it doesn't (wrong password, out of range, or nothing was ever
 *     saved), it opens a hotspot named "ZAN-Setup-XXXX" with a web form at
 *     192.168.4.1 to enter the real Wi-Fi name/password. The LED blinks
 *     while in this mode. Submitting the form saves it and restarts.
 *   - Holding the reset button (BOOT / GPIO0 by default) for 3 seconds
 *     erases the saved Wi-Fi and restarts into the hotspot - use this when
 *     moving a device to a new place with different Wi-Fi.
 *
 * Made by ZAN Tech - https://github.com/ZAN-Tech-bd/esp-wifi-provisioning
 * Licensed under the MIT License.
 */
#pragma once

#include <Arduino.h>
#include <WebServer.h>

class ZanWifiSetupClass {
 public:
  // Call once in setup(). Blocks briefly (a few seconds) while trying any
  // saved Wi-Fi; returns once either connected or the setup hotspot is up.
  void begin();

  // Call on every loop() iteration - handles the web server, the status
  // LED, and watching the reset button. Very cheap when idle.
  void loop();

  // True once connected to the real Wi-Fi network.
  bool isConnected();

  // --- Optional configuration - call these BEFORE begin(), any you skip
  // keep their default shown below. ---

  void setHotspotPrefix(const char *prefix);   // default "ZAN-Setup-"
  void setStatusLedPin(int pin);               // default LED_BUILTIN, -1 disables
  void setResetButtonPin(int pin);             // default 0 (BOOT button), -1 disables
  void setConnectTimeoutMs(uint32_t ms);       // default 10000

 private:
  const char *hotspotPrefix_ = "ZAN-Setup-";
  int statusLedPin_ = -2;   // sentinel: "use LED_BUILTIN if it exists" - resolved in begin()
  int resetButtonPin_ = 0;
  uint32_t connectTimeoutMs_ = 10000;

  WebServer server_{80};
  bool apMode_ = false;
  uint32_t resetPressedAtMs_ = 0;
  uint32_t lastBlinkMs_ = 0;
  bool ledState_ = false;

  String hotspotName() const;
  void startAccessPoint();
  void handleRoot();
  void handleSave();
  void handleNotFound();
  void checkResetButton();
  void updateStatusLed();
};

extern ZanWifiSetupClass ZanWifiSetup;
