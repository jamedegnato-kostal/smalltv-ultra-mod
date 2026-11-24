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
:root{--bg:#0e1116;--card:#171c24;--mut:#8b96a5;--fg:#e6edf3;--acc:#3fb950;--acc2:#2f81f7;--red:#f85149;--bd:#262d38}
*{box-sizing:border-box}
body{margin:0;font-family:system-ui,-apple-system,Segoe UI,Roboto,sans-serif;background:var(--bg);color:var(--fg);font-size:15px;line-height:1.45}
header{padding:14px 16px;border-bottom:1px solid var(--bd);display:flex;align-items:center;gap:10px;max-width:1040px;margin:0 auto}
header h1{font-size:17px;margin:0;font-weight:600}
header .logo{width:26px;height:26px;flex:0 0 auto}
header .dot{width:9px;height:9px;border-radius:50%;background:var(--mut);flex:0 0 auto}
header .dot.ok{background:var(--acc)}
header .dot.warn{background:#e3b341}
header .spacer{flex:1}
header .help-toggle{background:#222b36;border:1px solid var(--bd);color:var(--mut);padding:7px 11px;border-radius:9px;font-size:13px;cursor:pointer}
nav{display:flex;gap:5px;padding:9px 16px;overflow-x:auto;border-bottom:1px solid var(--bd);position:sticky;top:0;background:rgba(14,17,22,.96);backdrop-filter:blur(12px);z-index:5;scrollbar-width:thin}
nav button{background:none;border:0;color:var(--mut);padding:9px 13px;border-radius:9px;font-size:14px;cursor:pointer;white-space:nowrap;transition:background .15s,color .15s}
nav button.active{background:var(--card);color:var(--fg)}
nav button:hover{color:var(--fg)}
main{padding:24px 16px 32px;max-width:900px;margin:0 auto}
.tab{display:none}.tab.active{display:block}
.card{background:var(--card);border:1px solid var(--bd);border-radius:14px;padding:18px;margin-bottom:14px;box-shadow:0 8px 24px rgba(0,0,0,.12)}
h2{font-size:14px;text-transform:uppercase;letter-spacing:.04em;color:var(--mut);margin:0 0 12px}
.welcome{display:flex;align-items:flex-start;justify-content:space-between;gap:16px;margin:2px 0 20px}
.welcome .eyebrow{margin:0 0 7px;color:var(--acc);font-size:11px;font-weight:700;letter-spacing:.12em;text-transform:uppercase}
.welcome h2{font-size:24px;letter-spacing:-.03em;text-transform:none;color:var(--fg);margin:0 0 5px}
.welcome p{margin:0;color:var(--mut);font-size:14px}
.status-pill{flex:0 0 auto;border:1px solid var(--bd);border-radius:999px;padding:7px 11px;font-size:12px;color:var(--mut);background:var(--card)}
.status-pill.ok{color:var(--acc);border-color:rgba(63,185,80,.35);background:rgba(63,185,80,.08)}
.status-pill.warn{color:#e3b341;border-color:rgba(227,179,65,.35);background:rgba(227,179,65,.08)}
.quick-grid{display:grid;grid-template-columns:repeat(3,minmax(0,1fr));gap:12px;margin-bottom:14px}
.stat-card{min-width:0;background:linear-gradient(145deg,#1a2029,#151a22);border:1px solid var(--bd);border-radius:14px;padding:16px}
.stat-card .label{color:var(--mut);font-size:11px;font-weight:650;letter-spacing:.08em;text-transform:uppercase}
.stat-card strong{display:block;margin-top:10px;font-size:17px;overflow:hidden;text-overflow:ellipsis;white-space:nowrap}
.stat-card a{display:inline-block;margin-top:5px;color:var(--acc2);font-size:13px;text-decoration:none;overflow:hidden;text-overflow:ellipsis;white-space:nowrap;max-width:100%}
.home-actions{display:flex;flex-wrap:wrap;gap:9px;margin:16px 0 22px}
.details{border-top:1px solid var(--bd);padding-top:14px}
.details summary{color:var(--mut);cursor:pointer;font-size:13px;user-select:none}
.details[open] summary{margin-bottom:12px}
.details .card{box-shadow:none;margin:0;padding:0;border:0}
label{display:block;margin:10px 0 4px;font-size:13px;color:var(--mut)}
input[type=text],input[type=password],input[type=number],input[type=url],input[type=file],select{width:100%;padding:9px 10px;background:#0b0e13;border:1px solid var(--bd);border-radius:8px;color:var(--fg);font-size:15px;min-height:40px}
input[type=file]{padding:7px}
input[type=range]{width:100%;accent-color:var(--acc2)}
.row{display:flex;gap:10px}.row>*{flex:1;min-width:0}
.chk{display:flex;align-items:center;gap:8px;margin:8px 0}
.chk input{width:18px;height:18px;accent-color:var(--acc)}
.chk label{margin:0;color:var(--fg);font-size:14px}
.req{border:1px solid var(--bd);background:#0b0e13;border-radius:8px;padding:10px 12px;margin:0 0 14px;font-size:13px;line-height:1.5;color:var(--mut)}
.req b{color:var(--fg);font-weight:600}
button.btn,.btn{display:inline-flex;align-items:center;justify-content:center;gap:6px;background:var(--acc);color:#04130a;border:0;padding:10px 16px;border-radius:9px;font-size:14px;font-weight:600;cursor:pointer;text-decoration:none;min-height:40px}
button.btn.sec,.btn.sec{background:#222b36;color:var(--fg);border:1px solid var(--bd)}
button.btn.danger,.btn.danger{background:var(--red);color:#1a0606}
button.btn:disabled{opacity:.5;cursor:not-allowed}
.muted{color:var(--mut);font-size:13px}
.status-box{white-space:pre-wrap;overflow-wrap:anywhere;font-size:13px;line-height:1.7;color:var(--mut)}
table{width:100%;border-collapse:collapse}
td{padding:6px 4px}
.wifi-row{display:grid;grid-template-columns:minmax(0,1fr) minmax(0,1fr) auto;gap:10px;align-items:end;padding:10px 0;border-bottom:1px solid var(--bd)}
.wifi-row:first-child{padding-top:0}
.wifi-row label{margin-top:0}
.wifi-row input{min-width:0}
.kv{display:flex;justify-content:space-between;gap:12px;padding:5px 0;border-bottom:1px solid var(--bd)}
.kv:last-child{border:0}.kv b{font-weight:600}
.toast{min-height:22px;margin:12px 0 0;color:var(--acc);font-size:13px;overflow-wrap:anywhere}
.toast.error{color:var(--red)}
.net{padding:8px 10px;border:1px solid var(--bd);background:#0b0e13;border-radius:8px;margin:4px 4px 4px 0;cursor:pointer;display:inline-flex;justify-content:space-between;color:var(--fg);font-size:13px}
.net:hover{border-color:var(--acc2)}
small.hint{display:block;color:var(--mut);margin-top:4px;font-size:12px;line-height:1.55}
small.hint:not([id]){display:none}
body.show-help small.hint:not([id]){display:block}
.chip{display:inline-block;margin-left:8px;padding:2px 8px;border-radius:10px;font-size:11px;font-weight:600;letter-spacing:.03em;background:var(--acc2);color:#fff;vertical-align:middle}
@media(max-width:600px){header{padding:12px}nav{padding:8px 10px}main{padding:18px 12px 28px}.quick-grid{grid-template-columns:1fr 1fr;gap:9px}.stat-card{padding:13px}.stat-card strong{font-size:15px}.welcome h2{font-size:21px}.card{padding:15px}.wifi-row{grid-template-columns:minmax(0,1fr) minmax(0,1fr)}.wifi-row button{grid-column:1/-1}}
@media(max-width:380px){.quick-grid{grid-template-columns:1fr}.welcome{display:block}.status-pill{display:inline-flex;margin-top:12px}.row{flex-direction:column;gap:0}}
</style>
</head>
<body>
<header>
 <svg class="logo" viewBox="0 0 64 64" aria-hidden="true"><g fill="none" stroke="var(--acc)" stroke-width="3" stroke-linecap="round" stroke-linejoin="round"><rect x="7" y="13" width="50" height="38" rx="6"/><polyline points="15,38 23,30 30,34 37,22 44,28 50,20"/><path d="M24 51v6M40 51v6M20 57h8M36 57h8"/></g></svg>
 <span id="dot" class="dot"></span>
 <h1>SmallTV Clean</h1>
 <span id="chip" class="chip">ESP8266</span>
 <span class="spacer"></span>
 <button id="helpToggle" class="help-toggle" type="button" aria-expanded="false">Tipps anzeigen</button>
</header>
<nav aria-label="Hauptnavigation">
 <button data-t="status" class="active" type="button">Übersicht</button>
 <button data-t="wifi" type="button">WLAN</button>
 <button data-t="system" type="button">System</button>
</nav>
<main>
 <section id="status" class="tab active">
  <div class="welcome">
   <div><p class="eyebrow">SMALLTV · WEBVERWALTUNG</p><h2>Gerätestatus</h2><p>Netzwerk und Firmware auf einen Blick.</p></div>
   <span id="statusPill" class="status-pill">Wird geprüft …</span>
  </div>
  <div class="quick-grid">
   <div class="stat-card"><span class="label">Netzwerk</span><strong id="statusNetwork">Wird geladen …</strong></div>
   <div class="stat-card"><span class="label">Geräteadresse</span><a id="statusAddress" href="http://192.168.4.1">Wird geladen …</a></div>
   <div class="stat-card"><span class="label">Laufzeit</span><strong id="statusUptime">—</strong></div>
  </div>
  <div class="home-actions">
   <button class="btn sec" type="button" data-go="wifi">WLAN einrichten</button>
   <button class="btn sec" type="button" data-go="system">Firmware &amp; System</button>
   <button class="btn sec" type="button" onclick="refreshStatus()">Status aktualisieren</button>
  </div>
  <div class="card"><h2>Technische Details</h2><div id="status" class="status-box">Status wird geladen …</div></div>
 </section>

 <section id="wifi" class="tab">
  <div class="welcome"><div><p class="eyebrow">VERBINDUNG</p><h2>WLAN-Einstellungen</h2><p>Netzwerk auswählen und Gerätenamen festlegen.</p></div></div>
  <div class="card">
   <h2>Gespeicherte Netzwerke</h2>
   <div class="home-actions" style="margin:0 0 12px"><button class="btn sec" type="button" onclick="scanWifi()">Netzwerke suchen</button><button class="btn sec" type="button" onclick="addWifi()">+ WLAN hinzufügen</button></div>
   <div id="scanList" aria-live="polite"></div>
   <div id="wifiRows"></div>
   <small class="hint">2,4-GHz-WLAN. Bis zu 4 Netzwerke. Bei mehreren gespeicherten Netzen verbindet sich das Gerät mit einem erreichbaren Netzwerk. Ein leeres Passwortfeld lässt das gespeicherte Passwort unverändert.</small>
  </div>
  <div class="card">
   <h2>Gerätename</h2>
   <label for="hostname">Hostname</label><input id="hostname" type="text" maxlength="63" placeholder="smalltv">
   <small class="hint">Das Gerät ist im lokalen Netzwerk unter <code>http://hostname.local</code> erreichbar, sofern mDNS unterstützt wird.</small>
  </div>
  <div class="card">
   <h2>Setup-Hotspot</h2>
   <label for="apSsid">Hotspot-Name</label><input id="apSsid" type="text" maxlength="32">
   <small class="hint">Der Hotspot ist unverschlüsselt und die Weboberfläche ohne Anmeldung erreichbar. Er wird zur Einrichtung bereitgestellt, wenn keine WLAN-Verbindung verfügbar ist. Standardadresse: <b>192.168.4.1</b>.</small>
   <div class="home-actions"><button class="btn" type="button" onclick="saveConfig()">Einstellungen speichern</button></div>
  </div>
 </section>

 <section id="system" class="tab">
  <div class="welcome"><div><p class="eyebrow">WARTUNG</p><h2>Firmware &amp; System</h2><p>Updates, Sicherung und Gerätewartung.</p></div></div>
  <div class="card">
   <h2>Firmware aktualisieren</h2>
   <label for="firmwareFile">Firmware-Datei (.bin)</label><input id="firmwareFile" type="file" accept=".bin,application/octet-stream">
   <div class="home-actions"><button class="btn" type="button" onclick="uploadFirmware()">Firmware installieren</button><button class="btn sec" type="button" onclick="checkUpdate()">Online-Update prüfen</button><button id="selfUpdate" class="btn" type="button" disabled onclick="startSelfUpdate()">Update installieren</button></div>
   <small class="hint">Während eines Firmware-Updates die Stromversorgung nicht trennen. Das Gerät startet nach erfolgreicher Installation automatisch neu.</small>
  </div>
  <div class="card">
   <h2>Konfiguration sichern</h2>
   <p class="muted">WLAN-Zugangsdaten und Geräteeinstellungen als JSON-Datei sichern oder eine vorhandene Sicherung wiederherstellen.</p>
   <div class="row">
    <div><label for="backupFile">Backup importieren (.json)</label><input id="backupFile" type="file" accept=".json,application/json"></div>
    <div><label>Sicherung exportieren</label><a class="btn sec" href="/api/export" style="width:100%">Konfiguration herunterladen</a></div>
   </div>
   <div class="home-actions"><button class="btn sec" type="button" onclick="importConfig()">Backup importieren</button></div>
   <small class="hint">Eine Sicherungsdatei kann WLAN-Passwörter enthalten. Bewahre sie entsprechend geschützt auf. Nach dem Import startet das Gerät neu.</small>
  </div>
  <div class="card">
   <h2>Gerätewartung</h2>
   <div class="home-actions" style="margin:0"><button class="btn sec" type="button" onclick="rebootDevice()">Neustart</button><button class="btn danger" type="button" onclick="factoryReset()">Werksreset</button></div>
   <small class="hint">Der Werksreset löscht gespeicherte WLAN-Zugangsdaten und Einstellungen und startet das Gerät anschließend neu.</small>
  </div>
 </section>
 <div id="message" class="toast" role="status" aria-live="polite"></div>
</main>
<script>
const $ = id => document.getElementById(id);
let config = {};
let latestUpdate = false;
let activeTab = "status";
async function api(path, options) {
  const response = await fetch(path, options || {});
  const raw = await response.text();
  let data = raw;
  try { data = raw ? JSON.parse(raw) : {}; } catch (_) {}
  if (!response.ok) throw new Error((data && data.error) || raw || ("HTTP " + response.status));
  return data;
}
function message(text, isError) {
  $("message").textContent = text;
  $("message").classList.toggle("error", !!isError);
}
function showTab(name) {
  const target = document.getElementById(name);
  if (!target || !target.classList.contains("tab")) return;
  activeTab = name;
  document.querySelectorAll("main .tab").forEach(tab => tab.classList.toggle("active", tab.id === name));
  document.querySelectorAll("nav [data-t]").forEach(button => button.classList.toggle("active", button.dataset.t === name));
  window.scrollTo(0, 0);
}
document.querySelectorAll("nav [data-t]").forEach(button => button.addEventListener("click", () => showTab(button.dataset.t)));
document.querySelectorAll("[data-go]").forEach(button => button.addEventListener("click", () => showTab(button.dataset.go)));
$("helpToggle").addEventListener("click", () => {
  const enabled = document.body.classList.toggle("show-help");
  $("helpToggle").setAttribute("aria-expanded", String(enabled));
  $("helpToggle").textContent = enabled ? "Tipps ausblenden" : "Tipps anzeigen";
});
function addWifi(ssid, password) {
  const row = document.createElement("div");
  row.className = "wifi-row";
  row.innerHTML = '<div><label>SSID</label><input class="ssid" type="text" maxlength="32" autocomplete="off"></div><div><label>Passwort</label><input class="password" type="password" maxlength="64" autocomplete="new-password" placeholder="leer = gespeichert lassen"></div><button class="btn danger" type="button">Entfernen</button>';
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
  showWifiRows(c.wifi || []);
}
async function load() {
  try {
    config = await api("/api/config");
    loadConfigFields(config);
    await refreshStatus();
  } catch (error) { message("Laden fehlgeschlagen: " + error.message, true); }
}
function formatUptime(seconds) {
  const total = Math.max(0, Number(seconds) || 0);
  const days = Math.floor(total / 86400);
  const hours = Math.floor((total % 86400) / 3600);
  const minutes = Math.floor((total % 3600) / 60);
  return days ? days + " T " + hours + " Std." : hours + " Std. " + minutes + " Min.";
}
async function refreshStatus() {
  try {
    const s = await api("/api/status");
    const connected = !!s.connected;
    const address = s.ip || "—";
    $("statusNetwork").textContent = connected ? (s.ssid || "Verbunden") : "Setup-Hotspot";
    $("statusAddress").textContent = address;
    $("statusAddress").href = address === "—" ? "#" : "http://" + address;
    $("statusAddress").title = address;
    $("statusUptime").textContent = formatUptime(s.uptime);
    $("statusPill").textContent = connected ? "WLAN verbunden" : "Setup-Modus";
    $("statusPill").classList.toggle("ok", connected);
    $("statusPill").classList.toggle("warn", !connected);
    $("dot").classList.toggle("ok", connected);
    $("dot").classList.toggle("warn", !connected);
    $("status").textContent =
      "Firmware: " + s.fw + " " + s.version + "\n" +
      "Variante: " + s.variant + "\n" +
      "Netzwerk: " + (connected ? (s.ssid + " · " + address) : ("Setup-Hotspot · " + address)) + "\n" +
      "Signal: " + (connected ? (s.rssi + " dBm") : "—") + "\n" +
      "Freier Heap: " + s.heap + " Byte\n" +
      "Größter freier Block: " + s.maxBlock + " Byte\n" +
      "Laufzeit: " + formatUptime(s.uptime) + "\n" +
      "Reset: " + s.reset + (s.updateMsg ? "\nUpdate: " + s.updateMsg : "");
  } catch (error) {
    $("status").textContent = "Status nicht verfügbar: " + error.message;
    $("statusPill").textContent = "Nicht erreichbar";
    $("statusPill").classList.remove("ok");
    $("statusPill").classList.add("warn");
    $("dot").classList.remove("ok");
    $("dot").classList.add("warn");
  }
}
async function scanWifi() {
  try {
    message("WLAN-Netzwerke werden gesucht …");
    const list = await api("/api/scan");
    $("scanList").textContent = "";
    if (!list.length) { $("scanList").textContent = "Keine Netzwerke gefunden."; return; }
    list.forEach(item => {
      const button = document.createElement("button");
      button.className = "net";
      button.type = "button";
      button.textContent = item.ssid + " (" + item.rssi + " dBm)" + (item.open ? " · offen" : "");
      button.onclick = () => {
        const empty = Array.from(document.querySelectorAll(".wifi-row")).find(row => !row.querySelector(".ssid").value.trim());
        if (empty) empty.querySelector(".ssid").value = item.ssid;
        else addWifi(item.ssid, "");
      };
      $("scanList").appendChild(button);
    });
    message("Netzwerksuche abgeschlossen.");
  } catch (error) { message("WLAN-Suche fehlgeschlagen: " + error.message, true); }
}
async function saveConfig() {
  const wifi = Array.from(document.querySelectorAll(".wifi-row")).map(row => ({
    ssid: row.querySelector(".ssid").value.trim(),
    pass: row.querySelector(".password").value
  })).filter(row => row.ssid);
  const body = {wifi, hostname:$("hostname").value.trim(), apSsid:$("apSsid").value.trim()};
  try {
    const result = await api("/api/config", {
      method:"POST", headers:{"Content-Type":"application/json"}, body:JSON.stringify(body)
    });
    config = Object.assign({}, config, body);
    message(result.reboot ? "Gespeichert. Netzwerkänderung: Gerät startet neu." : "Einstellungen gespeichert.");
    if (result.reboot) setTimeout(refreshStatus, 4000);
  } catch (error) { message("Speichern fehlgeschlagen: " + error.message, true); }
}
async function uploadFirmware() {
  const file = $("firmwareFile").files[0];
  if (!file) { message("Bitte eine Firmware-BIN-Datei auswählen.", true); return; }
  if (!confirm("Firmware installieren und das Gerät danach neu starten?")) return;
  const data = new FormData();
  data.append("firmware", file, file.name);
  message("Firmware wird übertragen …");
  try {
    const response = await fetch("/update", {method:"POST", body:data});
    const result = await response.text();
    if (!response.ok) throw new Error(result || ("HTTP " + response.status));
    message("Firmware übertragen. Gerät startet neu.");
  } catch (error) { message("Firmware-Update fehlgeschlagen: " + error.message, true); }
}
async function importConfig() {
  const file = $("backupFile").files[0];
  if (!file) { message("Bitte eine JSON-Sicherungsdatei auswählen.", true); return; }
  if (!confirm("Konfiguration importieren? Das Gerät startet danach neu.")) return;
  try {
    await api("/api/import", {method:"POST", headers:{"Content-Type":"application/json"}, body:await file.text()});
    message("Backup importiert. Gerät startet neu.");
  } catch (error) { message("Import fehlgeschlagen: " + error.message, true); }
}
async function checkUpdate() {
  try {
    const result = await api("/api/checkupdate");
    latestUpdate = !!(result.ok && result.newer);
    $("selfUpdate").disabled = !latestUpdate;
    message(result.ok ? (latestUpdate ? "Neue Version verfügbar: " + result.latest : "Firmware ist aktuell (" + result.current + ").") : "Update-Prüfung: " + result.error, !result.ok);
  } catch (error) { message("Update-Prüfung fehlgeschlagen: " + error.message, true); }
}
async function startSelfUpdate() {
  if (!latestUpdate || !confirm("Neueste Firmware installieren?")) return;
  try {
    await api("/api/selfupdate", {method:"POST"});
    message("Online-Update vorgemerkt. Gerät startet nach dem Download neu.");
  } catch (error) { message("Online-Update fehlgeschlagen: " + error.message, true); }
}
async function rebootDevice() {
  if (!confirm("SmallTV neu starten?")) return;
  try { await api("/api/reboot", {method:"POST"}); message("Neustart ausgelöst."); }
  catch (error) { message("Neustart fehlgeschlagen: " + error.message, true); }
}
async function factoryReset() {
  if (!confirm("Alle Einstellungen löschen und Werkseinstellungen laden?")) return;
  try { await api("/api/factory", {method:"POST"}); message("Werkseinstellungen geladen. Gerät startet neu."); }
  catch (error) { message("Reset fehlgeschlagen: " + error.message, true); }
}
load();
setInterval(refreshStatus, 10000);
</script>
</body>
</html>
)HTML";
