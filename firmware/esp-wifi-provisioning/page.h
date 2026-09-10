/*
 * ZAN Tech - ESP32 Wi-Fi Setup
 * page.h - the setup page's HTML, kept out of the .ino so the actual
 * program logic there isn't cluttered with markup.
 *
 * Want to change how the setup page looks? This is the only file you need
 * to edit - nothing in the .ino needs to change to match. Both pages are
 * fully self-contained (inline CSS/JS, no external fonts or CDNs) since
 * the device has no internet access while its own hotspot is active.
 */
#pragma once

// Shown at "/" - the form the user fills in with their Wi-Fi name/password.
const char SETUP_PAGE_HTML[] = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1, maximum-scale=1">
<title>ZAN Tech Wi-Fi Setup</title>
<style>
  :root { --brand: #0EA5A4; --brand-dark: #0b7f7e; --text: #0f172a; --muted: #64748b; }
  * { box-sizing: border-box; }
  .brand { display: flex; align-items: center; justify-content: center; gap: 8px; margin-bottom: 20px; }
  .brand-dot { width: 8px; height: 8px; border-radius: 50%; background: var(--brand); }
  .brand-name { font-size: 0.8em; font-weight: 700; letter-spacing: 0.4px; color: var(--muted); }
  body {
    margin: 0; min-height: 100vh; padding: 24px;
    display: flex; align-items: center; justify-content: center;
    font-family: -apple-system, "Segoe UI", Roboto, sans-serif;
    background: linear-gradient(160deg, var(--brand), var(--brand-dark));
  }
  .card {
    background: #fff; border-radius: 20px; padding: 32px 28px;
    max-width: 380px; width: 100%; box-shadow: 0 20px 40px rgba(0,0,0,0.25);
  }
  .icon { display: block; margin: 0 auto 14px; width: 52px; height: 52px; }
  h1 { margin: 0 0 4px; font-size: 1.4em; color: var(--text); text-align: center; }
  p.sub { margin: 0 0 26px; color: var(--muted); font-size: 0.92em; text-align: center; }
  label { display: block; margin: 16px 0 6px; font-size: 0.85em; font-weight: 600; color: #334155; }
  input[type="text"], input[type="password"] {
    width: 100%; padding: 13px 14px; border: 1.5px solid #e2e8f0; border-radius: 12px;
    font-size: 1em; outline: none; transition: border-color 0.15s;
  }
  input:focus { border-color: var(--brand); }
  .field { position: relative; }
  .field input { padding-right: 60px; }
  .toggle-pass {
    position: absolute; right: 8px; top: 50%; transform: translateY(-50%);
    background: none; border: none; color: var(--brand-dark); font-size: 0.82em;
    font-weight: 600; cursor: pointer; padding: 6px 4px;
  }
  button[type="submit"] {
    margin-top: 26px; width: 100%; padding: 15px; border: none; border-radius: 12px;
    background: var(--brand); color: #fff; font-size: 1.05em; font-weight: 700; cursor: pointer;
  }
  button[type="submit"]:active { opacity: 0.85; }
  button[type="submit"]:disabled { opacity: 0.6; cursor: default; }
  p.hint { margin: 18px 0 0; color: #94a3b8; font-size: 0.78em; text-align: center; }
</style>
</head>
<body>
  <div class="card">
    <div class="brand"><span class="brand-dot"></span><span class="brand-name">ZAN TECH</span></div>
    <svg class="icon" viewBox="0 0 24 24" fill="none" stroke="#0EA5A4" stroke-width="2" stroke-linecap="round">
      <path d="M4 11.5a12 12 0 0 1 16 0"/>
      <path d="M7.3 15a7.5 7.5 0 0 1 9.4 0"/>
      <path d="M10.6 18.4a3 3 0 0 1 2.8 0"/>
      <circle cx="12" cy="20.2" r="1" fill="#0EA5A4" stroke="none"/>
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
const char SAVED_PAGE_HTML[] = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1, maximum-scale=1">
<title>ZAN Tech Wi-Fi Setup</title>
<style>
  :root { --brand: #0EA5A4; --brand-dark: #0b7f7e; --text: #0f172a; --muted: #64748b; }
  * { box-sizing: border-box; }
  .brand { display: flex; align-items: center; justify-content: center; gap: 8px; margin-bottom: 20px; }
  .brand-dot { width: 8px; height: 8px; border-radius: 50%; background: var(--brand); }
  .brand-name { font-size: 0.8em; font-weight: 700; letter-spacing: 0.4px; color: var(--muted); }
  body {
    margin: 0; min-height: 100vh; padding: 24px;
    display: flex; align-items: center; justify-content: center;
    font-family: -apple-system, "Segoe UI", Roboto, sans-serif;
    background: linear-gradient(160deg, var(--brand), var(--brand-dark));
  }
  .card {
    background: #fff; border-radius: 20px; padding: 36px 28px;
    max-width: 380px; width: 100%; box-shadow: 0 20px 40px rgba(0,0,0,0.25);
    text-align: center;
  }
  .icon { display: block; margin: 0 auto 16px; width: 56px; height: 56px; }
  h1 { margin: 0 0 8px; font-size: 1.4em; color: var(--text); }
  p.sub { margin: 0; color: var(--muted); font-size: 0.95em; line-height: 1.5; }
  p.hint { margin: 20px 0 0; color: #94a3b8; font-size: 0.8em; line-height: 1.5; }
</style>
</head>
<body>
  <div class="card">
    <div class="brand"><span class="brand-dot"></span><span class="brand-name">ZAN TECH</span></div>
    <svg class="icon" viewBox="0 0 24 24" fill="none" stroke="#0EA5A4" stroke-width="2" stroke-linecap="round" stroke-linejoin="round">
      <circle cx="12" cy="12" r="10" stroke="#0EA5A4"/>
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
