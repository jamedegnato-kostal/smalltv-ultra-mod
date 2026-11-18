// Smoke checks for the Settings UI that the firmware actually serves.
const assert = require('node:assert/strict');
const fs = require('node:fs');
const vm = require('node:vm');

const html = fs.readFileSync('src/webui_settings.html', 'utf8');
const script = html.match(/<script>([\s\S]*?)<\/script>/);
assert.ok(script, 'active Settings UI script exists');
new vm.Script(script[1], { filename: 'src/webui_settings.html' });

for (const [id, title] of [
  ['status', 'Übersicht'], ['settingsDisplay', 'Anzeige'], ['settingsClock', 'Uhr'],
  ['settingsThemes', 'Themes'], ['showflow', 'Showflow'], ['photos', 'Fotobibliothek'], ['wifi', 'Netzwerk'],
  ['update', 'System'], ['settingsAdvanced', 'Erweitert']
]) {
  assert.match(html, new RegExp(`data-t="${id}"[^>]*>${title}`), `navigation entry: ${title}`);
  assert.match(html, new RegExp(`<section id="${id}" class="tab`), `page exists: ${id}`);
}

assert.match(html, /function showTab\(id\)[\s\S]*aria-current/);
assert.match(html, /id="saveStatus"[^>]*role="status"[^>]*aria-live="polite"/);
assert.match(html, /prefers-reduced-motion/);
assert.match(html, /:focus-visible/);

assert.ok(html.includes('body.light .toast{background:var(--card);color:var(--fg);border-color:var(--bd)}'));
assert.ok(html.includes('body.light .status-pill.warn{color:#80520a'));
assert.ok(html.includes('body.light .save-status.dirty{color:#80520a}'));

assert.match(html, /data-page="settingsClock"><h2>Uhr-Designs/);
assert.match(html, /function arrangeSettingsPages\(\)/);
assert.match(html, /id="showflowEnabled"/);
assert.match(html, /id="showflowRandom"/);
assert.match(html, /id="showflowPhotoAddMode"[^>]*>[\s\S]*?value="random" selected/);
assert.match(html, /id="showflowPhotoAddId"/);
assert.match(html, /Maximal 16 Schritte/);
assert.match(html, /function renderShowflowSteps\(\)/);
assert.match(html, /function addShowflowPhoto\(\)/);
assert.doesNotMatch(html, /themePlaylist|photoPlayback|photosPerTheme|photoDwellSeconds/);

// GIF classification is based on the file signature, not a potentially empty or
// misleading filename/MIME type. GIF-looking files with a bad signature fail closed.
assert.match(html, /function photoFileHeader\(file\)/);
assert.match(html, /bytes\[0\]===71&&bytes\[1\]===73&&bytes\[2\]===70/);
assert.match(html, /bytes\[4\]===55\|\|bytes\[4\]===57/);
assert.match(html, /function photoIsGif\(file\)/);
assert.match(html, /var isGif=await photoIsGif\(picked\[i\]\)/);
assert.ok(!html.includes("picked[i].type==='image/gif'?await gifToAnimation"), 'upload routing must use verified file bytes');

// GIF capture must snapshot at display cadence before asynchronous JPEG encoding.
// The gallery preview must fetch each stored frame instead of always showing frame zero.
assert.match(html, /for\(var sample=0;sample<375&&frames\.length<12;sample\+\+\)/);
assert.match(html, /photoDelay\(16\)/);
assert.match(html, /ctx\.getImageData\(0,0,240,240\)/);
assert.match(html, /Math\.imul\(hash\^pixels\[q\+2\],16777619\)/);
assert.match(html, /frames\.length<2\)throw new Error\('Keine unterschiedlichen GIF-Frames erkannt/);
assert.doesNotMatch(html, /frames\.length<2\)return \{blob:await photoBlob/);
assert.match(html, /data-animation-frames/);
assert.match(html, /\/api\/photos\/frame\?id=/);
const portal = fs.readFileSync('src/WebPortal.cpp', 'utf8');
assert.match(portal, /server\.on\("\/api\/photos\/frame", HTTP_GET, handlePhotoFrame\)/);
assert.match(portal, /void handlePhotoFrame\(\)/);

for (const endpoint of [
  '/api/config', '/api/status', '/api/photos/upload', '/api/photos/delete-all',
  '/api/themes/install', '/api/themes/select', '/api/themes/metadata',
  '/api/themes/layers/order', '/api/reboot', '/api/factory'
]) assert.ok(html.includes(endpoint), `active UI still uses ${endpoint}`);

console.log('active Settings UI smoke checks passed');
