#pragma once
#include <Arduino.h>
static const char WEBUI_CLEAN_HTML[] PROGMEM = R"HTML(
<!doctype html>
<html lang="de">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<meta name="color-scheme" content="dark">
<title>SmallTV Clean</title>
<style>
:root{font-family:system-ui,sans-serif;color:#edf2f7;background:#101419;--panel:#1a222a;--line:#34414d;--accent:#8bc5a0;--muted:#9aa8b5}
*{box-sizing:border-box}body{margin:0;line-height:1.45}header,main{max-width:850px;margin:auto}header{padding:22px 16px;background:#161d24;border-bottom:1px solid var(--line)}h1{font-size:22px;margin:0}header p{margin:5px 0 0;color:var(--muted)}main{padding:16px}section{background:var(--panel);border:1px solid var(--line);border-radius:12px;padding:16px;margin:0 0 14px}h2{font-size:16px;margin:0 0 12px;color:var(--accent)}label{display:block;margin:12px 0 5px;color:#c7d1da;font-size:13px}input,button{font:inherit}input[type=text],input[type=password],input[type=file]{width:100%;min-height:42px;padding:8px 10px;color:#edf2f7;background:#0e1318;border:1px solid var(--line);border-radius:7px}.wifi-row{display:grid;grid-template-columns:minmax(120px,1fr) minmax(120px,1fr) auto;gap:8px;align-items:end;margin:8px 0}.wifi-row label{margin-top:0}.wifi-row button{min-height:42px}.btn{min-height:40px;padding:8px 13px;background:#2b3b34;color:#eaf6ed;border:1px solid #527361;border-radius:7px;cursor:pointer}.btn.secondary{background:#222d37;border-color:var(--line)}.btn.danger{background:#4b282b;border-color:#8c454b}.btn:disabled{opacity:.5;cursor:not-allowed}.muted{color:var(--muted);font-size:13px}.status{white-space:pre-wrap;font-size:13px;color:#c9d5df}.actions{display:flex;gap:8px;flex-wrap:wrap;margin-top:14px}.grid{display:grid;grid-template-columns:repeat(auto-fit,minmax(190px,1fr));gap:10px}.bar{height:1px;background:var(--line);margin:14px 0}#message{min-height:22px;margin:10px 0;color:var(--accent)}a{color:var(--accent)}@media(max-width:520px){.wifi-row{grid-template-columns:1fr 1fr}.wifi-row button{grid-column:1/-1}}
</style>
</head>
<body>
<header><h1>SmallTV · Clean Firmware</h1><p>Webverwaltung · WLAN · Firmware-Wartung</p></header>
<main>
<section>
<h2>Gerätestatus</h2>
<div id="status" class="status">Status wird geladen …</div>
<div class="actions"><button class="btn secondary" type="button" onclick="refreshStatus()">Aktualisieren</button><a class="btn secondary" href="/api/export">Konfiguration sichern</a></div>
</section>
<section>
<h2>WLAN und Gerätename</h2>
<div id="wifiRows"></div>
<div class="actions"><button class="btn secondary" type="button" onclick="addWifi()">WLAN hinzufügen</button><button class="btn secondary" type="button" onclick="scanWifi()">Netzwerke suchen</button></div>
<div id="scanList" class="muted"></div>
<label for="hostname">Hostname</label><input id="hostname" type="text" maxlength="63">
<div class="bar"></div>
<h2>Setup-Hotspot</h2>
<div class="grid">
<div><label for="apSsid">Hotspot-Name</label><input id="apSsid" type="text" maxlength="32"></div>
<div><label for="apPass">Hotspot-Passwort (optional)</label><input id="apPass" type="password" maxlength="63" placeholder="leer = unverändert"></div>
</div>
<p class="muted">Der ESP8266 unterstützt 2,4-GHz-WLAN. Ohne gespeicherte Verbindung startet der Hotspot unter 192.168.4.1. Die Weboberfläche ist ohne Login erreichbar.</p>
</section>
<section>
<h2>Firmware und System</h2>
<div class="grid">
<div><label for="firmwareFile">Firmware-Datei (.bin)</label><input id="firmwareFile" type="file" accept=".bin,application/octet-stream"></div>
<div><label for="backupFile">Konfigurationsdatei importieren</label><input id="backupFile" type="file" accept=".json,application/json"></div>
</div>
<div class="actions">
<button class="btn" type="button" onclick="uploadFirmware()">Firmware installieren</button>
<button class="btn secondary" type="button" onclick="importConfig()">Backup importieren</button>
<button class="btn secondary" type="button" onclick="checkUpdate()">Online-Update prüfen</button>
<button id="selfUpdate" class="btn" type="button" disabled onclick="startSelfUpdate()">Update installieren</button>
<button class="btn secondary" type="button" onclick="rebootDevice()">Neustart</button>
<button class="btn danger" type="button" onclick="factoryReset()">Werksreset</button>
</div>
<p class="muted">Während eines Firmware-Updates die Stromversorgung nicht trennen. Backups können WLAN-Zugangsdaten enthalten.</p>
</section>
<div id="message" role="status" aria-live="polite"></div>
<div class="actions"><button class="btn" type="button" onclick="saveConfig()">Einstellungen speichern</button></div>
</main>
<script>
const $ = id => document.getElementById(id);
let config = {};
let latestUpdate = false;
async function api(path, options) {
  const response = await fetch(path, options || {});
  const raw = await response.text();
  let data = raw;
  try { data = raw ? JSON.parse(raw) : {}; } catch (_) {}
  if (!response.ok) throw new Error((data && data.error) || raw || ("HTTP " + response.status));
  return data;
}
function message(text) { $("message").textContent = text; }
function addWifi(ssid, password) {
  const row = document.createElement("div");
  row.className = "wifi-row";
  row.innerHTML = '<div><label>SSID</label><input class="ssid" type="text" maxlength="32"></div><div><label>Passwort</label><input class="password" type="password" maxlength="64" placeholder="leer = gespeichert lassen"></div><button class="btn danger" type="button">Entfernen</button>';
  row.querySelector(".ssid").value = ssid || "";
  row.querySelector(".password").value = password || "";
  row.querySelector("button").onclick = () => row.remove();
  $("wifiRows").appendChild(row);
}
function showWifiRows(rows) {
  $("wifiRows").textContent = "";
  (rows && rows.length ? rows : [{ssid:""}]).forEach(row => addWifi(row.ssid, ""));
}
function loadConfigFields(c) {
  $("hostname").value = c.hostname || "smalltv";
  $("apSsid").value = c.apSsid || "SmallTV-Setup";
  $("apPass").value = "";
  showWifiRows(c.wifi || []);
}
async function load() {
  try {
    config = await api("/api/config");
    loadConfigFields(config);
    await refreshStatus();
  } catch (error) { message("Laden fehlgeschlagen: " + error.message); }
}
async function refreshStatus() {
  try {
    const s = await api("/api/status");
    $("status").textContent =
      "Firmware: " + s.fw + " " + s.version + "\n" +
      "Netzwerk: " + (s.connected ? (s.ssid + " · " + s.ip) : ("Setup-Modus · " + s.ip)) + "\n" +
      "Signal: " + (s.connected ? (s.rssi + " dBm") : "—") + "\n" +
      "Freier Heap: " + s.heap + " Byte\n" +
      "Größter freier Block: " + s.maxBlock + " Byte\n" +
      "Laufzeit: " + Math.floor((s.uptime || 0) / 3600) + " h " + Math.floor(((s.uptime || 0) % 3600) / 60) + " min\n" +
      "Reset: " + s.reset + (s.updateMsg ? "\nUpdate: " + s.updateMsg : "");
  } catch (error) { $("status").textContent = "Status nicht verfügbar: " + error.message; }
}
async function scanWifi() {
  try {
    message("WLAN-Netzwerke werden gesucht …");
    const list = await api("/api/scan");
    $("scanList").textContent = "";
    if (!list.length) { $("scanList").textContent = "Keine Netzwerke gefunden."; return; }
    list.forEach(item => {
      const button = document.createElement("button");
      button.className = "btn secondary";
      button.type = "button";
      button.style.margin = "4px";
      button.textContent = item.ssid + " (" + item.rssi + " dBm)";
      button.onclick = () => addWifi(item.ssid, "");
      $("scanList").appendChild(button);
    });
    message("Netzwerksuche abgeschlossen.");
  } catch (error) { message("WLAN-Suche fehlgeschlagen: " + error.message); }
}
async function saveConfig() {
  const wifi = Array.from(document.querySelectorAll(".wifi-row")).map(row => ({
    ssid: row.querySelector(".ssid").value.trim(),
    pass: row.querySelector(".password").value
  })).filter(row => row.ssid);
  const body = {wifi, hostname:$("hostname").value.trim(), apSsid:$("apSsid").value.trim()};
  if ($("apPass").value) body.apPass = $("apPass").value;
  try {
    const result = await api("/api/config", {
      method:"POST", headers:{"Content-Type":"application/json"}, body:JSON.stringify(body)
    });
    config = Object.assign({}, config, body);
    message(result.reboot ? "Gespeichert. Netzwerkänderung: Gerät startet neu." : "Einstellungen gespeichert.");
    if (result.reboot) setTimeout(refreshStatus, 4000);
  } catch (error) { message("Speichern fehlgeschlagen: " + error.message); }
}
async function uploadFirmware() {
  const file = $("firmwareFile").files[0];
  if (!file) { message("Bitte eine Firmware-BIN-Datei auswählen."); return; }
  if (!confirm("Firmware installieren und das Gerät danach neu starten?")) return;
  const data = new FormData();
  data.append("firmware", file, file.name);
  message("Firmware wird übertragen …");
  try {
    const response = await fetch("/update", {method:"POST", body:data});
    const result = await response.text();
    if (!response.ok) throw new Error(result || ("HTTP " + response.status));
    message("Firmware übertragen. Gerät startet neu.");
  } catch (error) { message("Firmware-Update fehlgeschlagen: " + error.message); }
}
async function importConfig() {
  const file = $("backupFile").files[0];
  if (!file) { message("Bitte eine JSON-Sicherungsdatei auswählen."); return; }
  if (!confirm("Konfiguration importieren? Das Gerät startet danach neu.")) return;
  try {
    await api("/api/import", {method:"POST", headers:{"Content-Type":"application/json"}, body:await file.text()});
    message("Backup importiert. Gerät startet neu.");
  } catch (error) { message("Import fehlgeschlagen: " + error.message); }
}
async function checkUpdate() {
  try {
    const result = await api("/api/checkupdate");
    latestUpdate = !!(result.ok && result.newer);
    $("selfUpdate").disabled = !latestUpdate;
    message(result.ok ? (latestUpdate ? "Neue Version verfügbar: " + result.latest : "Firmware ist aktuell (" + result.current + ").") : "Update-Prüfung: " + result.error);
  } catch (error) { message("Update-Prüfung fehlgeschlagen: " + error.message); }
}
async function startSelfUpdate() {
  if (!latestUpdate || !confirm("Neueste Firmware installieren?")) return;
  try {
    await api("/api/selfupdate", {method:"POST"});
    message("Online-Update vorgemerkt. Gerät startet nach dem Download neu.");
  } catch (error) { message("Online-Update fehlgeschlagen: " + error.message); }
}
async function rebootDevice() {
  if (!confirm("SmallTV neu starten?")) return;
  try { await api("/api/reboot", {method:"POST"}); message("Neustart ausgelöst."); }
  catch (error) { message("Neustart fehlgeschlagen: " + error.message); }
}
async function factoryReset() {
  if (!confirm("Alle Einstellungen löschen und Werkseinstellungen laden?")) return;
  try { await api("/api/factory", {method:"POST"}); message("Werkseinstellungen geladen. Gerät startet neu."); }
  catch (error) { message("Reset fehlgeschlagen: " + error.message); }
}
load();
setInterval(refreshStatus, 10000);
</script>
</body>
</html>
)HTML";
