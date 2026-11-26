#!/usr/bin/env python3
"""Static UI navigation, diagnostics privacy, and embedded JavaScript checks."""
from pathlib import Path
import shutil
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[2]
source = (ROOT / "src/webui_stable.h").read_text(encoding="utf-8")

# Navigation labels must keep matching sections and the top utility actions.
for tab in ("status", "wifi", "display", "themes", "photos", "playback", "update"):
    assert f'data-t="{tab}"' in source, f"missing navigation item: {tab}"
    assert f'<section id="{tab}"' in source, f"missing tab section: {tab}"
assert source.count('class="nav-icon"') == 7
assert 'id="debugToggle"' in source and 'id="debugDialog"' in source
assert 'id="helpToggle"' in source and 'id="themeMode"' in source

# The Debug action uses only read-only status endpoints and never requests
# /api/config, which contains saved network credentials.
debug_start = source.index("function loadDebug()")
debug_end = source.index("function openDebug()", debug_start)
debug_code = source[debug_start:debug_end]
for endpoint in ("/api/status", "/api/photos", "/api/playback"):
    assert endpoint in debug_code
assert "/api/config" not in debug_code
assert 'ip:"[ausgeblendet]"' in debug_code
assert "function copyDebugReport()" in source
assert "window.prompt(" in source

# Keep the original SmallTV green surfaces and brand colours while adding
# semantic accents for navigation and diagnostic sections.
assert ":root{--bg:#101611;--card:#19221a;" in source
assert "body.light{--bg:#f2f7f1;--card:#fff;" in source
for token in ("--accent-info:", "--accent-cyan:", "--accent-violet:", "--accent-coral:", "--accent-amber:"):
    assert token in source, f"missing semantic accent token: {token}"
for selector in ('nav button[data-t="themes"] .nav-icon', '#debugToggle svg', '.debug-panel:nth-child(4)'):
    assert selector in source, f"missing accent application: {selector}"

# The page is one embedded JS program; ask Node to validate its syntax.
script = source.split("<script>", 1)[1].split("</script>", 1)[0]
node = shutil.which("node")
if not node:
    raise SystemExit("Node.js is required to syntax-check the embedded web UI")
with tempfile.NamedTemporaryFile("w", suffix=".js", encoding="utf-8", delete=False) as handle:
    handle.write(script)
    script_path = handle.name
try:
    subprocess.run([node, "--check", script_path], check=True)
finally:
    Path(script_path).unlink(missing_ok=True)

print("SmallTV web UI navigation and diagnostics checks passed.")
