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
assert.match(html, /function resourceDiagnostics\(s\)/);
for (const field of ['heapMin', 'maxblkMin', 'fsFree', 'configTempBytes', 'photoTempBytes', 'memoryPhases']) {
  assert.ok(html.includes(field), `resource diagnostics expose ${field}`);
}
assert.match(html, /<summary>Speicher- und Heap-Diagnose<\/summary>/);
assert.ok(html.includes('photoLegacyFiles'));
assert.ok(html.includes('legacyFiles'));

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

// Photo-only profile: classify GIF signatures and reject animation input
// before upload; the firmware has no GIF frame parser, endpoint or renderer.
assert.match(html, /function photoFileHeader\(file\)/);
assert.match(html, /function photoIsGif\(file\)/);
assert.match(html, /bytes\[0\]===71&&bytes\[1\]===73&&bytes\[2\]===70/);
assert.match(html, /GIF-Animationen werden von dieser Foto-only-Firmware nicht unterstützt/);
assert.ok(html.includes('accept="image/jpeg,image/png,image/webp'));
assert.match(html, /function photoToJpeg\(file\)/);
assert.match(html, /function uploadPhotos\(\)/);
assert.doesNotMatch(html, /gifToAnimation|data-animation-frames/);
assert.ok(!html.includes('/api/photos/frame'));
const portal = fs.readFileSync('src/WebPortal.cpp', 'utf8');
assert.doesNotMatch(portal, /handlePhotoFrame|validatePhotoAnimation|PhotoAnimationUploadPath|photoUploadAnimated|smalltv_photo::isAnimation/);
assert.ok(portal.includes('photoValidateJpeg(PhotoUploadPath)'));
assert.ok(portal.includes('LittleFS.rename(PhotoUploadPath, target)'));
for (const endpoint of [
  '/api/config', '/api/status', '/api/photos/upload', '/api/photos/delete-all',
  '/api/themes/install', '/api/themes/select', '/api/themes/metadata',
  '/api/themes/layers/order', '/api/reboot', '/api/factory'
]) assert.ok(html.includes(endpoint), `active UI still uses ${endpoint}`);

assert.doesNotMatch(html, /\/api\/checkupdate|\/api\/selfupdate|Online-Update/);
assert.match(html, /\/update/);
assert.doesNotMatch(portal, /OtaUpdate\.h|handleCheckUpdate|handleSelfUpdate|\/api\/checkupdate|\/api\/selfupdate/);
assert.match(portal, /server\.on\("\/update", HTTP_POST, handleUpdateDone, handleUpdateUpload\)/);

console.log('active Settings UI smoke checks passed');
