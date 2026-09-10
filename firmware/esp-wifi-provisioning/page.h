/*
 * ZAN Tech - ESP32 Wi-Fi Setup
 * page.h - the setup page's HTML, kept out of the .ino so the actual
 * program logic there isn't cluttered with markup.
 *
 * Want to change how the setup page looks? This is the only file you need
 * to edit - nothing in the .ino needs to change to match.
 */
#pragma once

// Shown at "/" - the form the user fills in with their Wi-Fi name/password.
const char SETUP_PAGE_HTML[] = R"rawliteral(
<h2>Wi-Fi Setup</h2>
<form action='/save' method='POST'>
<input name='ssid' placeholder='Wi-Fi name'><br>
<input name='pass' type='password' placeholder='Wi-Fi password'><br>
<input type='submit' value='Connect'>
</form>
)rawliteral";

// Shown right after the form is submitted, just before the device restarts.
const char SAVED_PAGE_HTML[] = R"rawliteral(
<h3>Saved! Restarting...</h3>
)rawliteral";
