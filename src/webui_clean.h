#pragma once
#include <Arduino.h>
static const char WEBUI_CLEAN_HTML[] PROGMEM = R"HTML(
<!doctype html>
<html lang="de">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<meta name="color-scheme" content="dark light">
<title>SmallTV Settings</title>
<style>

:root{--bg:#101611;--card:#19221a;--mut:#9aa99b;--fg:#edf4ed;--acc:#72c77c;--acc2:#a0d9a5;--red:#ed7373;--bd:#2c3a2e}
*{box-sizing:border-box}
body{margin:0;padding-bottom:76px;font-family:system-ui,-apple-system,Segoe UI,Roboto,sans-serif;background:var(--bg);color:var(--fg);font-size:15px}
header{padding:14px 16px;border-bottom:1px solid var(--bd);display:flex;align-items:center;gap:10px;max-width:1040px;margin:0 auto}
header h1{font-size:17px;margin:0;font-weight:600}
header .logo{width:26px;height:26px;flex:0 0 auto}
header .dot{width:9px;height:9px;border-radius:50%;background:var(--mut)}
header .dot.ok{background:var(--acc)}
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
.stat-card{min-width:0;background:linear-gradient(145deg,#1d281f,#182119);border:1px solid var(--bd);border-radius:14px;padding:16px}
.stat-card .label{color:var(--mut);font-size:11px;font-weight:650;letter-spacing:.08em;text-transform:uppercase}
.stat-card strong{display:block;margin-top:10px;font-size:17px;overflow:hidden;text-overflow:ellipsis;white-space:nowrap}
.stat-card a{display:inline-block;margin-top:5px;color:var(--acc2);font-size:13px;text-decoration:none}
.home-actions{display:flex;flex-wrap:wrap;gap:9px;margin:16px 0 22px}
.details{border-top:1px solid var(--bd);padding-top:14px}
.details summary{color:var(--mut);cursor:pointer;font-size:13px;user-select:none}
.details[open] summary{margin-bottom:12px}
.details .card{box-shadow:none;margin:0;padding:0;border:0}
label{display:block;margin:10px 0 4px;font-size:13px;color:var(--mut)}
input[type=text],input[type=password],input[type=number],input[type=url],input[type=search],input[type=time],select{
 width:100%;padding:9px 10px;background:#0b0e13;border:1px solid var(--bd);border-radius:8px;color:var(--fg);font-size:15px}
input[type=range]{width:100%}
.row{display:flex;gap:10px}.row>*{flex:1}
.chk{display:flex;align-items:center;gap:8px;margin:8px 0}
.chk input{width:18px;height:18px}
.chk label{margin:0;color:var(--fg);font-size:14px}
.req{border:1px solid var(--bd);background:#0b0e13;border-radius:8px;padding:10px 12px;margin:0 0 14px;font-size:13px;line-height:1.5;color:var(--mut)}
.req b{color:var(--fg);font-weight:600}
button.btn{background:var(--acc);color:#04130a;border:0;padding:10px 16px;border-radius:9px;font-size:15px;font-weight:600;cursor:pointer}
button.btn.sec{background:#222b36;color:var(--fg)}
button.btn.danger{background:var(--red);color:#1a0606}
button.btn:disabled{opacity:.5}
.muted{color:var(--mut);font-size:13px}
table{width:100%;border-collapse:collapse}
td{padding:6px 4px}
.symrow input{margin:0}
.kv{display:flex;justify-content:space-between;padding:5px 0;border-bottom:1px solid var(--bd)}
.kv:last-child{border:0}.kv b{font-weight:600}
.toast{position:fixed;bottom:16px;left:50%;transform:translateX(-50%);background:#0b0e13;border:1px solid var(--bd);padding:10px 16px;border-radius:10px;opacity:0;transition:.3s;pointer-events:none}
.toast.show{opacity:1}
.net{padding:8px;border:1px solid var(--bd);border-radius:8px;margin:4px 0;cursor:pointer;display:flex;justify-content:space-between}
.net:hover{border-color:var(--acc2)}
.bar{height:8px;background:#0b0e13;border-radius:6px;overflow:hidden;margin-top:8px}
.bar>div{height:100%;width:0;background:var(--acc2);transition:.2s}
small.hint{display:block;color:var(--mut);margin-top:4px;font-size:12px}
small.hint:not([id]){display:none;line-height:1.55}
body.show-help small.hint:not([id]){display:block}
.chip{display:inline-block;margin-left:8px;padding:2px 8px;border-radius:10px;font-size:11px;font-weight:600;letter-spacing:.03em;background:var(--acc2);color:#fff;vertical-align:middle}
.savebar{position:fixed;z-index:20;left:0;right:0;bottom:0;padding:10px 16px;background:rgba(14,17,22,.96);border-top:1px solid var(--bd)}body.light .savebar{background:rgba(244,249,244,.97)}
.savebar-inner{max-width:900px;margin:auto;display:flex;align-items:center;justify-content:flex-end;gap:12px}.save-status{margin-right:auto;color:var(--mut);font-size:13px}.save-status.dirty{color:#e3b341}.savebar button{min-width:176px}
dialog{width:min(560px,calc(100vw - 28px));max-height:85vh;overflow:auto;background:var(--card);color:var(--fg);border:1px solid var(--bd);border-radius:16px;padding:20px;box-shadow:0 24px 80px rgba(0,0,0,.45)}
body.light{--bg:#f2f7f1;--card:#fff;--mut:#59695d;--fg:#1b2b20;--bd:#d6e3d7;--acc:#338b4f;--acc2:#347b50;--red:#bd3f43}
body.light nav{background:rgba(242,247,241,.96)}body.light .stat-card,body.light .theme-tile{background:linear-gradient(145deg,#fff,#f5f9f4)}
body.light .stat-card{box-shadow:0 5px 18px rgba(28,66,36,.06)}
body.light input[type=text],body.light input[type=password],body.light input[type=number],body.light input[type=url],body.light input[type=search],body.light input[type=time],body.light select,body.light .req{background:#f8fbf8;color:var(--fg)}
body.light button.btn.sec{background:#e7f0e7;color:#25432c}body.light header .help-toggle{background:#edf4ec;color:var(--mut)}
/* Shared visual system: calm office colours, clear hierarchy and accessible controls. */
:root{color-scheme:dark;--field:#0b0e13;--shadow:0 10px 28px rgba(0,0,0,.14);--focus:rgba(114,199,124,.28)}
body{line-height:1.5;padding-bottom:88px}
body.light{color-scheme:light;--field:#f8fbf8;--shadow:0 8px 24px rgba(28,66,36,.055);--focus:rgba(51,139,79,.2)}
header{max-width:1120px;min-height:68px;padding:16px 22px;gap:12px}header h1{font-size:18px;letter-spacing:-.02em}header .logo{width:30px;height:30px}
nav{padding:10px max(22px,calc((100vw - 1120px)/2));gap:6px}nav button{min-height:42px;padding:10px 14px;border-radius:10px;font-weight:550}nav button.active{box-shadow:inset 0 0 0 1px var(--bd)}
main{max-width:1040px;padding:30px 22px 40px}.card{padding:22px;margin-bottom:16px;border-radius:16px;box-shadow:var(--shadow)}.card h2{color:var(--fg);font-size:14px;letter-spacing:.02em;text-transform:none;margin-bottom:15px}
.page-intro{margin:0 0 22px}.page-intro .eyebrow{margin:0 0 5px;color:var(--acc2);font-size:11px;font-weight:700;letter-spacing:.12em;text-transform:uppercase}.page-intro h2{margin:0;color:var(--fg);font-size:26px;line-height:1.2;letter-spacing:-.035em;text-transform:none}.page-intro p:last-child{max-width:680px;margin:8px 0 0;color:var(--mut);font-size:14px}
label{margin:14px 0 6px;font-weight:550}input[type=text],input[type=password],input[type=number],input[type=url],input[type=search],input[type=time],select{min-height:42px;padding:9px 11px;border-radius:10px;background:var(--field)}
button.btn{padding:11px 17px;border-radius:10px;font-size:14px;font-weight:650;transition:filter .15s,transform .15s,box-shadow .15s}button.btn:hover:not(:disabled){filter:brightness(1.06);transform:translateY(-1px)}button.btn:disabled{opacity:.48;cursor:not-allowed}button.btn.sec{border:1px solid var(--bd)}button.btn.danger{background:#c84d50;color:#fff}body.light button.btn.danger{background:#bd3f43}
:focus-visible{outline:3px solid var(--acc);outline-offset:3px;box-shadow:0 0 0 4px var(--focus)}input[type=range],input[type=checkbox]{accent-color:var(--acc)}
.savebar{padding:12px 22px;backdrop-filter:blur(14px)}body.light .savebar{background:rgba(242,247,241,.95)}.savebar-inner{max-width:996px;min-height:46px}.savebar button{min-width:190px;min-height:44px}.save-status.dirty{font-weight:600}
.toast{z-index:30;bottom:82px;max-width:min(560px,calc(100vw - 32px));text-align:center;line-height:1.4;box-shadow:var(--shadow)}.status-pill{font-weight:600}.stat-card{box-shadow:var(--shadow)}.details summary{padding:9px 0}
@media(min-width:900px){.quick-grid{gap:14px}.card{padding:24px}.theme-grid{grid-template-columns:repeat(auto-fit,minmax(230px,1fr))}}
@media(prefers-reduced-motion:reduce){*,*::before,*::after{scroll-behavior:auto!important;transition-duration:.01ms!important;animation-duration:.01ms!important;animation-iteration-count:1!important}}
@media(max-width:600px){body{padding-bottom:92px}header{padding:12px 14px;min-height:58px}header h1{font-size:15px}header .logo{width:25px;height:25px}header .help-toggle{padding:7px 9px}nav{padding:8px 10px;gap:3px}nav button{padding:9px 11px;font-size:13px}main{padding:22px 13px 30px}.quick-grid{grid-template-columns:1fr 1fr;gap:9px}.stat-card{padding:13px}.stat-card strong{font-size:15px}.welcome h2{font-size:21px}.page-intro h2{font-size:23px}.card{padding:17px;border-radius:14px}.savebar{padding:8px 12px}.savebar-inner{gap:8px}.save-status{font-size:11px;line-height:1.3}.savebar button{min-width:0;white-space:nowrap;padding:10px 12px}.row{flex-wrap:wrap}.row>*{min-width:min(100%,220px)}.photo-upload-actions button{flex:1 1 140px}.dialog-actions{position:sticky;bottom:-20px;background:var(--card);padding:12px 0 0}}

/* Keep feedback messages readable in the light theme. */
.toast{color:var(--fg)}
body.light .toast{background:var(--card);color:var(--fg);border-color:var(--bd)}
body.light .status-pill.warn{color:#80520a;border-color:rgba(128,82,10,.28);background:rgba(227,179,65,.15)}
body.light .save-status.dirty{color:#80520a}


/* Clean control UI: consistent green surfaces in both colour modes. */
:root{--field:#0b100c;--surface2:#1d281f;--nav:rgba(14,20,15,.97);--warn:#e3b341}
body.light{--field:#f8fbf8;--surface2:#f5f9f4;--nav:rgba(242,247,241,.97)}
nav{background:var(--nav)}
input[type=text],input[type=password],input[type=file],select{background:var(--field);color:var(--fg)}
.stat-card{background:var(--surface2)}
button:focus-visible,a:focus-visible,input:focus-visible{outline:3px solid var(--acc);outline-offset:3px}
button.btn:disabled{filter:saturate(.35)}
.status-pill.bad{color:var(--red);border-color:var(--red);background:rgba(237,115,115,.08)}
.status-pill.warn{color:var(--warn)}
.toast{max-width:min(560px,calc(100vw - 28px));z-index:30}
.toast.error{border-color:var(--red)}
.toast.success{border-color:var(--acc)}
.net{width:100%;min-height:44px;background:var(--card);color:var(--fg);text-align:left}
.stat-card .sub{display:block;margin-top:5px;color:var(--mut);font-size:12px}
.bar>div{will-change:width}
@media(max-width:600px){.quick-grid{grid-template-columns:repeat(2,minmax(0,1fr))}.status-pill{white-space:normal;text-align:center}}
</style>
</head>
<body>
<header>
 <svg class="logo" viewBox="0 0 64 64" aria-hidden="true"><g fill="none" stroke="var(--acc)" stroke-width="3" stroke-linecap="round" stroke-linejoin="round"><rect x="7" y="13" width="50" height="38" rx="6"/><polyline points="15,38 23,30 30,34 37,22 44,28 50,20"/><path d="M24 51v6M40 51v6M20 57h8M36 57h8"/></g></svg>
 <span id="dot" class="dot"></span><h1>SmallTV · Arbeitsplatz</h1>
 <span id="chip" class="chip" style="display:none"></span><span id="hi" class="muted"></span><span class="spacer"></span>
 <button id="themeMode" class="help-toggle" type="button">Helles Design</button>
 <button id="helpToggle" class="help-toggle" type="button" aria-expanded="false">Tipps</button>
</header>
<nav aria-label="Hauptnavigation">
 <button data-t="status" class="active" aria-current="page" type="button">Übersicht</button>
 <button data-t="wifi" type="button">Netzwerk</button>
 <button data-t="update" type="button">System</button>
</nav>
<main>
 <section id="status" class="tab active">
  <div class="welcome">
   <div><p class="eyebrow">SMALLTV · GERÄTEÜBERSICHT</p><h2>Gerätestatus</h2><p>Verbindung und Firmware auf einen Blick.</p></div>
   <span id="statusPill" class="status-pill">Status wird geprüft</span>
  </div>
  <div class="quick-grid">
   <div class="stat-card"><span class="label">WLAN</span><strong id="statusNetwork">Wird geladen …</strong></div>
   <div class="stat-card"><span class="label">Geräteadresse</span><a id="statusAddress" href="#">Adresse öffnen</a></div>
   <div class="stat-card"><span class="label">Laufzeit</span><strong id="statusUptime">—</strong></div><div class="stat-card"><span class="label">Freier RAM</span><strong id="statusHeap">—</strong><span id="statusHeapSub" class="sub">Größter Block: —</span></div>
  </div>
  <div class="home-actions">
   <button class="btn sec" type="button" data-go="wifi">WLAN einrichten</button>
   <button class="btn sec" type="button" data-go="update">System &amp; Firmware</button>
   <button class="btn sec" type="button" onclick="loadStatus()">Status aktualisieren</button>
  </div>
  <details class="details"><summary>Technische Details</summary><div class="card"><div id="statusBox" class="muted">Wird geladen …</div></div></details>
 </section>

 <section id="wifi" class="tab">
  <div class="page-intro"><p class="eyebrow">Verbindung</p><h2>Netzwerk</h2><p>WLAN-Zugang und Gerätenamen konfigurieren.</p></div>
  <div class="card">
   <h2>Gespeicherte WLAN-Netzwerke</h2>
   <button class="btn sec" type="button" onclick="scan()">Netzwerke suchen</button>
   <div id="scanList" aria-live="polite"></div>
   <table id="wifiTable"></table>
   <button class="btn sec" style="margin-top:10px" type="button" onclick="addWifi()">+ Netzwerk hinzufügen</button>
   <small class="hint">Nur 2,4 GHz. Bis zu vier Netzwerke; beim Start verbindet sich das Gerät mit einem erreichbaren Netzwerk. Tippe ein Suchergebnis an, um es zu übernehmen. Ein leeres Passwortfeld behält das gespeicherte Passwort.</small>
  </div>
  <div class="card">
   <h2>Gerätename</h2>
   <label for="hostname">Hostname</label><input id="hostname" type="text" maxlength="63" placeholder="smalltv">
   <small class="hint">Im lokalen Netzwerk unter <code>http://&lt;hostname&gt;.local</code> erreichbar, sofern mDNS unterstützt wird.</small>
  </div>
  <div class="card">
   <h2>Einrichtungs-Hotspot</h2>
   <label for="apSsid">Hotspot-Name</label><input id="apSsid" type="text" maxlength="32">
   <small class="hint">Der Hotspot ist unverschlüsselt und die Weboberfläche ohne Anmeldung erreichbar. Er wird zur Einrichtung bereitgestellt, wenn keine WLAN-Verbindung verfügbar ist. Standardadresse: <b>192.168.4.1</b>.</small>
  </div>
 </section>

 <section id="update" class="tab">
  <div class="page-intro"><p class="eyebrow">Geräteverwaltung</p><h2>System</h2><p>Firmware aktualisieren, Einstellungen sichern und das Gerät warten.</p></div>
  <div class="card">
   <h2>Online-Update</h2>
   <div class="muted">Installiert: <b id="fwVer">wird geladen …</b></div>
   <div style="margin-top:10px">
    <button class="btn sec" type="button" onclick="checkUpdate()" id="chkBtn">Nach Update suchen</button>
    <button class="btn" style="margin-left:8px" type="button" onclick="selfUpdate()" id="ghUpBtn" disabled>Jetzt aktualisieren</button>
   </div>
   <div id="ghMsg" class="muted" style="margin-top:8px" role="status"></div>
   <small class="hint">Sucht nach einer neueren ESP8266-Firmware. Das Online-Update wird zunächst vorgemerkt und beim Neustart ausgeführt. Bei Problemen kann eine passende BIN-Datei manuell installiert werden. Gerät während eines Updates nicht ausschalten.</small>
  </div>
  <div class="card">
   <h2>Manuelles Firmware-Update (OTA)</h2>
   <input id="fw" type="file" accept=".bin,application/octet-stream">
   <div style="margin-top:12px"><button class="btn" type="button" onclick="upload()" id="upBtn">Firmware installieren</button></div>
   <div class="bar"><div id="upBar"></div></div>
   <div id="upMsg" class="muted" style="margin-top:8px" role="status"></div>
   <small class="hint">Wähle eine passende ESP8266-Firmware-Datei mit der Endung <code>.bin</code>. Nach erfolgreichem Upload startet das Gerät neu.</small>
  </div>
  <div class="card">
   <h2>Einstellungen sichern</h2>
   <a class="btn sec" href="/api/export">Einstellungen sichern</a>
   <label for="cfgFile">Sicherung wiederherstellen (.json)</label><input id="cfgFile" type="file" accept=".json,application/json">
   <div style="margin-top:10px"><button class="btn" type="button" onclick="importCfg()">Sicherung importieren und neu starten</button></div>
   <small class="hint">Die Sicherung kann WLAN-Passwörter enthalten. Bewahre sie entsprechend geschützt auf. Beim Import wird das Gerät neu gestartet.</small>
  </div>
  <div class="card">
   <h2>Wartung</h2>
   <button class="btn sec" type="button" onclick="reboot()">Neu starten</button>
   <button class="btn danger" style="margin-left:8px" type="button" onclick="factory()">Werkseinstellungen</button>
   <small class="hint">Der Werksreset löscht gespeicherte WLAN-Zugangsdaten und Einstellungen und startet das Gerät anschließend neu.</small>
  </div>
 </section>
</main>

<div class="savebar">
 <div class="savebar-inner">
  <span id="saveStatus" class="save-status" role="status" aria-live="polite">Änderungen werden erst beim Speichern übernommen.</span>
  <button class="btn" id="saveSettingsBtn" type="button" onclick="saveAll()" disabled>Änderungen speichern</button>
 </div>
</div>
<div id="toast" class="toast" role="status" aria-live="polite"></div>

<script>
var $ = function(id){ return document.getElementById(id); };
var config = {};
var latestUpdate = false;
var dirty = false;
var toastTimer = 0;
var statusTimer = 0;\nvar statusLoading = false;\nvar updateInProgress = false;

function api(path, options) {
  return fetch(path, options || {}).then(function(response) {
    return response.text().then(function(raw) {
      var data = raw;
      try { data = raw ? JSON.parse(raw) : {}; } catch (_) {}
      if (!response.ok) throw new Error(data && data.error ? data.error : (raw || ("HTTP " + response.status)));
      return data;
    });
  });
}
function escapeHtml(value) {
  return String(value == null ? "" : value).replace(/[&<>"']/g, function(ch) {
    return {"&":"&amp;","<":"&lt;",">":"&gt;","\"":"&quot;","'":"&#39;"}[ch];
  });
}
function toast(message) {
  var el = $("toast");
  el.textContent = message;
  el.classList.add("show");
  clearTimeout(toastTimer);
  toastTimer = setTimeout(function(){ el.classList.remove("show"); }, 3200);
}
function setDirty() {
  dirty = true;
  $("saveStatus").textContent = "Ungespeicherte Änderungen";
  $("saveStatus").classList.add("dirty");
  $("saveSettingsBtn").disabled = false;
}
function setClean(text) {
  dirty = false;
  $("saveStatus").textContent = text || "Alle Einstellungen sind gespeichert.";
  $("saveStatus").classList.remove("dirty");
  $("saveSettingsBtn").disabled = true;
}
function showTab(name) {
  var target = $(name);
  if (!target || !target.classList.contains("tab")) return;
  document.querySelectorAll("main .tab").forEach(function(tab){ tab.classList.toggle("active", tab.id === name); });
  document.querySelectorAll("nav [data-t]").forEach(function(button){ button.classList.toggle("active", button.dataset.t === name); });
  window.scrollTo(0, 0);
}
document.querySelectorAll("nav [data-t]").forEach(function(button){
  button.addEventListener("click", function(){ showTab(button.dataset.t); });
});
document.querySelectorAll("[data-go]").forEach(function(button){
  button.addEventListener("click", function(){ showTab(button.dataset.go); });
});
$("helpToggle").addEventListener("click", function(){
  var enabled = document.body.classList.toggle("show-help");
  $("helpToggle").setAttribute("aria-expanded", String(enabled));
  $("helpToggle").textContent = enabled ? "Tipps ausblenden" : "Tipps";
});
$("themeMode").addEventListener("click", function(){
  var light = !document.body.classList.contains("light");
  document.body.classList.toggle("light", light);
  $("themeMode").textContent = light ? "Dunkles Design" : "Helles Design";
  try { localStorage.setItem("smalltv-ui-light", light ? "1" : "0"); } catch (_) {}
});
try {
  var savedLight = localStorage.getItem("smalltv-ui-light") === "1";
  document.body.classList.toggle("light", savedLight);
  $("themeMode").textContent = savedLight ? "Dunkles Design" : "Helles Design";
} catch (_) {}

function addWifiRow(network) {
  network = network || {};
  var tr = document.createElement("tr");
  tr.className = "symrow";
  var ssidCell = document.createElement("td");
  ssidCell.style.width = "44%";
  var ssid = document.createElement("input");
  ssid.className = "ws";
  ssid.type = "text";
  ssid.maxLength = 32;
  ssid.autocomplete = "off";
  ssid.placeholder = "SSID";
  ssid.value = network.ssid || "";
  ssid.addEventListener("input", setDirty);
  ssidCell.appendChild(ssid);
  var passCell = document.createElement("td");
  var pass = document.createElement("input");
  pass.className = "wp";
  pass.type = "password";
  pass.maxLength = 64;
  pass.autocomplete = "new-password";
  pass.placeholder = network.passSet ? "(unverändert)" : "Passwort";
  pass.addEventListener("input", setDirty);
  passCell.appendChild(pass);
  var removeCell = document.createElement("td");
  removeCell.style.width = "34px";
  var remove = document.createElement("button");
  remove.className = "btn sec";
  remove.type = "button";
  remove.style.padding = "6px 10px";
  remove.textContent = "×";
  remove.setAttribute("aria-label", "WLAN entfernen");
  remove.addEventListener("click", function(){ tr.remove(); setDirty(); });
  removeCell.appendChild(remove);
  tr.appendChild(ssidCell);
  tr.appendChild(passCell);
  tr.appendChild(removeCell);
  $("wifiTable").appendChild(tr);
}
function renderWifi(networks) {
  $("wifiTable").textContent = "";
  (networks && networks.length ? networks : [{}]).forEach(addWifiRow);
}
function addWifi() {
  if ($("wifiTable").querySelectorAll("tr").length >= 4) { toast("Maximal vier WLAN-Netzwerke."); return; }
  addWifiRow({});
  setDirty();
}
function collectWifi() {
  var networks = [];
  $("wifiTable").querySelectorAll("tr").forEach(function(tr){
    var ssid = tr.querySelector(".ws").value;
    if (!ssid.trim()) return;
    var item = {ssid:ssid};
    var pass = tr.querySelector(".wp").value;
    if (pass) item.pass = pass;
    networks.push(item);
  });
  return networks;
}
function scanPick(ssid) {
  var rows = $("wifiTable").querySelectorAll("tr");
  var target = null;
  for (var i=0; i<rows.length; i++) {
    if (!rows[i].querySelector(".ws").value.trim()) { target = rows[i]; break; }
  }
  if (!target) {
    if (rows.length >= 4) { toast("Maximal vier WLAN-Netzwerke."); return; }
    addWifiRow({});
    target = $("wifiTable").lastElementChild;
  }
  target.querySelector(".ws").value = ssid;
  target.querySelector(".ws").dispatchEvent(new Event("input", {bubbles:true}));
  target.querySelector(".wp").focus();
}
function scan() {
  $("scanList").innerHTML = '<div class="muted">Suche läuft …</div>';
  api("/api/scan").then(function(networks){
    $("scanList").textContent = "";
    networks.sort(function(a,b){ return b.rssi-a.rssi; });
    networks.forEach(function(network){
      var button = document.createElement("button");
      button.className = "net";
      button.type = "button";
      button.textContent = (network.open ? "" : "🔒 ") + network.ssid + " · " + network.rssi + " dBm";
      button.addEventListener("click", function(){ scanPick(network.ssid); });
      $("scanList").appendChild(button);
    });
    if (!networks.length) $("scanList").textContent = "Keine Netzwerke gefunden.";
  }).catch(function(error){ $("scanList").textContent = "Netzwerksuche fehlgeschlagen: " + error.message; });
}

function loadConfig() {
  api("/api/config").then(function(data){
    config = data || {};
    $("hostname").value = config.hostname || "smalltv";
    $("apSsid").value = config.apSsid || "SmallTV-Setup";
    renderWifi(config.wifi || []);
    setClean("Alle Einstellungen sind gespeichert.");
  }).catch(function(error){ toast("Einstellungen konnten nicht geladen werden: " + error.message); });
}
function saveAll() {
  if (!dirty) return;
  var payload = {
    wifi: collectWifi(),
    hostname: $("hostname").value.trim(),
    apSsid: $("apSsid").value.trim()
  };
  $("saveSettingsBtn").disabled = true;
  $("saveStatus").textContent = "Speichern …";
  api("/api/config", {
    method:"POST",
    headers:{"Content-Type":"application/json"},
    body:JSON.stringify(payload)
  }).then(function(result){
    config = Object.assign({}, config, payload);
    if (result.reboot) {
      setClean("Gespeichert – Gerät startet wegen Netzwerkänderung neu.");
      toast("Gespeichert. Das Gerät startet neu.");
      clearTimeout(statusTimer);
    } else {
      setClean("Alle Einstellungen sind gespeichert.");
      toast("Einstellungen gespeichert.");
    }
  }).catch(function(error){
    $("saveStatus").textContent = "Speichern fehlgeschlagen";
    $("saveStatus").classList.add("dirty");
    $("saveSettingsBtn").disabled = false;
    toast("Speichern fehlgeschlagen: " + error.message);
  });
}
$("hostname").addEventListener("input", setDirty);
$("apSsid").addEventListener("input", setDirty);

function formatUptime(seconds) {
  var total = Math.max(0, Number(seconds) || 0);
  var days = Math.floor(total / 86400);
  var hours = Math.floor((total % 86400) / 3600);
  var minutes = Math.floor((total % 3600) / 60);
  return (days ? days + "d " : "") + hours + "h " + minutes + "m";
}
function kv(key, value) {
  return '<div class="kv"><b>' + escapeHtml(key) + '</b><span>' + escapeHtml(value) + '</span></div>';
}
function showStatus(s) {
  var connected = !!s.connected;
  $("dot").className = "dot" + (connected ? " ok" : "");
  $("hi").textContent = s.mode === "ap" ? "Einrichtungsmodus" : (s.ip || "");
  var pill = $("statusPill");
  pill.textContent = s.mode === "ap" ? "Einrichtungsmodus" : (connected ? "Verbunden" : "Offline");
  pill.className = "status-pill" + (s.mode === "ap" ? " warn" : (connected ? " ok" : " bad"));
  $("statusNetwork").textContent = s.mode === "ap" ? "Einrichtungs-Hotspot" : (connected ? (s.ssid || "Verbunden") : "Nicht verbunden");
  var address = $("statusAddress");
  address.href = s.ip ? ("http://" + s.ip) : "#";
  address.textContent = s.ip || "—";
  $("statusUptime").textContent = formatUptime(s.uptime);\n  $("statusHeap").textContent = (Number(s.heap)||0).toLocaleString("de-DE") + " B";\n  $("statusHeapSub").textContent = "Größter Block: " + (Number(s.maxBlock)||0).toLocaleString("de-DE") + " B";
  $("fwVer").textContent = (s.fw || "SmallTV") + " " + (s.version || "");
  var rows = kv("Firmware", (s.fw || "SmallTV") + " " + (s.version || "")) +
    kv("Variante", s.variant || "ESP8266 Web-only") +
    kv("Modus", String(s.mode || "—").toUpperCase()) +
    kv("WLAN", s.ssid || "—") +
    kv("IP-Adresse", s.ip || "—") +
    kv("mDNS", "http://" + (config.hostname || "smalltv") + ".local") +
    kv("Signal", connected ? (s.rssi + " dBm") : "—") +
    (s.wifiFail ? kv("Letzter Verbindungsfehler", s.wifiFail) : "") +
    kv("Freier Heap", (s.heap || 0) + " Byte") +
    kv("Größter freier Block", (s.maxBlock || 0) + " Byte") +
    kv("Laufzeit", formatUptime(s.uptime)) +
    kv("Letzter Reset", s.reset || "—") +
    (s.updateMsg ? kv("Update-Status", s.updateMsg) : "");
  $("statusBox").innerHTML = rows;
  if (s.updateMsg && !$("ghMsg").textContent) $("ghMsg").textContent = s.updateMsg;
}
function loadStatus() {
  api("/api/status").then(showStatus).catch(function(error){
    $("statusPill").textContent = "Nicht erreichbar";
    $("statusPill").className = "status-pill warn";
    $("dot").className = "dot";
    $("statusBox").textContent = "Status nicht verfügbar: " + error.message;
  });
}
function checkUpdate() {
  $("ghMsg").textContent = "Suche nach Updates …";
  $("chkBtn").disabled = true;
  api("/api/checkupdate").then(function(update){
    $("chkBtn").disabled = false;
    if (!update.ok) {
      $("ghMsg").textContent = "Suche fehlgeschlagen: " + (update.error || "Unbekannter Fehler");
      return;
    }
    if (update.newer) {
      $("ghMsg").textContent = "Version " + update.latest + " ist verfügbar (installiert: " + update.current + ").";
      $("ghUpBtn").disabled = false;
    } else {
      $("ghMsg").textContent = "Firmware ist aktuell (" + update.current + ").";
      $("ghUpBtn").disabled = true;
    }
  }).catch(function(error){
    $("chkBtn").disabled = false;
    $("ghMsg").textContent = "Update-Prüfung fehlgeschlagen: " + error.message;
  });
}
function selfUpdate() {
  if (!latestUpdate && $("ghUpBtn").disabled) return;
  if (!confirm("Neueste Firmware herunterladen und installieren? Das Gerät startet bei Erfolg neu.")) return;
  $("ghUpBtn").disabled = true;
  $("chkBtn").disabled = true;
  $("ghMsg").textContent = "Online-Update wird gestartet …";
  api("/api/selfupdate", {method:"POST"}).then(function(){
    $("ghMsg").textContent = "Update vorgemerkt. Das Gerät startet nach dem Download neu.";
  }).catch(function(error){
    $("ghMsg").textContent = "Update konnte nicht gestartet werden: " + error.message;
    $("chkBtn").disabled = false;
  });
}
function upload() {
  var file = $("fw").files[0];
  if (!file) { toast("Bitte zuerst eine Firmware-BIN-Datei auswählen."); return; }
  if (!confirm("Firmware installieren und das Gerät danach neu starten?")) return;
  var data = new FormData();
  data.append("firmware", file, file.name);
  var request = new XMLHttpRequest();
  $("upBtn").disabled = true;
  $("upBar").style.width = "0%";
  $("upMsg").textContent = "Upload wird gestartet …";
  request.upload.addEventListener("progress", function(event){
    if (event.lengthComputable) {
      var percent = Math.round(event.loaded * 100 / event.total);
      $("upBar").style.width = percent + "%";
      $("upMsg").textContent = "Firmware wird übertragen: " + percent + "%";
    }
  });
  request.addEventListener("load", function(){
    $("upBtn").disabled = false;
    if (request.status >= 200 && request.status < 300) {
      $("upBar").style.width = "100%";
      $("upMsg").textContent = "Firmware übertragen. Gerät startet neu.";
    } else {
      $("upMsg").textContent = "Firmware-Update fehlgeschlagen: " + (request.responseText || ("HTTP " + request.status));
    }
  });
  request.addEventListener("error", function(){
    $("upBtn").disabled = false;
    $("upMsg").textContent = "Verbindung unterbrochen. Prüfe nach einigen Sekunden, ob das Gerät neu startet.";
  });
  request.open("POST", "/update");
  request.send(data);
}
function importCfg() {
  var file = $("cfgFile").files[0];
  if (!file) { toast("Bitte zuerst eine JSON-Sicherungsdatei auswählen."); return; }
  var reader = new FileReader();
  reader.onload = function(){
    try { JSON.parse(reader.result); }
    catch (_) { toast("Die Datei enthält kein gültiges JSON."); return; }
    if (!confirm("Diese Konfiguration importieren? Das Gerät startet anschließend neu.")) return;
    api("/api/import", {
      method:"POST",
      headers:{"Content-Type":"application/json"},
      body:reader.result
    }).then(function(){
      toast("Sicherung importiert. Gerät startet neu.");
    }).catch(function(error){ toast("Import fehlgeschlagen: " + error.message); });
  };
  reader.readAsText(file);
}
function reboot() {
  if (!confirm("SmallTV neu starten?")) return;
  api("/api/reboot", {method:"POST"}).then(function(){ toast("Neustart ausgelöst."); })
    .catch(function(error){ toast("Neustart fehlgeschlagen: " + error.message); });
}
function factory() {
  if (!confirm("Alle Einstellungen löschen und das Gerät auf Werkseinstellungen zurücksetzen?")) return;
  api("/api/factory", {method:"POST"}).then(function(){ toast("Werksreset ausgeführt. Gerät startet neu."); })
    .catch(function(error){ toast("Werksreset fehlgeschlagen: " + error.message); });
}

function initialise() {
  loadConfig();
  loadStatus();
  statusTimer = setInterval(function(){
    if (document.visibilityState !== "hidden") loadStatus();
  }, 10000);
}
document.addEventListener("visibilitychange", function(){
  if (document.visibilityState !== "hidden") loadStatus();
});
initialise();
</script>
</body>
</html>
)HTML";
