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
  ['settingsThemes', 'Themes'], ['photos', 'Fotos'], ['wifi', 'Netzwerk'],
  ['update', 'System'], ['settingsAdvanced', 'Erweitert']
]) {
  assert.match(html, new RegExp(`data-t="${id}"[^>]*>${title}`), `navigation entry: ${title}`);
  assert.match(html, new RegExp(`<section id="${id}" class="tab`), `page exists: ${id}`);
}

assert.match(html, /function showTab\(id\)[\s\S]*aria-current/);
assert.match(html, /id="saveStatus"[^>]*role="status"[^>]*aria-live="polite"/);
assert.match(html, /prefers-reduced-motion/);
assert.match(html, /:focus-visible/);
assert.match(html, /data-page="settingsClock"><h2>Uhr-Designs/);
assert.match(html, /function arrangeSettingsPages\(\)/);

for (const endpoint of [
  '/api/config', '/api/status', '/api/photos/upload', '/api/photos/delete-all',
  '/api/themes/install', '/api/themes/select', '/api/themes/metadata',
  '/api/themes/layers/order', '/api/reboot', '/api/factory'
]) assert.ok(html.includes(endpoint), `active UI still uses ${endpoint}`);

console.log('active Settings UI smoke checks passed');
