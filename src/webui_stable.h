#pragma once
#include <Arduino.h>
static const char WEBUI_STABLE_HTML[] PROGMEM = R"HTML(
<!doctype html>
<html lang="de">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<meta name="color-scheme" content="dark light">
<title>SmallTV Stable</title>
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
.theme-item{display:flex;gap:12px;align-items:flex-start;padding:12px 0;border-bottom:1px solid var(--bd)}.theme-item:last-child{border-bottom:0}.theme-item .meta{flex:1;min-width:0}.theme-item .meta strong{display:block;overflow-wrap:anywhere}.theme-item .meta small{display:block;color:var(--mut);margin-top:3px}.theme-item .meta .error{color:var(--red);overflow-wrap:anywhere}.theme-item .btn{padding:7px 10px;font-size:13px}
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
@media(prefers-reduced-motion:reduce){*,*::before,*::after{scroll-behavior:auto!important;transition-duration:.01ms!important;animation-duration:.01ms!important;animation-iteration-count:1!important}}

/* Keep feedback messages readable in the light theme. */
.toast{color:var(--fg)}
body.light .toast{background:var(--card);color:var(--fg);border-color:var(--bd)}
body.light .status-pill.warn{color:#80520a;border-color:rgba(128,82,10,.28);background:rgba(227,179,65,.15)}
body.light .save-status.dirty{color:#80520a}


/* Clean control UI: consistent green surfaces in both colour modes. */
:root{--field:#0b100c;--surface2:#1d281f;--nav:rgba(14,20,15,.97);--warn:#e3b341}
body.light{--field:#f8fbf8;--surface2:#f5f9f4;--nav:rgba(242,247,241,.97)}
nav{background:var(--nav)}
.playlist-item{align-items:center;gap:9px}.playlist-item[draggable="true"]{cursor:grab}.playlist-item.dragging{opacity:.45}.playlist-item .meta{min-width:0;overflow-wrap:anywhere}.playlist-item .duration{width:88px;flex:0 0 88px}.playlist-item .duration input{min-height:38px;padding:6px}.playlist-item .actions{display:flex;gap:5px;flex-wrap:wrap}.playlist-item .actions .btn{padding:7px 9px;font-size:12px}
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
/* UI refinement: preserve the original SmallTV green palette; tune only contrast and surfaces. */
:root{--bg:#101611;--card:#19221a;--surface2:#1d281f;--field:#0b100c;--fg:#edf4ed;--mut:#9eaa9f;--bd:#2f3d31;--acc:#72c77c;--acc2:#a0d9a5;--red:#ed7373;--warn:#e3b341;--nav:rgba(14,20,15,.97);--shadow:0 12px 30px rgba(0,0,0,.18);--focus:rgba(114,199,124,.25)}
body.light{--bg:#f2f7f1;--card:#fff;--surface2:#f5f9f4;--field:#f8fbf8;--fg:#1b2b20;--mut:#59695d;--bd:#d6e3d7;--acc:#338b4f;--acc2:#347b50;--red:#bd3f43;--warn:#80560f;--nav:rgba(242,247,241,.97);--shadow:0 8px 24px rgba(28,66,36,.07);--focus:rgba(51,139,79,.18)}
nav{background:var(--nav);border-bottom-color:var(--bd)}
nav button{display:inline-flex;align-items:center;gap:8px;padding:10px 14px;min-height:44px}
nav button.active{background:var(--surface2);color:var(--fg);box-shadow:inset 0 0 0 1px var(--bd)}
nav button:hover{background:var(--surface2)}
.nav-icon,.utility-btn svg{width:17px;height:17px;flex:0 0 auto;fill:none;stroke:currentColor;stroke-width:1.8;stroke-linecap:round;stroke-linejoin:round}
.utility-btn{display:inline-flex;align-items:center;justify-content:center;gap:7px;min-height:40px;padding:8px 11px;background:var(--surface2);border:1px solid var(--bd);color:var(--fg);font-weight:600}
.utility-btn:hover{border-color:var(--acc);color:var(--acc2)}
header .dot{box-shadow:0 0 0 3px rgba(114,199,124,.12)}header .dot.ok{background:var(--acc)}
.stat-card{background:var(--surface2);border-color:var(--bd)}.stat-card .label{letter-spacing:.06em}
button.btn{background:var(--acc);color:#04130a}
button.btn.sec{background:var(--surface2);color:var(--fg);border:1px solid var(--bd)}
button.btn.sec:hover:not(:disabled){border-color:var(--acc);color:var(--acc2)}
button.btn.danger{background:var(--red);color:#2a0b10}body.light button.btn.danger{background:var(--red);color:#fff}
body.light button.btn:not(.sec):not(.danger){color:#fff}
.page-intro .eyebrow,.welcome .eyebrow{color:var(--acc2)}.chip{background:var(--acc);color:#102315}.bar>div{background:var(--acc)}
.status-pill.ok{color:var(--acc2);border-color:rgba(114,199,124,.38);background:rgba(114,199,124,.09)}
body.light .status-pill.ok{color:#276b3b;border-color:rgba(51,139,79,.28);background:rgba(51,139,79,.08)}
.status-pill.warn{color:var(--warn);border-color:rgba(227,179,65,.38);background:rgba(227,179,65,.08)}
body.light .status-pill.warn{color:#80560f;border-color:rgba(128,86,15,.25);background:rgba(227,179,65,.17)}
.status-pill.bad{color:var(--red);border-color:var(--red);background:rgba(237,115,115,.08)}
header .spacer{min-width:8px}
dialog#debugDialog{width:min(760px,calc(100vw - 28px));max-width:760px;max-height:88vh;overflow:auto;padding:22px}
dialog#debugDialog::backdrop{background:rgba(4,10,5,.64);backdrop-filter:blur(3px)}
.debug-head{display:flex;align-items:flex-start;justify-content:space-between;gap:16px}
.debug-head .eyebrow{margin:0 0 5px;color:var(--acc2);font-size:11px;font-weight:700;letter-spacing:.12em}
.debug-head h2{margin:0;color:var(--fg);font-size:24px;letter-spacing:-.03em;text-transform:none}
.debug-head p:last-child{margin:7px 0 0;color:var(--mut);font-size:13px}
.debug-toolbar{display:flex;align-items:center;justify-content:space-between;gap:12px;margin:18px 0}
.debug-toolbar>div{display:flex;gap:8px;flex-wrap:wrap}
.debug-grid{display:grid;grid-template-columns:repeat(4,minmax(0,1fr));gap:10px;margin:0 0 14px}
.debug-tile{min-width:0;padding:13px;border:1px solid var(--bd);border-radius:12px;background:var(--surface2)}
.debug-tile span{display:block;color:var(--mut);font-size:11px;font-weight:650;text-transform:uppercase;letter-spacing:.06em}
.debug-tile strong{display:block;margin-top:6px;font-size:15px;overflow-wrap:anywhere}
.debug-tile small{display:block;margin-top:3px;color:var(--mut);font-size:11px}
.debug-tile.ok strong{color:var(--acc2)}.debug-tile.warn strong{color:var(--warn)}.debug-tile.bad strong{color:var(--red)}
.debug-columns{display:grid;grid-template-columns:repeat(2,minmax(0,1fr));gap:10px}
.debug-panel{min-width:0;padding:14px;border:1px solid var(--bd);border-radius:12px;background:var(--card)}
.debug-panel h3{margin:0 0 10px;font-size:13px;color:var(--fg)}
.debug-kv .kv{gap:12px;font-size:12px;align-items:flex-start}
.debug-kv .kv b{color:var(--mut);font-weight:500}
.debug-kv .kv span{text-align:right;overflow-wrap:anywhere;max-width:62%}
.debug-raw{margin-top:14px;border-top:1px solid var(--bd);padding-top:12px}
.debug-raw summary{cursor:pointer;color:var(--mut);font-size:13px}
.debug-raw pre{max-height:240px;overflow:auto;padding:12px;border:1px solid var(--bd);border-radius:9px;background:var(--field);font:12px/1.5 ui-monospace,SFMono-Regular,Consolas,monospace;white-space:pre-wrap;overflow-wrap:anywhere}
.debug-privacy{margin:14px 0 0;color:var(--mut);font-size:11px;line-height:1.5}
@media(max-width:760px){header{padding:12px 14px;gap:8px;flex-wrap:wrap}header h1{font-size:16px}header .spacer{flex:1}.utility-btn{min-height:38px;padding:7px 9px}nav{padding:8px 12px}nav button{padding:9px 11px}main{padding:22px 14px 36px}.debug-grid{grid-template-columns:repeat(2,minmax(0,1fr))}}
@media(max-width:520px){.utility-btn span{display:none}.utility-btn{width:40px;height:40px;padding:9px}.utility-btn svg{width:19px;height:19px}.debug-head{align-items:flex-start}.debug-head h2{font-size:21px}.debug-toolbar{align-items:flex-start;flex-direction:column}.debug-columns{grid-template-columns:1fr}dialog#debugDialog{padding:16px}.quick-grid{grid-template-columns:repeat(2,minmax(0,1fr))}}

/* Additional semantic accents: retain the original SmallTV green palette. */
:root{
 --accent-info:#8bbce8;
 --accent-cyan:#7ccbd2;
 --accent-violet:#baa2e8;
 --accent-coral:#e9a18a;
 --accent-amber:#e4bf69;
}
body.light{
 --accent-info:#286fa8;
 --accent-cyan:#247b85;
 --accent-violet:#7152a4;
 --accent-coral:#a34f36;
 --accent-amber:#815d12;
}
/* Small, consistent colour cues help distinguish navigation areas. */
nav button[data-t="wifi"] .nav-icon{color:var(--accent-info)}
nav button[data-t="display"] .nav-icon{color:var(--accent-cyan)}
nav button[data-t="themes"] .nav-icon{color:var(--accent-violet)}
nav button[data-t="photos"] .nav-icon{color:var(--accent-coral)}
nav button[data-t="playback"] .nav-icon{color:var(--accent-amber)}
nav button[data-t="update"] .nav-icon{color:var(--accent-info)}
#themeMode svg{color:var(--accent-amber)}
#debugToggle svg{color:var(--accent-violet)}
#helpToggle svg{color:var(--accent-info)}
/* The green stays the primary action and brand colour; cards get a thin category marker. */
.stat-card{position:relative;overflow:hidden}
.stat-card::before{content:"";position:absolute;inset:0 0 auto;height:3px;background:var(--stat-accent,var(--acc));opacity:.9}
.stat-card .label{color:var(--stat-accent,var(--mut))}
.quick-grid .stat-card:nth-child(1){--stat-accent:var(--acc2)}
.quick-grid .stat-card:nth-child(2){--stat-accent:var(--accent-info)}
.quick-grid .stat-card:nth-child(3){--stat-accent:var(--accent-violet)}
.quick-grid .stat-card:nth-child(4){--stat-accent:var(--accent-amber)}
.quick-grid .stat-card:nth-child(5){--stat-accent:var(--accent-cyan)}
/* Match section eyebrow and diagnostic-panel accents to their navigation icon. */
#wifi .page-intro .eyebrow{color:var(--accent-info)}
#display .page-intro .eyebrow{color:var(--accent-cyan)}
#themes .page-intro .eyebrow{color:var(--accent-violet)}
#photos .page-intro .eyebrow{color:var(--accent-coral)}
#playback .page-intro .eyebrow{color:var(--accent-amber)}
#update .page-intro .eyebrow{color:var(--accent-info)}
.debug-panel{border-top:2px solid var(--panel-accent,var(--bd))}
.debug-panel:nth-child(1){--panel-accent:var(--acc)}
.debug-panel:nth-child(2){--panel-accent:var(--accent-violet)}
.debug-panel:nth-child(3){--panel-accent:var(--accent-cyan)}
.debug-panel:nth-child(4){--panel-accent:var(--accent-amber)}
.debug-panel h3{color:var(--panel-accent,var(--fg))}


/* Playback controls: compact overview, clear run state and touch-friendly queue editing. */
.playback-hero{
 display:grid;gap:18px;padding:20px;margin:0 0 16px;border:1px solid var(--bd);
 border-radius:16px;background:linear-gradient(145deg,var(--card),var(--surface2));box-shadow:var(--shadow)
}
.playback-hero[data-state="playing"]{border-color:var(--acc);box-shadow:0 0 0 1px rgba(114,199,124,.12),var(--shadow)}
.playback-hero[data-state="dirty"]{border-color:var(--warn)}
.playback-hero[data-state="error"]{border-color:var(--red)}
.playback-hero-top{display:flex;align-items:center;justify-content:space-between;gap:12px;flex-wrap:wrap}
.playback-save-state{color:var(--mut);font-size:12px}
.playback-now p{margin:0 0 5px;color:var(--acc2);font-size:10px;font-weight:750;letter-spacing:.13em}
.playback-now h3{margin:0;color:var(--fg);font-size:clamp(20px,3vw,26px);line-height:1.25;overflow-wrap:anywhere}
.playback-now>span{display:block;margin-top:5px;color:var(--mut);font-size:13px}
.playback-summary{display:flex;gap:0;border-top:1px solid var(--bd);border-bottom:1px solid var(--bd);padding:10px 0}
.playback-summary>div{display:flex;flex:1;flex-direction:column;gap:2px;padding:0 14px;border-right:1px solid var(--bd)}
.playback-summary>div:first-child{padding-left:0}
.playback-summary>div:last-child{border:0}
.playback-summary strong{font-size:17px;color:var(--fg)}
.playback-summary span{font-size:11px;color:var(--mut)}
.playback-actions{display:flex;align-items:center;gap:8px;flex-wrap:wrap}
.playback-actions .btn{min-height:44px}
.playback-action-divider{height:26px;width:1px;background:var(--bd);margin:0 3px}
.playback-settings-grid{display:grid;grid-template-columns:repeat(2,minmax(0,1fr));gap:18px}
.playback-setting{min-width:0}
.playback-setting>small{display:block;margin-top:7px;color:var(--mut);font-size:12px;line-height:1.5}
.playback-autostart{padding:12px 14px;border:1px solid var(--bd);border-radius:12px;background:var(--surface2)}
.playback-autostart .chk{margin:0;min-height:42px;align-items:center}
.playback-card-heading{display:flex;align-items:flex-start;justify-content:space-between;gap:12px;margin-bottom:14px}
.playback-card-heading h2{margin:0 0 3px}
.playback-card-heading p{margin:0;color:var(--mut);font-size:12px}
.playback-limit{display:inline-flex;align-items:center;justify-content:center;min-height:28px;padding:3px 10px;border:1px solid var(--bd);border-radius:999px;color:var(--mut);font-size:11px;font-weight:650;white-space:nowrap}
.playlist-add-grid{display:grid;grid-template-columns:repeat(2,minmax(0,1fr));gap:12px}
.playlist-add-panel{min-width:0;padding:14px;border:1px solid var(--bd);border-radius:13px;background:var(--surface2)}
.playlist-add-type{display:inline-block;margin-bottom:3px;font-size:10px;font-weight:800;letter-spacing:.1em;color:var(--accent-cyan)}
.playlist-add-panel.theme .playlist-add-type{color:var(--accent-violet)}
.playlist-add-panel.photo .playlist-add-type{color:var(--accent-coral)}
.playlist-add-panel label{margin-top:5px}
.playlist-add-panel .btn{width:100%;margin-top:9px;min-height:42px}
.playlist-heading{align-items:center}
.playlist-heading-actions{display:flex;align-items:center;gap:7px}
.playlist-heading-actions .btn{min-height:36px;padding:7px 10px;font-size:12px}
.playlist-entries{display:grid;gap:8px}
.playlist-empty{padding:26px 16px;text-align:center;border:1px dashed var(--bd);border-radius:13px;background:var(--surface2)}
.playlist-empty strong{display:block;color:var(--fg);font-size:14px}
.playlist-empty p{margin:5px 0 0;color:var(--mut);font-size:12px}
.playlist-item{
 display:grid;grid-template-columns:38px minmax(0,1fr) minmax(100px,128px) auto;
 align-items:center;gap:12px;padding:12px;border:1px solid var(--bd);border-radius:13px;
 background:var(--surface2);transition:border-color .15s,background .15s,opacity .15s
}
.playlist-item[draggable="true"]{cursor:grab}
.playlist-item[draggable="true"]:active{cursor:grabbing}
.playlist-item.dragging{opacity:.45}
.playlist-item.is-current{border-color:var(--acc);box-shadow:inset 3px 0 var(--acc)}
.playlist-order{display:flex;align-items:center;justify-content:center;width:34px;height:34px;border-radius:10px;background:var(--card);color:var(--mut);font-size:12px;font-weight:750;font-variant-numeric:tabular-nums}
.playlist-item.is-current .playlist-order{background:var(--acc);color:#102315}
.playlist-meta{min-width:0;display:flex;flex-direction:column;align-items:flex-start;gap:2px;overflow-wrap:anywhere}
.playlist-kind{display:inline-block;font-size:9px;font-weight:800;letter-spacing:.1em;color:var(--accent-coral)}
.playlist-kind.theme{color:var(--accent-violet)}
.playlist-meta strong{max-width:100%;color:var(--fg);font-size:13px;overflow-wrap:anywhere}
.playlist-meta small{color:var(--mut);font-size:11px}
.playlist-duration{width:auto;min-width:0;flex:0 0 auto}
.playlist-duration label{margin:0 0 4px;font-size:10px;white-space:nowrap}
.playlist-duration-field{display:flex;align-items:center;gap:6px}
.playlist-duration-field input{width:76px;min-width:0;min-height:40px;padding:7px 8px;font-size:14px;font-variant-numeric:tabular-nums}
.playlist-duration-field span{color:var(--mut);font-size:11px}
.playlist-item-actions{display:flex;align-items:center;justify-content:flex-end;gap:5px;flex-wrap:wrap}
.playlist-item-actions .btn{min-width:42px;min-height:42px;padding:7px 9px;font-size:13px}
.playlist-item-actions .playlist-remove{min-width:82px;font-size:11px}
.playlist-footer{display:flex;align-items:center;justify-content:space-between;gap:12px;flex-wrap:wrap;margin-top:16px;padding-top:14px;border-top:1px solid var(--bd)}
.playlist-saved-label{color:var(--mut);font-size:12px}
.playlist-footer-actions{display:flex;align-items:center;gap:8px;flex-wrap:wrap}
.playlist-footer-actions .btn{min-height:42px}
.playback-hero button:disabled,.playlist-footer button:disabled,.playlist-item button:disabled{cursor:not-allowed}
@media(max-width:700px){
 .playback-settings-grid{grid-template-columns:1fr;gap:12px}
 .playback-hero{padding:16px;gap:15px}
 .playlist-item{grid-template-columns:34px minmax(0,1fr) auto;gap:9px}
 .playlist-duration{grid-column:2;grid-row:2}
 .playlist-item-actions{grid-column:3;grid-row:1 / span 2;flex-direction:column;align-items:stretch}
 .playlist-item-actions .btn{min-height:40px}
 .playlist-item-actions .playlist-remove{min-width:42px;font-size:0}
 .playlist-item-actions .playlist-remove::after{content:"×";font-size:19px}
 .playlist-duration-field input{width:82px}
}
@media(max-width:480px){
 .playback-summary>div{padding:0 9px}
 .playback-actions{display:grid;grid-template-columns:repeat(2,minmax(0,1fr))}
 .playback-actions .btn{width:100%}
 .playback-action-divider{display:none}
 .playlist-add-grid{grid-template-columns:1fr}
 .playlist-heading{align-items:flex-start}
 .playlist-heading-actions{flex-wrap:wrap;justify-content:flex-end}
 .playlist-footer{align-items:flex-start}
 .playlist-footer-actions{width:100%}
 .playlist-footer-actions .btn{flex:1}
}

</style>
</head>
<body>
<header>
 <svg class="logo" viewBox="0 0 64 64" aria-hidden="true"><g fill="none" stroke="var(--acc)" stroke-width="3" stroke-linecap="round" stroke-linejoin="round"><rect x="7" y="13" width="50" height="38" rx="6"/><polyline points="15,38 23,30 30,34 37,22 44,28 50,20"/><path d="M24 51v6M40 51v6M20 57h8M36 57h8"/></g></svg>
 <span id="dot" class="dot"></span><h1>SmallTV Stable</h1>
 <span id="chip" class="chip" style="display:none"></span><span id="hi" class="muted"></span><span class="spacer"></span>
 <button id="themeMode" class="help-toggle utility-btn" type="button" aria-label="Farbschema wechseln"><svg viewBox="0 0 24 24" aria-hidden="true"><circle cx="12" cy="12" r="4"/><path d="M12 2v2M12 20v2M4.93 4.93l1.42 1.42M17.65 17.65l1.42 1.42M2 12h2M20 12h2M4.93 19.07l1.42-1.42M17.65 6.35l1.42-1.42"/></svg><span>Helles Design</span></button>
 <button id="debugToggle" class="help-toggle utility-btn" type="button" aria-label="Debug öffnen" aria-haspopup="dialog" aria-controls="debugDialog"><svg viewBox="0 0 24 24" aria-hidden="true"><path d="M9 3h6l1 3 3 1v6l-3 1-1 3H9l-1-3-3-1V7l3-1 1-3Z"/><circle cx="12" cy="10" r="2.5"/><path d="M8 17.5 6.5 21M16 17.5l1.5 3.5"/></svg><span>Debug</span></button>
 <button id="helpToggle" class="help-toggle utility-btn" type="button" aria-label="Tipps ein- oder ausblenden" aria-expanded="false"><svg viewBox="0 0 24 24" aria-hidden="true"><circle cx="12" cy="12" r="9"/><path d="M9.6 9a2.5 2.5 0 1 1 4.4 1.6c-1 1.1-2 1.2-2 3M12 17.5h.01"/></svg><span>Tipps</span></button>
</header>
<nav aria-label="Hauptnavigation">
 <button data-t="status" class="active" aria-current="page" type="button"><svg class="nav-icon" viewBox="0 0 24 24" aria-hidden="true"><rect x="3" y="3" width="7" height="7" rx="1.5"/><rect x="14" y="3" width="7" height="7" rx="1.5"/><rect x="3" y="14" width="7" height="7" rx="1.5"/><rect x="14" y="14" width="7" height="7" rx="1.5"/></svg>Übersicht</button>
 <button data-t="wifi" type="button"><svg class="nav-icon" viewBox="0 0 24 24" aria-hidden="true"><path d="M2 8.5a15 15 0 0 1 20 0M5 12a10.5 10.5 0 0 1 14 0M8.5 15.5a5.3 5.3 0 0 1 7 0"/><circle cx="12" cy="19" r="1" fill="currentColor" stroke="none"/></svg>WLAN</button>
 <button data-t="display" type="button"><svg class="nav-icon" viewBox="0 0 24 24" aria-hidden="true"><rect x="3" y="4" width="18" height="14" rx="2"/><path d="M8 22h8M12 18v4"/></svg>Display</button>
 <button data-t="themes" type="button"><svg class="nav-icon" viewBox="0 0 24 24" aria-hidden="true"><path d="m12 3 9 5-9 5-9-5 9-5Z"/><path d="m3 12 9 5 9-5M3 16l9 5 9-5"/></svg>Themes</button>
 <button data-t="photos" type="button"><svg class="nav-icon" viewBox="0 0 24 24" aria-hidden="true"><rect x="3" y="4" width="18" height="16" rx="2"/><circle cx="8.5" cy="9" r="1.5"/><path d="m21 15-5-5L5 20"/></svg>Fotos</button>
 <button data-t="playback" type="button"><svg class="nav-icon" viewBox="0 0 24 24" aria-hidden="true"><path d="M8 5v14l11-7-11-7Z" fill="currentColor" stroke="none"/><path d="M3 4v16"/></svg>Playlist</button>
 <button data-t="update" type="button"><svg class="nav-icon" viewBox="0 0 24 24" aria-hidden="true"><circle cx="12" cy="12" r="3"/><path d="M19.4 15a1.7 1.7 0 0 0 .3 1.9l.1.1-1.7 2.9-.2-.1a1.7 1.7 0 0 0-1.9-.3 1.7 1.7 0 0 0-1 1.5v.2h-3.4V21a1.7 1.7 0 0 0-1-1.5 1.7 1.7 0 0 0-1.9.3l-.2.1-1.7-2.9.1-.1a1.7 1.7 0 0 0 .3-1.9 1.7 1.7 0 0 0-1.5-1H5v-3.4h.3a1.7 1.7 0 0 0 1.5-1 1.7 1.7 0 0 0-.3-1.9l-.1-.2 1.7-2.9.2.1a1.7 1.7 0 0 0 1.9.3 1.7 1.7 0 0 0 1-1.5V3h3.4v.3a1.7 1.7 0 0 0 1 1.5 1.7 1.7 0 0 0 1.9-.3l.2-.1 1.7 2.9-.1.2a1.7 1.7 0 0 0-.3 1.9 1.7 1.7 0 0 0 1.5 1h.3v3.4h-.3a1.7 1.7 0 0 0-1.5 1Z"/></svg>System</button>
</nav>
<main>
 <section id="status" class="tab active">
  <div class="welcome">
   <div><p class="eyebrow">SMALLTV · GERÄTEÜBERSICHT</p><h2>Dein Gerät</h2><p>Verbindung, Speicher und Display auf einen Blick.</p></div>
   <span id="statusPill" class="status-pill">Status wird geprüft</span>
  </div>
  <div class="quick-grid">
   <div class="stat-card"><span class="label">WLAN</span><strong id="statusNetwork">Wird geladen …</strong></div>
   <div class="stat-card"><span class="label">Geräteadresse</span><a id="statusAddress" href="#">Adresse öffnen</a></div>
   <div class="stat-card"><span class="label">Laufzeit</span><strong id="statusUptime">—</strong></div><div class="stat-card"><span class="label">Freier RAM</span><strong id="statusHeap">—</strong><span id="statusHeapSub" class="sub">Größter Block: —</span></div><div class="stat-card"><span class="label">Anzeige</span><strong id="statusDisplay">Wird geladen …</strong><span class="sub">Aktueller Betriebsmodus</span></div>
  </div>
  <div class="home-actions">
   <button class="btn sec" type="button" data-go="wifi">WLAN einrichten</button>
   <button class="btn sec" type="button" data-go="themes">Themes verwalten</button>
   <button class="btn sec" type="button" data-go="playback">Playlist bearbeiten</button>
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
   <div class="req"><b>Offener Zugriff:</b> Der Hotspot und die Weboberfläche sind nicht passwortgeschützt. Verwende sie nur in einer vertrauenswürdigen Umgebung.</div>
   <h2>Einrichtungs-Hotspot</h2>
   <label for="apSsid">Hotspot-Name</label><input id="apSsid" type="text" maxlength="32">
   <small class="hint">Der Hotspot ist unverschlüsselt und die Weboberfläche ohne Anmeldung erreichbar. Er wird zur Einrichtung bereitgestellt, wenn keine WLAN-Verbindung verfügbar ist. Standardadresse: <b>192.168.4.1</b>.</small>
  </div>
 </section>


 <section id="display" class="tab">
  <div class="page-intro"><p class="eyebrow">Bildschirm</p><h2>Display</h2><p>Manuelle Display-Einstellungen. Die Werte werden dauerhaft gespeichert und ohne Neustart übernommen.</p></div>
  <div class="card">
   <h2>Helligkeit und Ausrichtung</h2>
   <label for="displayBrightness">Helligkeit <span id="displayBrightnessValue">90 %</span></label>
   <input id="displayBrightness" type="range" min="0" max="100" step="1" value="90">
   <label for="displayRotation">Display-Rotation</label>
   <select id="displayRotation">
    <option value="0">0° – normal</option>
    <option value="1">90° – im Uhrzeigersinn</option>
    <option value="2">180°</option>
    <option value="3">270° – im Uhrzeigersinn</option>
   </select>
  </div>
  <div class="card">
   <h2>Farbdarstellung</h2>
   <label for="displayColorOrder">Farbfolge</label>
   <select id="displayColorOrder">
    <option value="rgb">RGB</option>
    <option value="bgr">BGR</option>
   </select>
   <div class="chk"><input id="displayInvert" type="checkbox"><label for="displayInvert">Display-Farben invertieren</label></div>
   <small class="hint">RGB/BGR wählt die Farbfolge des Panels. Invertierung schaltet den entsprechenden ST7789-Panelmodus um.</small>
  </div>
  <div class="card">
   <h2>Uhrzeit und Zeitzone</h2>
   <label for="timezone">Zeitzone</label>
   <select id="timezone">
    <option value="CET-1CEST,M3.5.0,M10.5.0/3">Europe/Berlin – Mitteleuropa (CET/CEST)</option>
    <option value="GMT0BST,M3.5.0/1,M10.5.0">Europe/London – Großbritannien (GMT/BST)</option>
    <option value="UTC0">UTC – koordinierte Weltzeit</option>
    <option value="EST5EDT,M3.2.0/2,M11.1.0/2">America/New_York – US Eastern</option>
    <option value="PST8PDT,M3.2.0/2,M11.1.0/2">America/Los_Angeles – US Pacific</option>
    <option value="JST-9">Asia/Tokyo – Japan</option>
    <option value="AEST-10AEDT,M10.1.0,M4.1.0/3">Australia/Sydney – Eastern Australia</option>
   </select>
   <small class="hint">Die automatische Sommerzeit wird über die Zeitzonenregel berechnet. NTP wird benötigt, wenn ein Theme Zeitwerte verwendet oder der Nachtmodus aktiv ist. Dafür muss das Gerät Internetzugang haben.</small>
  </div>
  <div class="card">
   <h2>Nachtmodus</h2>
   <div class="chk"><input id="nightEnabled" type="checkbox"><label for="nightEnabled">Zeitgesteuertes Dimmen aktivieren</label></div>
   <div class="row">
    <div><label for="nightStart">Beginn</label><input id="nightStart" type="time" value="22:00"></div>
    <div><label for="nightEnd">Ende</label><input id="nightEnd" type="time" value="07:00"></div>
   </div>
   <label for="nightBrightness">Helligkeit nachts <span id="nightBrightnessValue">5 %</span></label>
   <input id="nightBrightness" type="range" min="0" max="100" step="1" value="5">
   <small class="hint">Der Nachtmodus verwendet die lokale Uhrzeit und die oben gewählte Zeitzone. Er wird erst nach erfolgreicher NTP-Synchronisation aktiv. 0 % schaltet die Hintergrundbeleuchtung vollständig aus. Der normale Helligkeitswert gilt außerhalb des Zeitfensters.</small>
  </div>
  <div class="card">
   <h2>Theme-Sprache</h2>
   <label for="themeLanguage">Sprache der Theme-Inhalte</label>
   <select id="themeLanguage">
    <option value="de">Deutsch</option>
    <option value="en">English</option>
   </select>
   <small class="hint">Gilt für lokalisierte Text-Layer und Monats-/Wochentagsnamen. Fehlt eine deutsche Übersetzung, verwendet das Theme den englischen Text.</small>
  </div>
  <div class="card">
   <h2>Farbkorrektur</h2>
   <label for="displayRedGain">Rot <span id="displayRedGainValue">100 %</span></label>
   <input id="displayRedGain" type="range" min="50" max="150" step="1" value="100">
   <label for="displayGreenGain">Grün <span id="displayGreenGainValue">100 %</span></label>
   <input id="displayGreenGain" type="range" min="50" max="150" step="1" value="100">
   <label for="displayBlueGain">Blau <span id="displayBlueGainValue">100 %</span></label>
   <input id="displayBlueGain" type="range" min="50" max="150" step="1" value="100">
   <small class="hint">100 % entspricht neutraler Darstellung. Die Korrektur wird zeilenweise beim Pixel-Output angewendet; es wird kein zusätzlicher Bildpuffer angelegt.</small>
  </div>
 </section>

 <section id="themes" class="tab">
  <div class="page-intro"><p class="eyebrow">INHALTE</p><h2>Themes verwalten</h2><p>Theme-Pakete installieren, prüfen, aktivieren und entfernen.</p></div>
  <div class="card">
   <h2>Theme installieren</h2>
   <input id="themeFile" type="file" accept=".stheme,application/octet-stream">
   <div style="margin-top:12px"><button class="btn" type="button" onclick="uploadTheme()" id="themeUploadBtn">Theme hochladen und prüfen</button></div>
   <div class="bar"><div id="themeUploadBar"></div></div>
   <div id="themeUploadMsg" class="muted" style="margin-top:8px" role="status"></div>
   <small class="hint">Genau ein .stheme-Paket pro Upload. Container, Manifest, Pfade und Assets werden geprüft. Maximal 16 Pakete. Vor dem Upload das laufende Theme deaktivieren, damit genug RAM für die Prüfung bleibt.</small>
  </div>
  <div class="card">
   <div style="display:flex;align-items:center;justify-content:space-between;gap:12px"><h2>Installierte Pakete</h2><button class="btn sec" type="button" onclick="loadThemes()">Aktualisieren</button></div>
   <div id="themeStorage" class="muted" style="margin:8px 0 12px">Freier Dateispeicher: wird geladen …</div>
   <div id="themeRuntimeMsg" class="muted" style="margin:8px 0 12px" role="status"></div>
   <div id="themeList" aria-live="polite" class="muted">Paketliste wird geladen …</div>
  </div>
 </section>

 <section id="photos" class="tab">
  <div class="page-intro"><p class="eyebrow">FOTOSPEICHER</p><h2>Foto-Slideshow</h2><p>Bilder aus LittleFS auf dem 240×240-Display zeigen. Die ausgewählten Dateien werden im Browser verkleinert und als JPEG übertragen.</p></div>
  <div class="card">
   <h2>Fotos hinzufügen</h2>
   <input id="photoFiles" type="file" accept="image/*" multiple>
   <div style="margin-top:12px"><button class="btn" type="button" onclick="uploadPhotos()" id="photoUploadBtn">Fotos verkleinern und hochladen</button></div>
   <div class="bar"><div id="photoUploadBar"></div></div>
   <div id="photoUploadMsg" class="muted" style="margin-top:8px" role="status"></div>
   <small class="hint">Mehrere Bilder werden nacheinander verarbeitet. Der Browser passt sie ohne Verzerrung in 240×240 Pixel ein und speichert sie als JPEG. Maximal 16 Fotos, je 256 KiB nach der Umwandlung. Vorher ein aktives Theme deaktivieren.</small>
  </div>
  <div class="card">
   <h2>Slideshow</h2>
   <label for="photoInterval">Anzeigedauer pro Bild (Sekunden)</label>
   <input id="photoInterval" type="number" min="5" max="3600" step="1" value="15">
   <div class="row" style="margin-top:12px;flex-wrap:wrap">
    <button class="btn" id="photoStartBtn" type="button" onclick="startPhotos()">Slideshow starten</button>
    <button class="btn sec" id="photoStopBtn" type="button" onclick="stopPhotos()" disabled>Stoppen</button>
    <button class="btn sec" type="button" onclick="movePhoto('previous')" id="photoPreviousBtn" disabled>Zurück</button>
    <button class="btn sec" type="button" onclick="movePhoto('next')" id="photoNextBtn" disabled>Weiter</button>
   </div>
   <div id="photoStatus" class="muted" style="margin-top:10px" role="status">Status wird geladen …</div>
   <small class="hint">Ein laufendes .stheme und die Foto-Slideshow verwenden dasselbe Display und können nicht gleichzeitig aktiv sein. Die Slideshow startet nicht automatisch nach einem Neustart.</small>
  </div>
  <div class="card">
   <div style="display:flex;align-items:center;justify-content:space-between;gap:12px"><h2>Gespeicherte Fotos</h2><button class="btn sec" type="button" onclick="loadPhotos()">Aktualisieren</button></div>
   <div id="photoStorage" class="muted" style="margin:8px 0 12px">Freier Dateispeicher: wird geladen …</div>
   <div id="photoList" aria-live="polite" class="muted">Fotoliste wird geladen …</div>
  </div>
 </section>

 <section id="playback" class="tab">
  <div class="page-intro"><p class="eyebrow">AUTOMATISCHE ANZEIGE</p><h2>Wiedergabe</h2><p>Fotos und Themes mischen, die Reihenfolge festlegen und die Anzeige direkt steuern.</p></div>

  <div class="playback-hero" id="playbackHero" data-state="stopped">
   <div class="playback-hero-top">
    <span id="playbackStateBadge" class="status-pill">Status wird geladen</span>
    <span id="playlistStatus" class="playback-save-state" role="status" aria-live="polite">Playlist wird geladen …</span>
   </div>
   <div class="playback-now">
    <p>JETZT AUF DEM DISPLAY</p>
    <h3 id="playbackNowTitle">Wird geladen …</h3>
    <span id="playbackNowMeta">Verbindungsstatus wird abgerufen.</span>
   </div>
   <div class="playback-summary">
    <div><strong id="playbackCount">—</strong><span>Elemente</span></div>
    <div><strong id="playbackModeValue">—</strong><span>Reihenfolge</span></div>
    <div><strong id="playbackAutoValue">—</strong><span>Autostart</span></div>
   </div>
   <div class="playback-actions">
    <button class="btn" id="playbackStartBtn" type="button" onclick="startPlayback()" disabled>Starten</button>
    <button class="btn danger" id="playbackStopBtn" type="button" onclick="playbackAction('stop')" disabled>Stoppen</button>
    <span class="playback-action-divider" aria-hidden="true"></span>
    <button class="btn sec" id="playbackPreviousBtn" type="button" onclick="playbackAction('previous')" aria-label="Vorheriges Element" disabled>Zurück</button>
    <button class="btn sec" id="playbackNextBtn" type="button" onclick="playbackAction('next')" aria-label="Nächstes Element" disabled>Weiter</button>
   </div>
  </div>

  <div class="card">
   <h2>Ablauf einstellen</h2>
   <div class="playback-settings-grid">
    <div class="playback-setting">
     <label for="playlistMode">Reihenfolge</label>
     <select id="playlistMode" onchange="renderPlaybackControls()" disabled>
      <option value="sequence">Eigene Reihenfolge</option>
      <option value="shuffle">Zufällig mischen</option>
     </select>
     <small>Bei Zufall wird jedes Element einmal pro Runde gezeigt. Der letzte Eintrag einer Runde wird nicht direkt wiederholt.</small>
    </div>
    <div class="playback-setting playback-autostart">
     <div class="chk"><input id="playlistAutoStart" type="checkbox" onchange="renderPlaybackControls()" disabled><label for="playlistAutoStart">Nach einem Neustart automatisch starten</label></div>
     <small>Die Wiedergabe beginnt nach dem Hochfahren, sofern die Playlist mindestens ein Element enthält.</small>
    </div>
   </div>
  </div>

  <div class="card">
   <div class="playback-card-heading"><div><h2>Inhalte hinzufügen</h2><p>Gespeicherte Fotos und installierte Themes auswählen.</p></div><span class="playback-limit">Max. 32</span></div>
   <div class="playlist-add-grid">
    <div class="playlist-add-panel photo">
     <span class="playlist-add-type">FOTO</span>
     <label for="playlistPhotoSelect">Gespeichertes Foto</label>
     <select id="playlistPhotoSelect" onchange="renderPlaybackControls()" disabled><option value="">Fotos werden geladen …</option></select>
     <button class="btn sec" id="playlistAddPhotoBtn" type="button" onclick="addPlaylistItem('photo')" disabled>+ Foto hinzufügen</button>
    </div>
    <div class="playlist-add-panel theme">
     <span class="playlist-add-type">THEME</span>
     <label for="playlistThemeSelect">Installiertes Theme</label>
     <select id="playlistThemeSelect" onchange="renderPlaybackControls()" disabled><option value="">Themes werden geladen …</option></select>
     <button class="btn sec" id="playlistAddThemeBtn" type="button" onclick="addPlaylistItem('theme')" disabled>+ Theme hinzufügen</button>
    </div>
   </div>
   <small class="hint">Ein Eintrag kann nur einmal vorkommen. Themes mit zurückgestellter Prüfung werden beim Start vollständig geprüft.</small>
  </div>

  <div class="card">
   <div class="playback-card-heading playlist-heading">
    <div><h2>Deine Playlist</h2><p id="playlistEditHint">Lege die Reihenfolge und Anzeigedauer deiner Inhalte fest.</p></div>
    <div class="playlist-heading-actions">
     <span id="playlistCountBadge" class="playback-limit">0 / 32</span>
     <button class="btn sec" id="playlistRefreshBtn" type="button" onclick="refreshPlayback()">Neu laden</button>
    </div>
   </div>
   <div id="playlistEntries" class="playlist-entries" aria-label="Playlist-Einträge"></div>
   <div class="playlist-footer">
    <span id="playlistSavedLabel" class="playlist-saved-label">Änderungen werden geprüft …</span>
    <div class="playlist-footer-actions">
     <button class="btn danger" id="playlistClearBtn" type="button" onclick="clearPlaylist()" disabled>Leeren</button>
     <button class="btn" id="playlistSaveBtn" type="button" onclick="savePlaylist()" disabled>Änderungen speichern</button>
    </div>
   </div>
   <small class="hint">Die Anzeigedauer lässt sich pro Eintrag zwischen 5 Sekunden und 60 Minuten einstellen. Zum Sortieren die Pfeile verwenden oder Einträge am Desktop per Drag-and-drop verschieben. Während der Wiedergabe ist die Bearbeitung gesperrt.</small>
  </div>
 </section>

<section id="update" class="tab">
  <div class="page-intro"><p class="eyebrow">GERÄTEVERWALTUNG</p><h2>System &amp; Wartung</h2><p>Firmware aktualisieren, Konfiguration sichern und das Gerät warten.</p></div>
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
   <div id="storageRecoveryHint" class="req" style="display:none">LittleFS ist nicht verfügbar. Ein ausdrücklich bestätigter Werksreset initialisiert das Dateisystem und setzt alle Einstellungen zurück.</div>
   <button class="btn sec" type="button" onclick="reboot()">Neu starten</button>
   <button class="btn danger" style="margin-left:8px" type="button" onclick="factory()">Werkseinstellungen</button>
   <small class="hint">Der Werksreset löscht gespeicherte WLAN-Zugangsdaten und Einstellungen und startet das Gerät anschließend neu.</small>
  </div>
 </section>
</main>

<dialog id="debugDialog" aria-labelledby="debugTitle">
 <div class="debug-head">
  <div><p class="eyebrow">SUPPORT · LOKALE DIAGNOSE</p><h2 id="debugTitle">Gerätediagnose</h2><p>Live-Status und technische Werte. Es werden keine Einstellungen geändert.</p></div>
  <button class="btn sec" type="button" id="debugClose" aria-label="Diagnose schließen">Schließen</button>
 </div>
 <div class="debug-toolbar">
  <span id="debugHealth" class="status-pill">Diagnose wird geladen …</span>
  <div><button class="btn sec" type="button" id="debugRefresh">Aktualisieren</button> <button class="btn" type="button" id="debugCopy">Bericht kopieren</button></div>
 </div>
 <div id="debugOverview" class="debug-grid"><div class="debug-tile"><span>Verbindung</span><strong>Wird geladen …</strong></div></div>
 <div class="debug-columns">
  <section class="debug-panel"><h3>Gerät und Speicher</h3><div id="debugDevice" class="debug-kv muted">Wird geladen …</div></section>
  <section class="debug-panel"><h3>Theme und Datenquellen</h3><div id="debugTheme" class="debug-kv muted">Wird geladen …</div></section>
  <section class="debug-panel"><h3>Fotos und Playlist</h3><div id="debugMedia" class="debug-kv muted">Wird geladen …</div></section>
  <section class="debug-panel"><h3>Fehler und Hinweise</h3><div id="debugIssues" class="debug-kv muted">Wird geladen …</div></section>
 </div>
 <details class="debug-raw"><summary>Bereinigten Diagnosebericht anzeigen</summary><pre id="debugRaw">Noch keine Daten.</pre></details>
 <p class="debug-privacy">WLAN-Passwörter und gespeicherte Zugangsdaten werden nicht abgefragt. Die IP-Adresse wird beim Kopieren ausgeblendet.</p>
</dialog>
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
var editRevision = 0;
var toastTimer = 0;
var statusTimer = 0;
var statusLoading = false;
var updateInProgress = false;

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
  editRevision++;
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
function lockSettingsControls(locked) {
  document.querySelectorAll("#wifi input,#wifi button,#display input,#display select").forEach(function(control) {
    control.disabled = locked;
  });
}
function showTab(name) {
  var target = $(name);
  if (!target || !target.classList.contains("tab")) return;
  document.querySelectorAll("main .tab").forEach(function(tab){ tab.classList.toggle("active", tab.id === name); });
  document.querySelectorAll("nav [data-t]").forEach(function(button){
    var active = button.dataset.t === name;
    button.classList.toggle("active", active);
    if (active) button.setAttribute("aria-current", "page"); else button.removeAttribute("aria-current");
  });
  window.scrollTo(0, 0);
  if (name === "themes") loadThemes();
  if (name === "photos") loadPhotos();
  if (name === "playback") loadPlayback();
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
  $("helpToggle").querySelector("span").textContent = enabled ? "Tipps ausblenden" : "Tipps";
});
$("debugToggle").addEventListener("click", openDebug);
$("debugClose").addEventListener("click", function(){ $("debugDialog").close(); });
$("debugRefresh").addEventListener("click", loadDebug);
$("debugCopy").addEventListener("click", copyDebugReport);
$("debugDialog").addEventListener("click", function(event){
  if(event.target === $("debugDialog")) $("debugDialog").close();
});
$("themeMode").addEventListener("click", function(){
  var light = !document.body.classList.contains("light");
  document.body.classList.toggle("light", light);
  $("themeMode").querySelector("span").textContent = light ? "Dunkles Design" : "Helles Design";
  try { localStorage.setItem("smalltv-ui-light", light ? "1" : "0"); } catch (_) {}
});
try {
  var savedLight = localStorage.getItem("smalltv-ui-light") === "1";
  document.body.classList.toggle("light", savedLight);
  $("themeMode").querySelector("span").textContent = savedLight ? "Dunkles Design" : "Helles Design";
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
  ssid.setAttribute("aria-label", "WLAN-Name (SSID)");
  ssid.value = network.ssid || "";
  ssid.addEventListener("input", setDirty);
  ssidCell.appendChild(ssid);
  var passCell = document.createElement("td");
  var pass = document.createElement("input");
  pass.className = "wp";
  pass.type = "password";
  pass.setAttribute("aria-label", "WLAN-Passwort");
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

function updateDisplayLabels() {
  $("displayBrightnessValue").textContent = $("displayBrightness").value + " %";
  $("displayRedGainValue").textContent = $("displayRedGain").value + " %";
  $("displayGreenGainValue").textContent = $("displayGreenGain").value + " %";
  $("displayBlueGainValue").textContent = $("displayBlueGain").value + " %";
  $("nightBrightnessValue").textContent = $("nightBrightness").value + " %";
}
function loadConfig() {
  api("/api/config").then(function(data){
    config = data || {};
    $("hostname").value = config.hostname || "smalltv";
    $("apSsid").value = config.apSsid || "SmallTV-Setup";
    renderWifi(config.wifi || []);
    $("displayBrightness").value = config.displayBrightness == null ? 90 : config.displayBrightness;
    $("displayRotation").value = String(config.displayRotation == null ? 0 : config.displayRotation);
    $("displayColorOrder").value = config.displayBgr ? "bgr" : "rgb";
    $("displayInvert").checked = !!config.displayInvert;
    $("displayRedGain").value = config.displayRedGain == null ? 100 : config.displayRedGain;
    $("displayGreenGain").value = config.displayGreenGain == null ? 100 : config.displayGreenGain;
    $("displayBlueGain").value = config.displayBlueGain == null ? 100 : config.displayBlueGain;
    $("timezone").value = config.timezone || "CET-1CEST,M3.5.0,M10.5.0/3";
    $("nightEnabled").checked = !!config.nightEnabled;
    $("nightStart").value = config.nightStart || "22:00";
    $("nightEnd").value = config.nightEnd || "07:00";
    $("nightBrightness").value = config.nightBrightness == null ? 5 : config.nightBrightness;
    $("themeLanguage").value = config.language || "en";
    updateDisplayLabels();
    setClean("Alle Einstellungen sind gespeichert.");
  }).catch(function(error){ toast("Einstellungen konnten nicht geladen werden: " + error.message); });
}
function saveAll() {
  if (!dirty) return;
  var submittedRevision = editRevision;
  var payload = {
    wifi: collectWifi(),
    hostname: $("hostname").value.trim(),
    apSsid: $("apSsid").value.trim(),
    displayBrightness: Number($("displayBrightness").value),
    displayRotation: Number($("displayRotation").value),
    displayBgr: $("displayColorOrder").value === "bgr",
    displayInvert: $("displayInvert").checked,
    displayRedGain: Number($("displayRedGain").value),
    displayGreenGain: Number($("displayGreenGain").value),
    displayBlueGain: Number($("displayBlueGain").value),
    timezone: $("timezone").value,
    nightEnabled: $("nightEnabled").checked,
    nightStart: $("nightStart").value || "22:00",
    nightEnd: $("nightEnd").value || "07:00",
    nightBrightness: Number($("nightBrightness").value),
    language: $("themeLanguage").value
  };
  $("saveSettingsBtn").disabled = true;
  lockSettingsControls(true);
  $("saveStatus").textContent = "Speichern …";
  api("/api/config", {
    method:"POST",
    headers:{"Content-Type":"application/json"},
    body:JSON.stringify(payload)
  }).then(function(result){
    lockSettingsControls(false);
    config = Object.assign({}, config, payload);
    if (editRevision !== submittedRevision) {
      dirty = true;
      $("saveStatus").textContent = result.reboot
        ? "Gespeichert – neuere Änderungen sind noch offen; Neustart läuft."
        : "Gespeichert – neuere Änderungen sind noch offen.";
      $("saveStatus").classList.add("dirty");
      $("saveSettingsBtn").disabled = false;
      toast("Neuere Änderungen wurden nicht mitgespeichert.");
      return;
    }
    if (result.reboot) {
      setClean("Gespeichert – Gerät startet wegen Netzwerkänderung neu.");
      toast("Gespeichert. Das Gerät startet neu.");
      clearTimeout(statusTimer);
    } else {
      setClean("Alle Einstellungen sind gespeichert.");
      toast("Einstellungen gespeichert.");
    }
  }).catch(function(error){
    lockSettingsControls(false);
    $("saveStatus").textContent = "Speichern fehlgeschlagen";
    $("saveStatus").classList.add("dirty");
    $("saveSettingsBtn").disabled = false;
    toast("Speichern fehlgeschlagen: " + error.message);
  });
}
$("hostname").addEventListener("input", setDirty);
$("apSsid").addEventListener("input", setDirty);
["displayBrightness","displayRedGain","displayGreenGain","displayBlueGain"].forEach(function(id) {
  $(id).addEventListener("input", function() { updateDisplayLabels(); setDirty(); });
});
["displayRotation","displayColorOrder"].forEach(function(id) {
  $(id).addEventListener("change", setDirty);
});
$("displayInvert").addEventListener("change", setDirty);
$("timezone").addEventListener("change", setDirty);
["nightEnabled","nightStart","nightEnd"].forEach(function(id) { $(id).addEventListener("change", setDirty); });
$("nightBrightness").addEventListener("input", function(){ updateDisplayLabels(); setDirty(); });
$("themeLanguage").addEventListener("change", setDirty);

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
function debugTile(label, value, detail, tone) {
  return '<div class="debug-tile ' + (tone || '') + '"><span>' + escapeHtml(label) + '</span><strong>' + escapeHtml(value) + '</strong>' + (detail ? '<small>' + escapeHtml(detail) + '</small>' : '') + '</div>';
}
function debugBytes(value) {
  return (Number(value)||0).toLocaleString("de-DE") + " Byte";
}
function loadDebug() {
  $("debugHealth").textContent = "Diagnose wird aktualisiert …";
  $("debugOverview").innerHTML = '<div class="debug-tile"><span>Status</span><strong>Wird geladen …</strong></div>';
  $("debugDevice").textContent = $("debugTheme").textContent = $("debugMedia").textContent = $("debugIssues").textContent = "Wird geladen …";
  function read(path) { return api(path).catch(function(error){ return {_requestError:error.message}; }); }
  Promise.all([read("/api/status"),read("/api/photos"),read("/api/playback")]).then(function(results) {
    var s=results[0], p=results[1], w=results[2], issues=[];
    results.forEach(function(item,index){ if(item._requestError) issues.push(["Status","Fotos","Playlist"][index] + ": " + item._requestError); });
    if(!s._requestError) {
      if(s.storageReady===false) issues.push("LittleFS ist nicht verfügbar.");
      if(!s.connected && s.mode!=="ap") issues.push("Keine WLAN-Verbindung.");
      if(s.themeError) issues.push("Theme: " + s.themeError);
      if(Number(s.dataFailures)>0) issues.push("Mindestens eine Theme-Datenquelle meldet Fehler.");
      if(Number(s.dataStale)>0) issues.push("Theme-Daten enthalten veraltete Werte.");
    }
    var online=!s._requestError && !!s.connected, memory=Number(s.heap)||0, freeStorage=Number(p.freeBytes)||0;
    $("debugHealth").textContent=issues.length ? issues.length + " Hinweis(e)" : "Keine Auffälligkeiten";
    $("debugHealth").className="status-pill " + (issues.length ? "warn" : "ok");
    $("debugOverview").innerHTML =
      debugTile("Verbindung",s._requestError?"Nicht lesbar":(s.mode==="ap"?"Einrichtungsmodus":(online?"Verbunden":"Offline")),s.ip||"Lokale Adresse nicht verfügbar",online?"ok":(s.mode==="ap"?"warn":"bad"))+
      debugTile("Freier RAM",s._requestError?"—":debugBytes(memory),"Größter Block: "+debugBytes(s.maxBlock),memory<6144?"warn":"ok")+
      debugTile("Dateispeicher",p._requestError?"Nicht lesbar":debugBytes(freeStorage),(Number(p.count)||0)+" Fotos gespeichert",s.storageReady===false?"bad":"ok")+
      debugTile("Anzeige",s._requestError?"—":(s.playback?"Playlist":(s.themeLoaded?"Theme aktiv":(s.photoSlideshow?"Slideshow":"Bereit"))),s.version?"Firmware "+s.version:"",s.themeError?"warn":"ok");
    $("debugDevice").innerHTML =
      kv("Firmware",(s.fw||"SmallTV")+" "+(s.version||""))+kv("Variante",s.variant||"ESP8266")+
      kv("WLAN-Modus",s.mode||"—")+kv("WLAN-Signal",s.connected?(s.rssi+" dBm"):"—")+kv("IP-Adresse",s.ip||"—")+
      kv("Freier Heap",debugBytes(s.heap))+kv("Größter Block",debugBytes(s.maxBlock))+kv("Theme-RAM",debugBytes(s.themeHeap))+
      kv("Dateispeicher frei",p._requestError?"Nicht lesbar":debugBytes(freeStorage))+kv("LittleFS",s.storageReady?"Bereit":"Nicht verfügbar")+
      kv("Betriebszeit",formatUptime(s.uptime))+kv("Letzter Neustart",s.reset||"—");
    $("debugTheme").innerHTML =
      kv("Theme",s.themeLoaded?(s.theme||"Aktiv"):"Kein Theme aktiv")+
      kv("Datenquellen",(Number(s.dataReady)||0)+"/"+(Number(s.dataSources)||0)+" aktuell")+
      kv("Veraltet",String(Number(s.dataStale)||0))+kv("Fehlversuche",String(Number(s.dataFailures)||0))+
      kv("Zeitsynchronisation",s.timeSynced?"Synchronisiert":"Nicht synchronisiert")+kv("Lokale Uhrzeit",s.localTime||"—")+
      (s.themeError?kv("Theme-Fehler",s.themeError):"")+(s.dataError?kv("Datenfehler",s.dataError):"");
    $("debugMedia").innerHTML =
      kv("Fotos gespeichert",p._requestError?"Nicht lesbar":String(Number(p.count)||0))+
      kv("Foto-Slideshow",p.active?"Aktiv":"Gestoppt")+
      kv("Playlist",w._requestError?"Nicht lesbar":(w.playing?"Aktiv":"Gestoppt"))+
      kv("Playlist-Einträge",w._requestError?"—":String(Number(w.count)||0))+
      kv("Playlist-Modus",w.mode==="shuffle"?"Zufällig":"Reihenfolge")+kv("Autostart",w.autoStart?"Aktiv":"Aus");
    $("debugIssues").innerHTML=issues.length
      ? issues.map(function(issue){return '<div class="kv"><b>Hinweis</b><span>'+escapeHtml(issue)+'</span></div>';}).join("")
      : '<div class="kv"><b>Status</b><span>Keine bekannten Fehler oder Warnungen.</span></div>';
    var report={
      generatedAt:new Date().toISOString(),
      firmware:{name:s.fw||"",version:s.version||"",variant:s.variant||""},
      connection:{mode:s.mode||"",connected:!!s.connected,ip:"[ausgeblendet]",rssiDbm:s.rssi==null?null:s.rssi},
      memory:{freeBytes:Number(s.heap)||0,maxBlockBytes:Number(s.maxBlock)||0,themeHeapBytes:Number(s.themeHeap)||0},
      storage:{ready:s.storageReady!==false,freeBytes:freeStorage,photoCount:Number(p.count)||0},
      runtime:{themeActive:!!s.themeLoaded,theme:s.theme||"",photoSlideshow:!!s.photoSlideshow,playback:!!s.playback,timeSynced:!!s.timeSynced,localTime:s.localTime||""},
      themeData:{sources:Number(s.dataSources)||0,ready:Number(s.dataReady)||0,stale:Number(s.dataStale)||0,failures:Number(s.dataFailures)||0},
      errors:{theme:s.themeError||"",data:s.dataError||"",requests:results.map(function(item){return item._requestError||"";}).filter(Boolean)},
      system:{uptimeSeconds:Number(s.uptime)||0,reset:s.reset||""},
      playlist:{playing:!!w.playing,count:Number(w.count)||0,mode:w.mode||"",autoStart:!!w.autoStart}
    };
    $("debugRaw").textContent=JSON.stringify(report,null,2);
  }).catch(function(error){
    $("debugHealth").textContent="Diagnose fehlgeschlagen";$("debugHealth").className="status-pill bad";$("debugIssues").textContent=error.message;
  });
}
function openDebug() {
  var dialog=$("debugDialog");if(!dialog.open) dialog.showModal();loadDebug();
}
function copyDebugReport() {
  var report=$("debugRaw").textContent;
  if(!report || report==="Noch keine Daten.") {toast("Bitte zuerst die Diagnose aktualisieren.");return;}
  if(navigator.clipboard && navigator.clipboard.writeText) {
    navigator.clipboard.writeText(report).then(function(){toast("Bereinigter Diagnosebericht kopiert.");})
      .catch(function(){window.prompt("Diagnosebericht markieren und kopieren:",report);});
  } else window.prompt("Diagnosebericht markieren und kopieren:",report);
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
  $("statusUptime").textContent = formatUptime(s.uptime);
  $("statusHeap").textContent = (Number(s.heap)||0).toLocaleString("de-DE") + " B";
  $("statusHeapSub").textContent = "Größter Block: " + (Number(s.maxBlock)||0).toLocaleString("de-DE") + " B";
  $("statusDisplay").textContent = s.playback ? "Playlist läuft" : (s.themeLoaded ? "Theme aktiv" : (s.photoSlideshow ? "Slideshow läuft" : "Bereit"));
  $("fwVer").textContent = (s.fw || "SmallTV") + " " + (s.version || "");
  $("storageRecoveryHint").style.display = s.storageReady === false ? "block" : "none";
  var rows = kv("Firmware", (s.fw || "SmallTV") + " " + (s.version || "")) +
    kv("LittleFS", s.storageReady ? "Bereit" : "Nicht verfügbar") +
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
    kv("NTP-Uhrzeit", s.timeSynced ? (s.localTime + " · " + (s.timezone || "")) : "Noch nicht synchronisiert / nicht benötigt") +
    kv("Theme-Sprache", s.language === "de" ? "Deutsch" : "English") +
    kv("Theme", s.themeLoaded ? ("Aktiv: " + s.theme) : "Kein aktives Theme") +
    kv("Theme-Datenquellen", (Number(s.dataReady)||0) + "/" + (Number(s.dataSources)||0) + " aktuell" + (Number(s.dataStale) ? " · " + s.dataStale + " veraltet" : "")) +
    (s.dataError ? kv("Letzter Datenfehler", s.dataError) : "") +
    (s.themeError ? kv("Theme-Fehler", s.themeError) : "") +
    kv("Heap für Theme-Runtime", (Number(s.themeHeap)||0) + " Byte") +
    (s.updateMsg ? kv("Update-Status", s.updateMsg) : "");
  $("statusBox").innerHTML = rows;
  if (s.updateMsg && !$("ghMsg").textContent) $("ghMsg").textContent = s.updateMsg;
}
function loadStatus() {
  if (statusLoading || document.visibilityState === "hidden") return;
  statusLoading = true;
  api("/api/status").then(showStatus).catch(function(error){
    $("statusPill").textContent = "Nicht erreichbar";
    $("statusPill").className = "status-pill warn";
    $("dot").className = "dot";
    $("statusBox").textContent = "Status nicht verfügbar: " + error.message;
  }).then(function(){ statusLoading = false; });
}
function checkUpdate() {
  $("ghMsg").textContent = "Suche nach Updates …";
  $("chkBtn").disabled = true;
  api("/api/checkupdate").then(function(update){
    $("chkBtn").disabled = false;
    latestUpdate = !!(update.ok && update.newer);
    if (!update.ok) {
      latestUpdate = false; $("ghUpBtn").disabled = true;
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
    latestUpdate = false; $("ghUpBtn").disabled = true;
    $("chkBtn").disabled = false;
    $("ghMsg").textContent = "Update-Prüfung fehlgeschlagen: " + error.message;
  });
}
function selfUpdate() {
  if (!latestUpdate || updateInProgress) return;
  if (!confirm("Neueste Firmware herunterladen und installieren? Das Gerät startet bei Erfolg neu.")) return;
  updateInProgress = true;
  $("ghUpBtn").disabled = true;
  $("chkBtn").disabled = true;
  $("ghMsg").textContent = "Update wird vorgemerkt. Das Gerät startet gleich neu. Bitte nicht ausschalten.";
  api("/api/selfupdate", {method:"POST"}).then(function(){
    $("ghMsg").textContent = "Update vorgemerkt. Das Gerät startet nach dem Download neu.";
  }).catch(function(error){
    updateInProgress = false;
    $("ghMsg").textContent = "Update konnte nicht vorgemerkt werden: " + error.message;
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

function loadThemes() {
  var list = $("themeList");
  if (!list) return;
  list.textContent = "Paketliste wird geladen …";
  api("/api/themes").then(function(data) {
    var themes = Array.isArray(data.themes) ? data.themes : [];
    $("themeStorage").textContent = "Freier Dateispeicher: " + (Number(data.freeBytes)||0).toLocaleString("de-DE") + " Byte";
    $("themeRuntimeMsg").textContent = data.selected
      ? "Aktives Theme: " + data.selected
      : (data.error ? "Kein Theme aktiv: " + data.error : "Kein Theme aktiv.");
    list.textContent = "";
    if (!themes.length) {
      list.textContent = "Noch keine .stheme-Pakete installiert.";
      return;
    }
    themes.forEach(function(theme) {
      var row = document.createElement("div");
      row.className = "theme-item";
      var meta = document.createElement("div");
      meta.className = "meta";
      var name = document.createElement("strong");
      name.textContent = theme.name || theme.id || "Unbenanntes Theme";
      var details = document.createElement("small");
      details.textContent = (theme.id || "") +
        (theme.version ? " · Version " + theme.version : "") +
        " · " + (Number(theme.bytes)||0).toLocaleString("de-DE") + " Byte";
      meta.appendChild(name);
      meta.appendChild(details);
      if (theme.author) {
        var author = document.createElement("small");
        author.textContent = "Autor: " + theme.author;
        meta.appendChild(author);
      }
      var validity = document.createElement("small");
      var active = data.selected === theme.id;
      var deferred = theme.error === "Deactivate active theme to validate this package";
      validity.textContent = deferred
        ? (active ? "Wird angezeigt · Paketprüfung zurückgestellt" : "Prüfung beim Aktivieren")
        : (theme.valid ? (active ? "Wird auf dem Display angezeigt" : "Paket gültig") : "Paket ungültig");
      if (!theme.valid && !deferred) validity.className = "error";
      meta.appendChild(validity);
      if (!theme.valid && theme.error) {
        var error = document.createElement("small");
        error.className = deferred ? "" : "error";
        error.textContent = deferred
          ? "Die Prüfung erfolgt, sobald das laufende Theme beendet wird."
          : theme.error;
        meta.appendChild(error);
      }
      var actions = document.createElement("div");
      var toggle = document.createElement("button");
      toggle.type = "button";
      toggle.className = "btn sec";
      toggle.textContent = active ? "Deaktivieren" : "Aktivieren";
      toggle.disabled = !theme.valid && !deferred;
      toggle.addEventListener("click", function(){ selectTheme(active ? "" : theme.id); });
      actions.appendChild(toggle);
      var remove = document.createElement("button");
      remove.type = "button";
      remove.className = "btn danger";
      remove.textContent = "Löschen";
      remove.disabled = active;
      remove.title = active ? "Theme zuerst deaktivieren" : "Theme löschen";
      remove.addEventListener("click", function(){ deleteTheme(theme.id); });
      actions.appendChild(remove);
      row.appendChild(meta);
      row.appendChild(actions);
      list.appendChild(row);
    });
  }).catch(function(error) {
    list.textContent = "Theme-Liste konnte nicht geladen werden: " + error.message;
  });
}
function selectTheme(id) {
  api("/api/themes/select", {
    method:"POST",
    headers:{"Content-Type":"application/json"},
    body:JSON.stringify({id:id})
  }).then(function(){
    toast(id ? "Theme aktiviert und gespeichert." : "Theme deaktiviert.");
    loadThemes();
    loadStatus();
  }).catch(function(error) {
    toast("Theme-Auswahl fehlgeschlagen: " + error.message);
    loadThemes();
  });
}
function uploadTheme() {
  var file = $("themeFile").files[0];
  if (!file) { toast("Bitte zuerst eine .stheme-Datei auswählen."); return; }
  if (!/\.stheme$/i.test(file.name)) { toast("Erwartet wird eine .stheme-Datei."); return; }
  if (!confirm("Dieses Theme-Paket hochladen und vollständig validieren?")) return;
  var data = new FormData();
  data.append("theme", file, file.name);
  var request = new XMLHttpRequest();
  $("themeUploadBtn").disabled = true;
  $("themeUploadBar").style.width = "0%";
  $("themeUploadMsg").textContent = "Upload wird gestartet …";
  request.upload.addEventListener("progress", function(event) {
    if (event.lengthComputable) {
      var percent = Math.round(event.loaded * 100 / event.total);
      $("themeUploadBar").style.width = percent + "%";
      $("themeUploadMsg").textContent = "Paket wird übertragen: " + percent + "%";
    }
  });
  request.addEventListener("load", function() {
    $("themeUploadBtn").disabled = false;
    var result = {};
    try { result = JSON.parse(request.responseText || "{}"); } catch (_) {}
    if (request.status >= 200 && request.status < 300) {
      $("themeUploadBar").style.width = "100%";
      $("themeUploadMsg").textContent = "Paket installiert und validiert: " + (result.id || "");
      $("themeFile").value = "";
      toast("Theme-Paket installiert.");
      loadThemes();
    } else {
      $("themeUploadMsg").textContent = "Installation fehlgeschlagen: " + (result.error || request.responseText || ("HTTP " + request.status));
    }
  });
  request.addEventListener("error", function() {
    $("themeUploadBtn").disabled = false;
    $("themeUploadMsg").textContent = "Verbindung unterbrochen. Prüfe die Paketliste, bevor du erneut hochlädst.";
  });
  request.open("POST", "/api/themes/install");
  request.send(data);
}
function deleteTheme(id) {
  if (!confirm("Theme-Paket '" + id + "' wirklich aus LittleFS löschen?")) return;
  api("/api/themes/delete", {
    method:"POST",
    headers:{"Content-Type":"application/json"},
    body:JSON.stringify({id:id})
  }).then(function() {
    toast("Theme-Paket gelöscht.");
    loadThemes();
  }).catch(function(error) {
    toast("Löschen fehlgeschlagen: " + error.message);
  });
}

function reboot() {
  if (!confirm("SmallTV neu starten?")) return;
  api("/api/reboot", {method:"POST"}).then(function(){ toast("Neustart ausgelöst."); })
    .catch(function(error){ toast("Neustart fehlgeschlagen: " + error.message); });
}
function factory() {
  if (!confirm("Alle gespeicherten Einstellungen, WLAN-Zugangsdaten, .stheme-Pakete und Fotos werden dauerhaft aus LittleFS gelöscht. Die Firmware bleibt installiert. Das Gerät startet anschließend neu. Fortfahren?")) return;
  api("/api/factory", {method:"POST"}).then(function(){ toast("Werksreset ausgeführt. Gerät startet neu."); })
    .catch(function(error){ toast("Werksreset fehlgeschlagen: " + error.message); });
}

function loadPhotos() {
  var list = $("photoList");
  if (!list) return;
  list.textContent = "Fotoliste wird geladen …";
  api("/api/photos").then(function(data) {
    var photos = Array.isArray(data.photos) ? data.photos : [];
    $("photoStorage").textContent = "Freier Dateispeicher: " + (Number(data.freeBytes)||0).toLocaleString("de-DE") + " Byte · " + (Number(data.count)||0) + "/16 Fotos";
    $("photoStatus").textContent = data.active
      ? "Slideshow aktiv · " + (data.current || "Bild wird geladen") + " · Wechsel alle " + data.interval + " Sekunden"
      : "Slideshow gestoppt. " + photos.length + " Foto(s) gespeichert.";
    $("photoStartBtn").disabled = !!data.active || !photos.length;
    $("photoStopBtn").disabled = !data.active;
    $("photoNextBtn").disabled = !data.active || !photos.length;
    $("photoPreviousBtn").disabled = !data.active || !photos.length;
    $("photoUploadBtn").disabled = !!data.active || photos.length >= 16;
    list.textContent = "";
    if (!photos.length) {
      list.textContent = "Noch keine Fotos gespeichert. Wähle oben Bilder aus.";
      return;
    }
    photos.forEach(function(photo) {
      var row = document.createElement("div");
      row.className = "theme-item";
      var meta = document.createElement("div");
      meta.className = "meta";
      var name = document.createElement("strong");
      name.textContent = photo.name || "Foto";
      var details = document.createElement("small");
      details.textContent = (Number(photo.bytes)||0).toLocaleString("de-DE") + " Byte";
      meta.appendChild(name);
      meta.appendChild(details);
      var remove = document.createElement("button");
      remove.type = "button";
      remove.className = "btn danger";
      remove.textContent = "Löschen";
      remove.disabled = !!data.active;
      remove.title = data.active ? "Slideshow zuerst stoppen" : "Foto löschen";
      remove.addEventListener("click", function(){ deletePhoto(photo.name); });
      row.appendChild(meta);
      row.appendChild(remove);
      list.appendChild(row);
    });
  }).catch(function(error) {
    list.textContent = "Fotoliste konnte nicht geladen werden: " + error.message;
  });
}
var playlistDraft = [];
var playbackDragIndex = -1;
var playbackSavedSignature = "";
var playbackLoading = false;
var playbackBusy = false;
var playbackState = {playing:false,current:-1,mode:"sequence",autoStart:false,entries:[]};

function playlistSignature() {
  var values = [
    $("playlistMode").value === "shuffle" ? "shuffle" : "sequence",
    $("playlistAutoStart").checked ? "1" : "0"
  ];
  playlistDraft.forEach(function(item) {
    values.push(item.type + "|" + item.id + "|" + Number(item.seconds));
  });
  return values.join("\n");
}
function playlistIsDirty() {
  return playbackSavedSignature !== "" && playlistSignature() !== playbackSavedSignature;
}
function playbackIsLocked() {
  return playbackState.playing || playbackBusy || playbackLoading || playbackSavedSignature === "";
}
function refreshPlayback() {
  if (playlistIsDirty() && !confirm("Nicht gespeicherte Änderungen verwerfen und die gespeicherte Playlist neu laden?")) return;
  loadPlayback(true);
}
function loadPlayback(force) {
  if (playbackLoading || playbackBusy) return Promise.resolve(false);
  var keepDraft = playlistIsDirty() && !force;
  playbackLoading = true;
  renderPlaybackControls();
  $("playlistStatus").textContent = "Playlist und Inhalte werden geladen …";
  return Promise.all([api("/api/playback"), api("/api/photos"), api("/api/themes")]).then(function(results) {
    var state = results[0];
    var photos = Array.isArray(results[1].photos) ? results[1].photos : [];
    var themes = Array.isArray(results[2].themes) ? results[2].themes.filter(function(theme) {
      // A running theme defers package validation. It remains selectable and is
      // validated after playback releases the current renderer.
      return theme.valid === true ||
        theme.error === "Deactivate active theme to validate this package";
    }) : [];
    var photoSelect = $("playlistPhotoSelect");
    var themeSelect = $("playlistThemeSelect");
    photoSelect.textContent = "";
    themeSelect.textContent = "";
    photoSelect.appendChild(new Option(photos.length ? "Foto auswählen …" : "Keine Fotos gespeichert", ""));
    themeSelect.appendChild(new Option(themes.length ? "Theme auswählen …" : "Keine Themes installiert", ""));
    photos.forEach(function(photo) {
      photoSelect.appendChild(new Option(photo.name, photo.name));
    });
    themes.forEach(function(theme) {
      var label = theme.name || theme.id;
      if (!theme.valid) label += " (Prüfung beim Start)";
      themeSelect.appendChild(new Option(label, theme.id));
    });

    playbackState = {
      playing:!!state.playing,
      current:Number(state.current),
      mode:state.mode === "shuffle" ? "shuffle" : "sequence",
      autoStart:!!state.autoStart,
      entries:(state.entries || []).map(function(item) {
        return {type:item.type,id:item.id,seconds:Number(item.seconds)|| (item.type === "photo" ? 15 : 60)};
      })
    };
    if (!keepDraft) {
      $("playlistMode").value = playbackState.mode;
      $("playlistAutoStart").checked = playbackState.autoStart;
      playlistDraft = playbackState.entries.map(function(item) {
        return {type:item.type,id:item.id,seconds:item.seconds};
      });
      playbackSavedSignature = playlistSignature();
    }
    renderPlaylist();
    return true;
  }).catch(function(error) {
    $("playlistStatus").textContent = "Playlist konnte nicht geladen werden: " + error.message;
    $("playlistStateBadge").textContent = "Verbindung fehlgeschlagen";
    $("playbackHero").setAttribute("data-state","error");
    toast("Wiedergabedaten konnten nicht geladen werden.");
    return false;
  }).then(function(ok) {
    playbackLoading = false;
    renderPlaybackControls();
    return ok;
  });
}

function renderPlaybackControls() {
  var ready = playbackSavedSignature !== "";
  var dirty = playlistIsDirty();
  var locked = playbackState.playing;
  var busy = playbackBusy || playbackLoading || !ready;
  var count = playlistDraft.length;
  var current = playbackState.current >= 0 ? playbackState.entries[playbackState.current] : null;
  var badge = $("playbackStateBadge");
  var hero = $("playbackHero");

  hero.setAttribute("data-state", playbackState.playing ? "playing" : (dirty ? "dirty" : "stopped"));
  badge.className = "status-pill" + (playbackState.playing ? " ok" : (dirty ? " warn" : ""));
  badge.textContent = playbackBusy ? "Aktion läuft …" : (playbackLoading || !ready ? "Wird geladen …" : (locked ? "Wiedergabe läuft" : (dirty ? "Änderungen offen" : "Wiedergabe gestoppt")));

  $("playbackNowTitle").textContent = playbackState.playing
    ? (current ? current.id : "Element wird geladen …")
    : (count ? "Bereit zum Start" : "Noch keine Inhalte");
  $("playbackNowMeta").textContent = playbackState.playing
    ? (current ? (current.type === "photo" ? "Foto" : "Theme") + " · Element " + (playbackState.current + 1) + " von " + playbackState.entries.length : "Das nächste Element wird vorbereitet.")
    : (count ? "Die Playlist ist gespeichert und kann gestartet werden." : "Füge mindestens ein Foto oder Theme hinzu.");
  $("playbackCount").textContent = String(count);
  $("playbackModeValue").textContent = $("playlistMode").value === "shuffle" ? "Zufällig" : "Reihenfolge";
  $("playbackAutoValue").textContent = $("playlistAutoStart").checked ? "Ein" : "Aus";

  $("playlistStatus").textContent = locked
    ? "Die Anzeige läuft. Zum Bearbeiten zuerst stoppen."
    : (dirty ? "Es gibt nicht gespeicherte Änderungen." : (count ? "Alle Änderungen sind gespeichert." : "Die Playlist ist leer."));
  $("playlistSavedLabel").textContent = playbackBusy ? "Wird gespeichert …" : (dirty ? "Nicht gespeicherte Änderungen" : (ready ? "Alles gespeichert" : "Lade gespeicherte Playlist …"));
  $("playlistCountBadge").textContent = count + " / 32";

  $("playbackStartBtn").textContent = dirty ? "Speichern & starten" : "Starten";
  $("playbackStartBtn").disabled = busy || locked || count === 0;
  $("playbackStopBtn").disabled = busy || !locked;
  $("playbackPreviousBtn").disabled = busy || !locked;
  $("playbackNextBtn").disabled = busy || !locked;
  $("playlistSaveBtn").disabled = busy || locked || !dirty;
  $("playlistSaveBtn").textContent = dirty ? "Änderungen speichern" : "Gespeichert";
  $("playlistClearBtn").disabled = busy || locked || count === 0;
  $("playlistRefreshBtn").disabled = busy;

  $("playlistMode").disabled = busy || locked;
  $("playlistAutoStart").disabled = busy || locked;
  $("playlistPhotoSelect").disabled = busy || locked || $("playlistPhotoSelect").options.length <= 1;
  $("playlistThemeSelect").disabled = busy || locked || $("playlistThemeSelect").options.length <= 1;
  $("playlistAddPhotoBtn").disabled = busy || locked || !$("playlistPhotoSelect").value || count >= 32;
  $("playlistAddThemeBtn").disabled = busy || locked || !$("playlistThemeSelect").value || count >= 32;

  $("playlistEditHint").textContent = locked
    ? "Die Playlist ist während der Wiedergabe gesperrt. Stoppe sie, um Inhalte zu ändern."
    : (dirty ? "Deine Änderungen sind noch nicht gespeichert. Du kannst sie speichern oder direkt mit „Speichern & starten“ übernehmen." : "Pfeile oder Drag-and-drop ändern die Reihenfolge. Die Dauer gilt nur für den jeweiligen Eintrag.");
}

function renderPlaylist() {
  var list = $("playlistEntries");
  list.textContent = "";
  if (!playlistDraft.length) {
    var empty = document.createElement("div");
    empty.className = "playlist-empty";
    var title = document.createElement("strong");
    title.textContent = "Deine Playlist ist noch leer";
    var note = document.createElement("p");
    note.textContent = "Wähle oben ein gespeichertes Foto oder ein installiertes Theme aus.";
    empty.appendChild(title);
    empty.appendChild(note);
    list.appendChild(empty);
    renderPlaybackControls();
    return;
  }

  var locked = playbackState.playing || playbackBusy || playbackLoading;
  var dirty = playlistIsDirty();
  playlistDraft.forEach(function(item, index) {
    var row = document.createElement("div");
    row.className = "theme-item playlist-item";
    var isCurrent = !dirty && playbackState.playing && playbackState.current === index;
    if (isCurrent) row.classList.add("is-current");
    row.draggable = !locked;
    row.setAttribute("aria-label",(item.type === "photo" ? "Foto " : "Theme ") + item.id + ", Element " + (index + 1));
    if (!locked) {
      row.addEventListener("dragstart", function(event) {
        playbackDragIndex = index;
        row.classList.add("dragging");
        if (event.dataTransfer) event.dataTransfer.setData("text/plain", String(index));
      });
      row.addEventListener("dragend", function() {
        row.classList.remove("dragging");
        playbackDragIndex = -1;
      });
      row.addEventListener("dragover", function(event) { event.preventDefault(); });
      row.addEventListener("drop", function(event) {
        event.preventDefault();
        var from = playbackDragIndex;
        if (event.dataTransfer) {
          var parsed = Number(event.dataTransfer.getData("text/plain"));
          if (Number.isInteger(parsed)) from = parsed;
        }
        movePlaylistItem(from,index);
      });
    }

    var order = document.createElement("span");
    order.className = "playlist-order";
    order.textContent = String(index + 1).padStart(2,"0");

    var meta = document.createElement("div");
    meta.className = "meta playlist-meta";
    var kind = document.createElement("span");
    kind.className = "playlist-kind " + (item.type === "photo" ? "photo" : "theme");
    kind.textContent = item.type === "photo" ? "FOTO" : "THEME";
    var title = document.createElement("strong");
    title.textContent = item.id;
    var subtitle = document.createElement("small");
    subtitle.textContent = isCurrent ? "Wird gerade angezeigt" : (index + 1) + ". Position";
    meta.appendChild(kind);
    meta.appendChild(title);
    meta.appendChild(subtitle);

    var duration = document.createElement("div");
    duration.className = "duration playlist-duration";
    var durationLabel = document.createElement("label");
    durationLabel.textContent = "Anzeigedauer";
    durationLabel.htmlFor = "playlistDuration" + index;
    var durationField = document.createElement("div");
    durationField.className = "playlist-duration-field";
    var durationInput = document.createElement("input");
    durationInput.id = "playlistDuration" + index;
    durationInput.type = "number";
    durationInput.min = "5";
    durationInput.max = "3600";
    durationInput.step = "1";
    durationInput.inputMode = "numeric";
    durationInput.value = String(item.seconds);
    durationInput.disabled = locked;
    durationInput.setAttribute("aria-label", "Anzeigedauer für " + item.id + " in Sekunden");
    durationInput.addEventListener("change", function() {
      var value = Number(durationInput.value);
      if (!Number.isInteger(value) || value < 5 || value > 3600) {
        toast("Die Dauer muss zwischen 5 und 3600 Sekunden liegen.");
        durationInput.value = String(item.seconds);
        return;
      }
      item.seconds = value;
      renderPlaybackControls();
    });
    var unit = document.createElement("span");
    unit.textContent = "Sek.";
    durationField.appendChild(durationInput);
    durationField.appendChild(unit);
    duration.appendChild(durationLabel);
    duration.appendChild(durationField);

    var actions = document.createElement("div");
    actions.className = "actions playlist-item-actions";
    [[ "↑",-1,"Nach oben" ],[ "↓",1,"Nach unten" ]].forEach(function(action) {
      var button = document.createElement("button");
      button.type = "button";
      button.className = "btn sec";
      button.textContent = action[0];
      button.title = action[2];
      button.setAttribute("aria-label",action[2] + ": " + item.id);
      button.disabled = locked || index + action[1] < 0 || index + action[1] >= playlistDraft.length;
      button.addEventListener("click", function() { movePlaylistItem(index,index + action[1]); });
      actions.appendChild(button);
    });
    var remove = document.createElement("button");
    remove.type = "button";
    remove.className = "btn danger playlist-remove";
    remove.textContent = "Entfernen";
    remove.setAttribute("aria-label","Aus Playlist entfernen: " + item.id);
    remove.disabled = locked;
    remove.addEventListener("click", function() { removePlaylistItem(index); });
    actions.appendChild(remove);

    row.appendChild(order);
    row.appendChild(meta);
    row.appendChild(duration);
    row.appendChild(actions);
    list.appendChild(row);
  });
  renderPlaybackControls();
}

function movePlaylistItem(from,to) {
  if (playbackIsLocked() || from < 0 || from >= playlistDraft.length || to < 0 || to >= playlistDraft.length || from === to) return;
  var moved = playlistDraft.splice(from,1)[0];
  playlistDraft.splice(to,0,moved);
  renderPlaylist();
}
function removePlaylistItem(index) {
  if (playbackIsLocked() || index < 0 || index >= playlistDraft.length) return;
  playlistDraft.splice(index,1);
  renderPlaylist();
}
function clearPlaylist() {
  if (playbackIsLocked() || !playlistDraft.length) return;
  if (!confirm("Alle " + playlistDraft.length + " Einträge aus der Playlist entfernen? Die Änderung wird erst nach dem Speichern übernommen.")) return;
  playlistDraft = [];
  renderPlaylist();
}
function addPlaylistItem(type) {
  if (playbackIsLocked()) return;
  var select = type === "photo" ? $("playlistPhotoSelect") : $("playlistThemeSelect");
  var id = select.value;
  if (!id) {
    toast(type === "photo" ? "Bitte zuerst ein Foto auswählen." : "Bitte zuerst ein Theme auswählen.");
    return;
  }
  if (playlistDraft.length >= 32) { toast("Die Playlist kann maximal 32 Elemente enthalten."); return; }
  if (playlistDraft.some(function(item) { return item.type === type && item.id === id; })) {
    toast("Dieses Element ist bereits in der Playlist.");
    return;
  }
  playlistDraft.push({type:type,id:id,seconds:type === "photo" ? 15 : 60});
  select.value = "";
  renderPlaylist();
}
function savePlaylist() {
  if (playbackState.playing) {
    toast("Stoppe zuerst die Wiedergabe, bevor du die Playlist speicherst.");
    return Promise.resolve(false);
  }
  if (playbackBusy || playbackLoading) return Promise.resolve(false);
  var mode = $("playlistMode").value === "shuffle" ? "shuffle" : "sequence";
  var body = mode + "\n" + ($("playlistAutoStart").checked ? "1" : "0") + "\n";
  for (var i = 0; i < playlistDraft.length; i++) {
    var item = playlistDraft[i];
    var seconds = Number(item.seconds);
    if (!Number.isInteger(seconds) || seconds < 5 || seconds > 3600) {
      toast("Jede Anzeigedauer muss zwischen 5 und 3600 Sekunden liegen.");
      return Promise.resolve(false);
    }
    body += (item.type === "photo" ? "P" : "T") + "|" + item.id + "|" + seconds + "\n";
  }
  playbackBusy = true;
  renderPlaybackControls();
  return api("/api/playback/save", {method:"POST",headers:{"Content-Type":"text/plain"},body:body})
    .then(function() {
      playbackSavedSignature = playlistSignature();
      toast("Playlist gespeichert.");
      return true;
    })
    .catch(function(error) {
      toast("Playlist konnte nicht gespeichert werden: " + error.message);
      return false;
    })
    .then(function(saved) {
      playbackBusy = false;
      renderPlaybackControls();
      return saved;
    });
}
function startPlayback() {
  if (playbackBusy || playbackLoading || playbackState.playing) return;
  if (!playlistDraft.length) {
    toast("Füge zuerst mindestens ein Foto oder Theme hinzu.");
    return;
  }
  var start = function() { playbackAction("start"); };
  if (playlistIsDirty()) {
    savePlaylist().then(function(saved) {
      if (saved) start();
    });
  } else {
    start();
  }
}
function playbackAction(action) {
  if (playbackBusy || playbackLoading) return Promise.resolve(false);
  if (action === "start") {
    startPlayback();
    return Promise.resolve(true);
  }
  if ((action === "previous" || action === "next") && !playbackState.playing) return Promise.resolve(false);
  if (action === "stop" && !playbackState.playing) return Promise.resolve(false);
  playbackBusy = true;
  renderPlaybackControls();
  return api("/api/playback/" + action, {method:"POST"})
    .then(function() {
      var messages = {
        stop:"Wiedergabe gestoppt.",
        previous:"Vorheriger Eintrag wird angezeigt.",
        next:"Nächster Eintrag wird angezeigt."
      };
      toast(messages[action] || "Wiedergabe aktualisiert.");
      playbackBusy = false;
      return loadPlayback(true);
    })
    .catch(function(error) {
      toast("Wiedergabe fehlgeschlagen: " + error.message);
      playbackBusy = false;
      return loadPlayback(true);
    });
}


function makeSmallJpeg(file) {
  return new Promise(function(resolve, reject) {
    var url = URL.createObjectURL(file);
    var image = new Image();
    image.onload = function() {
      var width = image.naturalWidth || image.width;
      var height = image.naturalHeight || image.height;
      if (!width || !height) {
        URL.revokeObjectURL(url);
        reject(new Error("Bildgröße konnte nicht gelesen werden."));
        return;
      }
      var canvas = document.createElement("canvas");
      canvas.width = 240;
      canvas.height = 240;
      var context = canvas.getContext("2d");
      if (!context) {
        URL.revokeObjectURL(url);
        reject(new Error("Bildbearbeitung wird vom Browser nicht unterstützt."));
        return;
      }
      context.fillStyle = "#000";
      context.fillRect(0, 0, 240, 240);
      context.imageSmoothingEnabled = true;
      context.imageSmoothingQuality = "high";
      var scale = Math.min(240 / width, 240 / height);
      var drawWidth = Math.max(1, Math.round(width * scale));
      var drawHeight = Math.max(1, Math.round(height * scale));
      context.drawImage(image, Math.round((240 - drawWidth) / 2), Math.round((240 - drawHeight) / 2), drawWidth, drawHeight);
      canvas.toBlob(function(blob) {
        URL.revokeObjectURL(url);
        if (!blob) reject(new Error("JPEG-Umwandlung fehlgeschlagen."));
        else resolve(blob);
      }, "image/jpeg", 0.82);
    };
    image.onerror = function() {
      URL.revokeObjectURL(url);
      reject(new Error("Bild kann vom Browser nicht geöffnet werden: " + file.name));
    };
    image.src = url;
  });
}
function sendPhotoBlob(blob, name, index, total) {
  return new Promise(function(resolve, reject) {
    var request = new XMLHttpRequest();
    var data = new FormData();
    data.append("photo", blob, name.replace(/\.[^.]*$/, "") + ".jpg");
    request.upload.addEventListener("progress", function(event) {
      var fraction = event.lengthComputable && event.total ? event.loaded / event.total : 0;
      $("photoUploadBar").style.width = Math.round(((index + fraction) * 100) / total) + "%";
      $("photoUploadMsg").textContent = "Foto " + (index + 1) + "/" + total + " wird übertragen …";
    });
    request.addEventListener("load", function() {
      var result = {};
      try { result = JSON.parse(request.responseText || "{}"); } catch (_) {}
      if (request.status >= 200 && request.status < 300) resolve(result);
      else reject(new Error(result.error || request.responseText || ("HTTP " + request.status)));
    });
    request.addEventListener("error", function(){ reject(new Error("Verbindung beim Foto-Upload unterbrochen.")); });
    request.open("POST", "/api/photos/upload");
    request.send(data);
  });
}
function uploadPhotos() {
  var files = Array.prototype.slice.call($("photoFiles").files || []);
  if (!files.length) { toast("Bitte zuerst Bilder auswählen."); return; }
  $("photoUploadBtn").disabled = true;
  $("photoUploadBar").style.width = "0%";
  $("photoUploadMsg").textContent = "Bilder werden für das Display vorbereitet …";
  var chain = Promise.resolve();
  files.forEach(function(file, index) {
    chain = chain.then(function() {
      return makeSmallJpeg(file).then(function(blob) {
        if (blob.size > 256 * 1024) throw new Error(file.name + " bleibt nach der Umwandlung größer als 256 KiB.");
        return sendPhotoBlob(blob, file.name, index, files.length);
      });
    });
  });
  chain.then(function() {
    $("photoUploadBar").style.width = "100%";
    $("photoUploadMsg").textContent = files.length + " Foto(s) wurden gespeichert.";
    $("photoFiles").value = "";
    toast("Foto-Upload abgeschlossen.");
  }).catch(function(error) {
    $("photoUploadMsg").textContent = "Foto-Upload fehlgeschlagen: " + error.message;
    toast("Foto-Upload fehlgeschlagen: " + error.message);
  }).then(function() {
    $("photoUploadBtn").disabled = false;
    loadPhotos();
  });
}
function startPhotos() {
  var interval = Number($("photoInterval").value || 15);
  if (!Number.isFinite(interval) || interval < 5 || interval > 3600) {
    toast("Anzeigedauer muss zwischen 5 und 3600 Sekunden liegen.");
    return;
  }
  api("/api/photos/start", {
    method:"POST",
    headers:{"Content-Type":"application/json"},
    body:JSON.stringify({interval:Math.round(interval)})
  }).then(function() {
    toast("Foto-Slideshow gestartet.");
    loadPhotos();
  }).catch(function(error) {
    toast("Slideshow konnte nicht gestartet werden: " + error.message);
    loadPhotos();
  });
}
function stopPhotos() {
  api("/api/photos/stop", {method:"POST"}).then(function() {
    toast("Foto-Slideshow gestoppt.");
    loadPhotos();
  }).catch(function(error){ toast("Stoppen fehlgeschlagen: " + error.message); });
}
function movePhoto(direction) {
  api("/api/photos/" + (direction === "previous" ? "previous" : "next"), {method:"POST"})
    .then(loadPhotos)
    .catch(function(error){ toast("Bildwechsel fehlgeschlagen: " + error.message); loadPhotos(); });
}
function deletePhoto(name) {
  if (!confirm("Foto '" + name + "' aus LittleFS löschen?")) return;
  api("/api/photos/delete", {
    method:"POST",
    headers:{"Content-Type":"application/json"},
    body:JSON.stringify({name:name})
  }).then(function() {
    toast("Foto gelöscht.");
    loadPhotos();
  }).catch(function(error){ toast("Foto konnte nicht gelöscht werden: " + error.message); });
}

function initialise() {
  loadConfig();
  loadStatus();
  statusTimer = setInterval(function(){
    if (document.visibilityState !== "hidden") loadStatus();
  }, 20000);
}
document.addEventListener("visibilitychange", function(){
  if (document.visibilityState !== "hidden") loadStatus();
});
initialise();
</script>
</body>
</html>
)HTML";
