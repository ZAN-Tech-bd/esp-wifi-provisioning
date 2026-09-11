/*
 * ZanWifiSetup - internal setup-page HTML.
 * -----------------------------------------
 * Not part of the public API - ZanWifiSetup.h doesn't include this on
 * purpose. Colors match zantechbd.com's actual theme (dark navy
 * background, cyan as the primary accent, red reserved for the "TECH"
 * half of the logo) - verified against the live site's computed styles,
 * not guessed.
 *
 * Want to change how the setup page looks? Edit this file - nothing else
 * in the library needs to change to match, as long as the form still posts
 * "ssid" and "pass" to /save.
 */
#pragma once

// Shown at "/" - the form the user fills in with their Wi-Fi name/password.
const char ZAN_WIFI_SETUP_PAGE_HTML[] = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1, maximum-scale=1">
<meta name="theme-color" content="#0B0F19">
<title>ZAN Tech Wi-Fi Setup</title>
<style>
  :root {
    --bg: #0B0F19; --panel: #151B2B; --cyan: #00F0FF; --red: #ED2626;
    --text: #FFFFFF; --muted: #9CA3AF;
  }
  * { box-sizing: border-box; }
  body {
    margin: 0; min-height: 100vh; padding: 24px;
    display: flex; align-items: center; justify-content: center;
    font-family: -apple-system, "Segoe UI", Roboto, sans-serif;
    background: radial-gradient(circle at 50% 0%, rgba(0,240,255,0.14), transparent 60%), var(--bg);
  }
  .card {
    background: var(--panel); border: 1px solid rgba(0,240,255,0.18); border-radius: 20px;
    padding: 32px 28px; max-width: 380px; width: 100%;
    box-shadow: 0 20px 50px rgba(0,0,0,0.5), 0 0 30px rgba(0,240,255,0.07);
  }
  .brand { display: flex; align-items: center; justify-content: center; gap: 6px; margin-bottom: 20px; }
  .brand-zan { font-size: 0.95em; font-weight: 800; letter-spacing: 0.5px; color: var(--text); }
  .brand-tech { font-size: 0.95em; font-weight: 800; letter-spacing: 0.5px; color: var(--red); }
  .icon { display: block; margin: 0 auto 14px; width: 52px; height: 52px; }
  h1 { margin: 0 0 4px; font-size: 1.4em; color: var(--text); text-align: center; }
  p.sub { margin: 0 0 26px; color: var(--muted); font-size: 0.92em; text-align: center; }
  label { display: block; margin: 16px 0 6px; font-size: 0.85em; font-weight: 600; color: var(--muted); }
  input[type="text"], input[type="password"] {
    width: 100%; padding: 13px 14px; border: 1.5px solid rgba(255,255,255,0.14); border-radius: 12px;
    font-size: 1em; outline: none; transition: border-color 0.15s, box-shadow 0.15s;
    background: rgba(255,255,255,0.04); color: var(--text);
  }
  input::placeholder { color: #5b6472; }
  input:focus { border-color: var(--cyan); box-shadow: 0 0 0 3px rgba(0,240,255,0.15); }
  .field { position: relative; }
  .field input { padding-right: 60px; }
  .toggle-pass {
    position: absolute; right: 8px; top: 50%; transform: translateY(-50%);
    background: none; border: none; color: var(--cyan); font-size: 0.82em;
    font-weight: 600; cursor: pointer; padding: 6px 4px;
  }
  button[type="submit"] {
    margin-top: 26px; width: 100%; padding: 15px; border: none; border-radius: 12px;
    background: var(--cyan); color: var(--bg); font-size: 1.05em; font-weight: 700; cursor: pointer;
  }
  button[type="submit"]:active { opacity: 0.85; }
  button[type="submit"]:disabled { opacity: 0.5; cursor: default; }
  p.hint { margin: 18px 0 0; color: #5b6472; font-size: 0.78em; text-align: center; }
</style>
</head>
<body>
  <div class="card">
    <div class="brand"><span class="brand-zan">ZAN</span><span class="brand-tech">TECH</span></div>
    <svg class="icon" viewBox="0 0 24 24" fill="none" stroke="#00F0FF" stroke-width="2" stroke-linecap="round">
      <path d="M4 11.5a12 12 0 0 1 16 0"/>
      <path d="M7.3 15a7.5 7.5 0 0 1 9.4 0"/>
      <path d="M10.6 18.4a3 3 0 0 1 2.8 0"/>
      <circle cx="12" cy="20.2" r="1" fill="#00F0FF" stroke="none"/>
    </svg>
    <h1>Connect to Wi-Fi</h1>
    <p class="sub">Enter your Wi-Fi details so this device can join your network.</p>

    <form action="/save" method="POST" onsubmit="return onSetupSubmit(this)">
      <label for="ssid">Wi-Fi name</label>
      <input type="text" id="ssid" name="ssid" placeholder="e.g. Home-WiFi"
             autocapitalize="none" autocorrect="off" autocomplete="off" required>

      <label for="pass">Wi-Fi password</label>
      <div class="field">
        <input type="password" id="pass" name="pass" placeholder="Leave blank if open"
               autocapitalize="none" autocorrect="off">
        <button type="button" class="toggle-pass" onclick="togglePassword()">Show</button>
      </div>

      <button type="submit" id="submitBtn">Connect</button>
    </form>
    <p class="hint">Saved only on this device - never sent anywhere else.</p>
  </div>
  <script>
    function togglePassword() {
      var field = document.getElementById('pass');
      var btn = document.querySelector('.toggle-pass');
      var showing = field.type === 'text';
      field.type = showing ? 'password' : 'text';
      btn.textContent = showing ? 'Show' : 'Hide';
    }
    function onSetupSubmit(form) {
      var btn = document.getElementById('submitBtn');
      btn.disabled = true;
      btn.textContent = 'Connecting…';
      return true;
    }
  </script>
</body>
</html>
)rawliteral";

// Shown right after the form is submitted, just before the device restarts.
const char ZAN_WIFI_SAVED_PAGE_HTML[] = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1, maximum-scale=1">
<meta name="theme-color" content="#0B0F19">
<title>ZAN Tech Wi-Fi Setup</title>
<style>
  :root {
    --bg: #0B0F19; --panel: #151B2B; --cyan: #00F0FF; --red: #ED2626; --green: #39FF14;
    --text: #FFFFFF; --muted: #9CA3AF;
  }
  * { box-sizing: border-box; }
  body {
    margin: 0; min-height: 100vh; padding: 24px;
    display: flex; align-items: center; justify-content: center;
    font-family: -apple-system, "Segoe UI", Roboto, sans-serif;
    background: radial-gradient(circle at 50% 0%, rgba(57,255,20,0.10), transparent 60%), var(--bg);
  }
  .card {
    background: var(--panel); border: 1px solid rgba(57,255,20,0.2); border-radius: 20px;
    padding: 36px 28px; max-width: 380px; width: 100%;
    box-shadow: 0 20px 50px rgba(0,0,0,0.5), 0 0 30px rgba(57,255,20,0.08);
    text-align: center;
  }
  .brand { display: flex; align-items: center; justify-content: center; gap: 6px; margin-bottom: 20px; }
  .brand-zan { font-size: 0.95em; font-weight: 800; letter-spacing: 0.5px; color: var(--text); }
  .brand-tech { font-size: 0.95em; font-weight: 800; letter-spacing: 0.5px; color: var(--red); }
  .icon { display: block; margin: 0 auto 16px; width: 56px; height: 56px; }
  h1 { margin: 0 0 8px; font-size: 1.4em; color: var(--text); }
  p.sub { margin: 0; color: var(--muted); font-size: 0.95em; line-height: 1.5; }
  p.hint { margin: 20px 0 0; color: #5b6472; font-size: 0.8em; line-height: 1.5; }
</style>
</head>
<body>
  <div class="card">
    <div class="brand"><span class="brand-zan">ZAN</span><span class="brand-tech">TECH</span></div>
    <svg class="icon" viewBox="0 0 24 24" fill="none" stroke="#39FF14" stroke-width="2" stroke-linecap="round" stroke-linejoin="round">
      <circle cx="12" cy="12" r="10" stroke="#39FF14"/>
      <path d="M8 12.5l2.5 2.5L16 9.5"/>
    </svg>
    <h1>Saved!</h1>
    <p class="sub">Restarting and connecting to your Wi-Fi now.</p>
    <p class="hint">If this device was using its own hotspot, your phone may lose that
      connection in a moment - that's expected. Reconnect your phone to your normal
      Wi-Fi. If the password was wrong, the device's hotspot will come back so you
      can try again.</p>
  </div>
</body>
</html>
)rawliteral";
