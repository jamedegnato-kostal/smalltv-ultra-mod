#!/usr/bin/env python3
"""Windows test bench for a real SmallTV Ultra (ESP8266) device."""
from __future__ import annotations

import base64
from concurrent.futures import ThreadPoolExecutor, as_completed
import json
import os
from pathlib import Path
import queue
import re
import shutil
import subprocess
import sys
import threading
import time
import urllib.error
import urllib.parse
import urllib.request
import webbrowser
import tkinter as tk
from tkinter import filedialog, messagebox, ttk

try:
    import serial
    from serial.tools import list_ports
except ImportError:
    serial = None
    list_ports = None

try:
    from PIL import Image, ImageOps
except ImportError:
    Image = ImageOps = None

try:
    from zeroconf import ServiceBrowser, ServiceListener, Zeroconf
except ImportError:
    ServiceBrowser = ServiceListener = Zeroconf = None


APP = "SmallTV Hardware Tester"
USER_DIR = Path(os.environ.get("LOCALAPPDATA", Path.home())) / "SmallTVHardwareTester"
LOG_DIR = USER_DIR / "logs"
PREFS_PATH = USER_DIR / "device.json"


def saved_device_url():
    try: return json.loads(PREFS_PATH.read_text(encoding="utf-8")).get("url", "")
    except (OSError, ValueError, AttributeError): return ""


def clean_url(url: str) -> str:
    return url.strip().rstrip("/")


class DeviceApi:
    """Talk directly to the firmware's HTTP API, including optional Digest auth."""
    def __init__(self, base: str, username: str = "", password: str = ""):
        self.base = clean_url(base)
        if not self.base.startswith(("http://", "https://")):
            raise ValueError("Die Geräteadresse muss mit http:// oder https:// beginnen.")
        self.username, self.password = username, password
        self.host_header = ""

    def request(self, method: str, route: str, data: bytes | None = None,
                content_type: str | None = None, timeout: int = 30):
        url = self.base + route
        manager = urllib.request.HTTPPasswordMgrWithDefaultRealm()
        if self.username or self.password:
            manager.add_password(None, self.base, self.username, self.password)
        opener = urllib.request.build_opener(urllib.request.HTTPDigestAuthHandler(manager))
        headers = {"User-Agent": "SmallTV-Hardware-Tester/1.0", "Connection": "close"}
        if self.host_header: headers["Host"] = self.host_header
        if content_type: headers["Content-Type"] = content_type
        request = urllib.request.Request(url, data=data, headers=headers, method=method)
        try:
            with opener.open(request, timeout=timeout) as response:
                return response.status, response.headers, response.read()
        except urllib.error.HTTPError as exc:
            return exc.code, exc.headers, exc.read()

    def json(self, method: str, route: str, payload=None, timeout: int = 30):
        data = None if payload is None else json.dumps(payload, separators=(",", ":")).encode()
        status, headers, raw = self.request(method, route, data,
            "application/json" if data is not None else None, timeout=timeout)
        try: parsed = json.loads(raw.decode("utf-8")) if raw else {}
        except ValueError: parsed = {"text": raw.decode("utf-8", "replace")}
        if status < 200 or status >= 300:
            raise RuntimeError(parsed.get("error") or parsed.get("text") or f"HTTP {status}")
        return status, headers, parsed

    def multipart(self, route: str, field: str, filename: str, content: bytes,
                  content_type: str = "application/octet-stream"):
        boundary = "----SmallTVTester" + base64.urlsafe_b64encode(os.urandom(12)).decode().rstrip("=")
        body = (f"--{boundary}\r\nContent-Disposition: form-data; name=\"{field}\"; filename=\"{Path(filename).name}\"\r\n"
                f"Content-Type: {content_type}\r\n\r\n").encode() + content + f"\r\n--{boundary}--\r\n".encode()
        status, headers, raw = self.request("POST", route, body,
            f"multipart/form-data; boundary={boundary}", timeout=120)
        try: parsed = json.loads(raw.decode("utf-8")) if raw else {}
        except ValueError: parsed = {"text": raw.decode("utf-8", "replace")}
        if status < 200 or status >= 300:
            raise RuntimeError(parsed.get("error") or parsed.get("text") or f"HTTP {status}")
        return status, headers, parsed


class Tester(tk.Tk):
    def __init__(self):
        super().__init__()
        self.title(APP)
        self.geometry("960x760")
        self.minsize(820, 620)
        self.configure(bg="#edf3ed")
        self.events = queue.Queue()
        self.monitor_thread = None
        self.monitor_stop = threading.Event()
        self.log_path = None
        default_bin = Path(__file__).resolve().parents[2] / ".pio" / "build" / "smalltv_settings" / "firmware.bin"
        self.bin_path = tk.StringVar(value=str(default_bin) if default_bin.is_file() else "")
        self.theme_path = tk.StringVar()
        self.photo_path = tk.StringVar()
        self.url = tk.StringVar(value=saved_device_url())
        self.user = tk.StringVar(value="admin")
        self.password = tk.StringVar()
        self.port = tk.StringVar()
        self.baud = tk.StringVar(value="460800")
        self.erase_flash = tk.BooleanVar(value=False)
        self.theme_kind = tk.StringVar(value="general")
        self.activate_theme = tk.BooleanVar(value=True)
        self.photo_count = tk.StringVar(value="3")
        self.photo_seconds = tk.StringVar(value="10")
        self._ui()
        self.refresh_ports()
        self.after(100, self._drain_events)
        self.protocol("WM_DELETE_WINDOW", self.close)

    def _ui(self):
        style = ttk.Style(self)
        try: style.theme_use("vista")
        except tk.TclError: pass
        style.configure("TFrame", background="#edf3ed")
        style.configure("Card.TLabelframe", background="#fff", padding=12)
        style.configure("Card.TLabelframe.Label", background="#fff", foreground="#245c34", font=("Segoe UI", 10, "bold"))
        style.configure("TLabel", background="#edf3ed", foreground="#203127", font=("Segoe UI", 9))
        style.configure("Hint.TLabel", foreground="#5c7062")
        style.configure("Accent.TButton", padding=7)
        root = ttk.Frame(self, padding=14); root.pack(fill="both", expand=True)
        ttk.Label(root, text="SmallTV Ultra · Hardware-Testbench", font=("Segoe UI", 18, "bold")).pack(anchor="w")
        ttk.Label(root, text="Firmware flashen, echte API-Endpunkte prüfen und Fotos / .stheme-Dateien am Gerät testen.", style="Hint.TLabel").pack(anchor="w", pady=(2, 10))

        device = ttk.LabelFrame(root, text="Gerät verbinden", style="Card.TLabelframe"); device.pack(fill="x", pady=(0, 9))
        row = ttk.Frame(device); row.pack(fill="x")
        ttk.Label(row, text="Adresse").pack(side="left")
        ttk.Entry(row, textvariable=self.url, width=29).pack(side="left", padx=(7, 8))
        ttk.Button(row, text="Gerät suchen", command=lambda: self.run_bg(self.discover_device)).pack(side="left", padx=(0, 8))
        ttk.Label(row, text="Digest-Benutzer").pack(side="left")
        ttk.Entry(row, textvariable=self.user, width=10).pack(side="left", padx=5)
        ttk.Label(row, text="Passwort").pack(side="left")
        ttk.Entry(row, textvariable=self.password, show="•", width=14).pack(side="left", padx=6)
        ttk.Button(row, text="API prüfen", command=lambda: self.run_bg(self.smoke_test)).pack(side="left", padx=(5, 0))
        ttk.Button(row, text="WebUI öffnen", command=self.open_ui).pack(side="left", padx=6)
        ttk.Label(device, text="„Gerät suchen“ prüft bereits bekannte Geräte im lokalen ARP-Netz. Alternativ Adresse aus dem Router eintragen (http://IP); die Firmware erlaubt IP-Hosts.", style="Hint.TLabel", wraplength=880).pack(anchor="w", pady=(7, 0))
        ttk.Button(device, text="Einrichtungs-Hotspot verwenden (192.168.4.1)", command=lambda: self.url.set("http://192.168.4.1")).pack(anchor="w", pady=(5, 0))

        notebook = ttk.Notebook(root); notebook.pack(fill="both", expand=True, pady=(0, 9))
        firmware = ttk.Frame(notebook, padding=12); themes = ttk.Frame(notebook, padding=12); photos = ttk.Frame(notebook, padding=12)
        notebook.add(firmware, text="Firmware flashen"); notebook.add(themes, text="Themes / .stheme"); notebook.add(photos, text="Foto-Diashow")
        self._firmware_tab(firmware); self._theme_tab(themes); self._photo_tab(photos)

        logframe = ttk.LabelFrame(root, text="Diagnose und Testprotokoll", style="Card.TLabelframe"); logframe.pack(fill="both", expand=True)
        self.log = tk.Text(logframe, height=11, wrap="word", bg="#101a13", fg="#d8efd9", insertbackground="white", font=("Consolas", 9), state="disabled")
        scroll = ttk.Scrollbar(logframe, command=self.log.yview); self.log.configure(yscrollcommand=scroll.set)
        self.log.pack(side="left", fill="both", expand=True); scroll.pack(side="right", fill="y")
        self._log("Bereit. Zuerst Gerät verbinden und API prüfen.")
        self._log("Serielle Diagnose und Foto-/Theme-Tests laufen gegen echte SmallTV-Endpunkte.")

    def _firmware_tab(self, frame):
        ttk.Label(frame, text="Für den SmallTV Ultra die .bin aus dem ESP8266-Target wählen, z. B. .pio/build/smalltv_settings/firmware.bin. OTA erhält LittleFS-Daten; serielles Flashen schreibt das Image ab Adresse 0x0.", wraplength=850).pack(anchor="w", pady=(0, 10))
        row = ttk.Frame(frame); row.pack(fill="x")
        ttk.Entry(row, textvariable=self.bin_path).pack(side="left", fill="x", expand=True)
        ttk.Button(row, text=".bin auswählen…", command=self.pick_bin).pack(side="left", padx=(8, 0))
        serial_box = ttk.LabelFrame(frame, text="USB / serielles Flashen", padding=10); serial_box.pack(fill="x", pady=12)
        row = ttk.Frame(serial_box); row.pack(fill="x")
        ttk.Label(row, text="COM-Port").pack(side="left")
        self.port_box = ttk.Combobox(row, textvariable=self.port, width=16, state="readonly"); self.port_box.pack(side="left", padx=6)
        ttk.Button(row, text="Ports aktualisieren", command=self.refresh_ports).pack(side="left", padx=4)
        ttk.Label(row, text="Baudrate").pack(side="left", padx=(16, 0))
        ttk.Combobox(row, textvariable=self.baud, values=("115200", "460800", "921600"), width=10).pack(side="left", padx=6)
        ttk.Checkbutton(serial_box, text="Vorher gesamten Flash löschen (löscht Settings, Fotos und Themes)", variable=self.erase_flash).pack(anchor="w", pady=(8, 3))
        ttk.Button(serial_box, text="ESP8266 per USB flashen", style="Accent.TButton", command=self.serial_flash).pack(anchor="w")
        ttk.Separator(frame).pack(fill="x", pady=5)
        ota_box = ttk.LabelFrame(frame, text="OTA über das Gerätnetzwerk", padding=10); ota_box.pack(fill="x", pady=7)
        ttk.Label(ota_box, text="Das Gerät muss erreichbar sein. Das hochgeladene .bin muss für das OTA-Layout dieser Firmware gebaut worden sein.", style="Hint.TLabel", wraplength=820).pack(anchor="w", pady=(0, 7))
        ttk.Button(ota_box, text="Firmware per OTA hochladen", style="Accent.TButton", command=self.ota_flash).pack(anchor="w")
        ttk.Button(frame, text="Serielles Log starten", command=self.start_monitor).pack(anchor="w", pady=(7, 0))

    def _theme_tab(self, frame):
        ttk.Label(frame, text="Die Datei wird an /api/themes/install übertragen und vom Gerät selbst mit seinem Theme-Parser geprüft.", wraplength=850).pack(anchor="w", pady=(0, 10))
        row = ttk.Frame(frame); row.pack(fill="x")
        ttk.Entry(row, textvariable=self.theme_path).pack(side="left", fill="x", expand=True)
        ttk.Button(row, text=".stheme auswählen…", command=self.pick_theme).pack(side="left", padx=(8, 0))
        row = ttk.Frame(frame); row.pack(fill="x", pady=9)
        ttk.Label(row, text="Bereich").pack(side="left")
        ttk.Combobox(row, textvariable=self.theme_kind, values=("general", "clock"), width=14, state="readonly").pack(side="left", padx=6)
        ttk.Checkbutton(row, text="Nach erfolgreicher Installation aktivieren", variable=self.activate_theme).pack(side="left", padx=10)
        ttk.Button(row, text="Theme installieren und prüfen", style="Accent.TButton", command=self.install_theme).pack(side="left", padx=8)
        ttk.Button(row, text="Installierte Themes auflisten", command=lambda: self.run_bg(self.list_themes)).pack(side="left")
        ttk.Label(frame, text="Der Test prüft Installation, Antwort-ID, erneuten Theme-Listeneintrag und Vorschau-Endpoint. Aktivierung wird anschließend im Status gegengeprüft.", style="Hint.TLabel", wraplength=850).pack(anchor="w", pady=9)

    def _photo_tab(self, frame):
        ttk.Label(frame, text="Wähle ein Bild; das Testwerkzeug beschneidet es wie die WebUI mittig auf 240×240 und sendet ein JPEG an die echte Firmware.", wraplength=850).pack(anchor="w", pady=(0, 10))
        row = ttk.Frame(frame); row.pack(fill="x")
        ttk.Entry(row, textvariable=self.photo_path).pack(side="left", fill="x", expand=True)
        ttk.Button(row, text="Bild auswählen…", command=self.pick_photo).pack(side="left", padx=(8, 0))
        row = ttk.Frame(frame); row.pack(fill="x", pady=9)
        ttk.Button(row, text="Foto hochladen und prüfen", style="Accent.TButton", command=self.upload_photo).pack(side="left")
        ttk.Button(row, text="Foto-Slideshow aktivieren", command=self.enable_slideshow).pack(side="left", padx=8)
        ttk.Button(row, text="Fotos am Gerät auflisten", command=lambda: self.run_bg(self.list_photos)).pack(side="left")
        ttk.Label(frame, text="Die Firmware zeigt Fotos nur im Theme-Modus innerhalb einer aktivierten Theme-Playlist. Der Slideshow-Schalter richtet diese Abhängigkeit ein und speichert die Einstellung am Gerät.", style="Hint.TLabel", wraplength=850).pack(anchor="w", pady=9)
        row = ttk.Frame(frame); row.pack(fill="x", pady=4)
        ttk.Label(row, text="Fotos pro Theme").pack(side="left")
        ttk.Combobox(row, textvariable=self.photo_count, values=("1", "2", "3", "4", "5", "6", "8", "10"), width=7, state="readonly").pack(side="left", padx=6)
        ttk.Label(row, text="Anzeigedauer je Foto (Sekunden; 0 = zufällig)").pack(side="left", padx=(12, 5))
        ttk.Combobox(row, textvariable=self.photo_seconds, values=("0", "5", "10", "15", "20", "30", "60"), width=7, state="readonly").pack(side="left")
        ttk.Button(frame, text="Geräte-WebUI öffnen", command=self.open_ui).pack(anchor="w", pady=(12, 0))

    def _log(self, message):
        stamp = time.strftime("%H:%M:%S")
        line = f"[{stamp}] {message}"
        self.log.configure(state="normal"); self.log.insert("end", line + "\n"); self.log.see("end"); self.log.configure(state="disabled")
        try:
            LOG_DIR.mkdir(parents=True, exist_ok=True)
            if self.log_path is None: self.log_path = LOG_DIR / (time.strftime("%Y%m%d-%H%M%S") + ".log")
            with self.log_path.open("a", encoding="utf-8") as f: f.write(line + "\n")
        except OSError: pass

    def _drain_events(self):
        try:
            while True:
                kind, message = self.events.get_nowait()
                if kind == "log": self._log(message)
                elif kind == "done": self._log(message)
                elif kind == "url":
                    self.url.set(message)
                    self._log("SmallTV-Adresse übernommen: " + message)
                elif kind == "error": self._log("FEHLER: " + message); messagebox.showerror(APP, message)
        except queue.Empty: pass
        self.after(100, self._drain_events)

    def run_bg(self, fn, *args):
        def run():
            try: fn(*args)
            except Exception as exc: self.events.put(("error", str(exc)))
        threading.Thread(target=run, daemon=True).start()

    def api(self): return DeviceApi(self.url.get(), self.user.get(), self.password.get())
    def pick_bin(self): self._pick(self.bin_path, "Firmware image", [("ESP8266 firmware", "*.bin"), ("Alle Dateien", "*.*")])
    def pick_theme(self): self._pick(self.theme_path, "SmallTV Theme", [("SmallTV Theme", "*.stheme")])
    def pick_photo(self): self._pick(self.photo_path, "Bild", [("Bilder", "*.jpg *.jpeg *.png *.bmp *.webp"), ("Alle Dateien", "*.*")])
    def _pick(self, var, title, types):
        p = filedialog.askopenfilename(title=title, filetypes=types)
        if p: var.set(p)

    def refresh_ports(self):
        if list_ports is None: ports = []
        else: ports = [p.device for p in list_ports.comports()]
        self.port_box["values"] = ports
        if ports and self.port.get() not in ports: self.port.set(ports[0])
        if not ports: self.port.set("")
        self._log("COM-Ports: " + (", ".join(ports) if ports else "kein USB-Seriellgerät erkannt"))

    def smoke_test(self):
        api = self.api(); self.events.put(("log", "Verbinde mit " + api.base))
        status = api.json("GET", "/api/status")[2]
        cfg = api.json("GET", "/api/config")[2]
        themes = api.json("GET", "/api/themes")[2]
        photos = api.json("GET", "/api/photos")[2]
        self.events.put(("log", f"PASS status: {status.get('fw','?')} {status.get('version','')} · {status.get('ip','?')} · WLAN={'online' if status.get('connected') else 'offline'}"))
        self.events.put(("log", f"PASS config: chip={cfg.get('chip','?')} · mode={cfg.get('mode','?')} · brightness={cfg.get('brightness','?')}%"))
        self.events.put(("log", f"PASS themes: {len(themes.get('themes', []))} installiert · aktiv={themes.get('selected') or '(keines)'}"))
        self.events.put(("log", f"PASS photos: {photos.get('count', '?')} installiert · {photos.get('photoBytes', 0)} Bytes"))
        self.events.put(("done", "API-Smoke-Test abgeschlossen. Keine Einstellungen wurden verändert."))

    def discover_device(self):
        """Find the firmware's mDNS HTTP service, then probe cached ARP neighbors."""
        services = []
        if Zeroconf is not None:
            class Listener(ServiceListener):
                def add_service(inner, zc, service_type, name):
                    try:
                        info = zc.get_service_info(service_type, name, timeout=700)
                        if not info: return
                        hostname = (info.server or name).rstrip(".")
                        for address in info.parsed_addresses():
                            if "." in address:
                                services.append((address, info.port, hostname))
                    except Exception: pass
                def update_service(inner, zc, service_type, name): inner.add_service(zc, service_type, name)
                def remove_service(inner, zc, service_type, name): pass
            zc = None; browser = None
            try:
                zc = Zeroconf()
                browser = ServiceBrowser(zc, "_http._tcp.local.", Listener())
                time.sleep(2.5)
            except Exception as exc:
                self.events.put(("log", "mDNS-Erkennung nicht verfügbar: " + str(exc)))
            finally:
                if browser is not None:
                    try: browser.cancel()
                    except Exception: pass
                if zc is not None:
                    try: zc.close()
                    except Exception: pass
        else:
            self.events.put(("log", "mDNS-Modul fehlt; nutze nur Windows-ARP-Tabelle. Install-Requirements.bat nachinstallieren."))

        try:
            arp = subprocess.run(["arp", "-a"], capture_output=True, text=True, timeout=5,
                                 encoding="utf-8", errors="replace", check=False).stdout
        except (OSError, subprocess.TimeoutExpired) as exc:
            raise RuntimeError("Windows-ARP-Tabelle konnte nicht gelesen werden: " + str(exc))
        rows = list(services)
        pattern = re.compile(r"^\s*(\d{1,3}(?:\.\d{1,3}){3})\s+([0-9a-fA-F-]{17})\s+\S+", re.M)
        for ip, mac in pattern.findall(arp):
            if ip.endswith(".255") or mac.lower() == "ff-ff-ff-ff-ff-ff": continue
            if ip not in [row[0] for row in rows]: rows.append((ip, 80, "ARP " + mac))
        rows = rows[:80]
        if not rows:
            raise RuntimeError("Kein mDNS-Gerät und keine lokalen ARP-Einträge gefunden. SmallTV ins gleiche WLAN bringen oder die Geräte-IP aus dem Router eintragen.")
        self.events.put(("log", f"Prüfe {len(rows)} mDNS-/ARP-Kandidaten; es wird kein IP-Bereich gescannt."))

        def probe(item):
            ip, port, source = item
            api = DeviceApi(f"http://{ip}:{port}" if port != 80 else "http://" + ip)
            try:
                _, _, value = api.json("GET", "/api/status", timeout=1)
                if all(k in value for k in ("fw", "version", "mode", "ip")):
                    return (ip, source, value, "SmallTV")
            except Exception as exc:
                msg = str(exc)
                if ("401" in msg or "Digest" in msg) and "smalltv" in source.lower():
                    return (ip, source, None, "SmallTV-Dienst (Digest-geschützt)")
            return None

        found = []
        with ThreadPoolExecutor(max_workers=8) as pool:
            futures = [pool.submit(probe, item) for item in rows]
            for future in as_completed(futures):
                candidate = future.result()
                if candidate: found.append(candidate)
        verified = [x for x in found if x[3] in ("SmallTV", "SmallTV-Dienst (Digest-geschützt)")]
        for item in found:
            ip, source, data, kind = item
            if data:
                self.events.put(("log", f"PASS {kind} bei {ip}: {data.get('fw')} {data.get('version')} · {source}"))
            else:
                self.events.put(("log", f"Kandidat {ip}: {kind} · {source}"))
        if len(verified) == 1:
            self.events.put(("url", "http://" + verified[0][0]))
            self.events.put(("done", "SmallTV eindeutig gefunden. Jetzt „API prüfen“ oder „WebUI öffnen“ drücken."))
        elif len(verified) > 1:
            self.events.put(("done", "Mehrere SmallTV-Geräte gefunden. Trage die gewünschte IP aus dem Protokoll in das Adressfeld ein."))
        elif found:
            self.events.put(("done", "Webserver-Kandidaten gefunden, aber nicht als SmallTV bestätigt. Prüfe die IP im Router und trage sie manuell ein."))
        else:
            raise RuntimeError("Kein SmallTV in den bereits bekannten Geräten gefunden. Prüfe, dass PC und SmallTV im selben WLAN sind, oder trage die IP-Adresse aus dem Router ein.")

    def open_ui(self, page=None):
        base = clean_url(self.url.get())
        if not base:
            return messagebox.showwarning(APP, "Zuerst „Gerät suchen“ drücken oder die SmallTV-IP-Adresse eintragen.")
        url = base + "/"
        webbrowser.open(url)
        self._log("WebUI im Browser geöffnet: " + url)

    def ota_flash(self):
        path = Path(self.bin_path.get())
        if not path.is_file(): return messagebox.showwarning(APP, "Bitte zuerst eine Firmware-.bin auswählen.")
        if path.stat().st_size < 16 * 1024: return messagebox.showerror(APP, "Das Image ist für das konfigurierte OTA zu klein.")
        if path.read_bytes()[:1] != b"\xe9": return messagebox.showerror(APP, "Das ist kein ESP8266-Firmware-Image (erwartete Image-Kennung 0xE9).")
        if not messagebox.askyesno("Firmware per OTA installieren", f"{path.name} ({path.stat().st_size:,} Bytes) an {clean_url(self.url.get())} senden?\n\nDas Gerät startet nach erfolgreichem Flash neu."):
            return
        self.run_bg(self._ota_worker, path)

    def _ota_worker(self, path):
        api = self.api(); self.events.put(("log", f"OTA startet: {path.name} ({path.stat().st_size} Bytes)"))
        _, _, response = api.multipart("/update", "firmware", path.name, path.read_bytes(), "application/octet-stream")
        text = response.get("text", "")
        if text.strip() != "OK": raise RuntimeError("OTA-Endpunkt meldete keinen Erfolg: " + (text or json.dumps(response)))
        self.events.put(("done", "OTA erfolgreich angenommen. Das SmallTV führt jetzt den Neustart aus."))

    def serial_flash(self):
        path = Path(self.bin_path.get())
        if not path.is_file(): return messagebox.showwarning(APP, "Bitte zuerst eine Firmware-.bin auswählen.")
        if path.stat().st_size < 16 * 1024 or path.read_bytes()[:1] != b"\xe9":
            return messagebox.showerror(APP, "Die ausgewählte Datei ist kein gültiges ESP8266-Firmware-Image.")
        port = self.port.get()
        if not port: return messagebox.showwarning(APP, "Kein COM-Port ausgewählt. SmallTV per USB anschließen und Ports aktualisieren.")
        if self.erase_flash.get():
            prompt = "Vor dem Schreiben den gesamten Flash löschen? Das entfernt dauerhaft WLAN-Settings, Fotos und installierte Themes."
        else:
            prompt = "Firmware über " + port + " nach 0x0 flashen? Settings und LittleFS werden nicht absichtlich gelöscht."
        if not messagebox.askyesno("Serielles Flashen bestätigen", prompt): return
        if not messagebox.askyesno("SmallTV Ultra / ESP8266", "Ist wirklich der SmallTV Ultra am gewählten COM-Port angeschlossen und in den Flash-Modus versetzt?"):
            return
        self.stop_monitor(log=False)
        self.run_bg(self._serial_worker, path, port, int(self.baud.get() or "460800"), self.erase_flash.get())

    def _esptool_command(self, port, baud, path):
        candidates = [Path.home() / ".platformio/packages/tool-esptoolpy/esptool.py",
            Path(__file__).resolve().parents[2] / ".pio/packages/tool-esptoolpy/esptool.py"]
        esptool = next((p for p in candidates if p.is_file()), None)
        if esptool:
            return [sys.executable, str(esptool), "--chip", "esp8266", "--port", port, "--baud", str(baud), "write_flash", "0x0", str(path)]
        if shutil.which("esptool"):
            return ["esptool", "--chip", "esp8266", "--port", port, "--baud", str(baud), "write-flash", "0x0", str(path)]
        if shutil.which("py"):
            return ["py", "-3", "-m", "esptool", "--chip", "esp8266", "--port", port, "--baud", str(baud), "write-flash", "0x0", str(path)]
        raise RuntimeError("esptool fehlt. Installiere PlatformIO/esptool (siehe README).")

    def _serial_worker(self, path, port, baud, erase):
        if self.monitor_thread and self.monitor_thread.is_alive(): self.monitor_thread.join(timeout=2)
        if erase:
            cmd = self._esptool_command(port, baud, path)
            cmd[-3:] = ["erase_flash"]
            self._run_process(cmd, "Flash löschen")
        cmd = self._esptool_command(port, baud, path)
        self._run_process(cmd, "ESP8266 schreiben")
        self.events.put(("done", "Serielles Flashen abgeschlossen. Gerät startet neu; API danach erneut prüfen."))

    def _run_process(self, cmd, label):
        self.events.put(("log", label + ": " + subprocess.list2cmdline(cmd)))
        proc = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True,
            encoding="utf-8", errors="replace", bufsize=1)
        for line in proc.stdout:
            line = line.rstrip()
            if line: self.events.put(("log", line))
        code = proc.wait()
        if code: raise RuntimeError(f"{label} fehlgeschlagen (Exit {code}); siehe Protokoll.")

    def start_monitor(self):
        if serial is None: return messagebox.showerror(APP, "pyserial ist nicht installiert.")
        if self.monitor_thread and self.monitor_thread.is_alive(): return self._log("Serieller Monitor läuft bereits.")
        if not self.port.get(): return messagebox.showwarning(APP, "Bitte zuerst einen COM-Port wählen.")
        self.monitor_stop.clear(); port = self.port.get()
        self.monitor_thread = threading.Thread(target=self._monitor_worker, args=(port,), daemon=True); self.monitor_thread.start()

    def _monitor_worker(self, port):
        try:
            with serial.Serial(port, 115200, timeout=0.5) as conn:
                self.events.put(("log", f"Serielles Log auf {port} geöffnet (115200 Baud)."))
                while not self.monitor_stop.is_set():
                    line = conn.readline()
                    if line: self.events.put(("log", "UART: " + line.decode("utf-8", "replace").rstrip()))
        except Exception as exc: self.events.put(("log", f"Serieller Monitor beendet: {exc}"))

    def stop_monitor(self, log=True):
        self.monitor_stop.set()
        if log and self.monitor_thread and self.monitor_thread.is_alive(): self._log("Stoppe seriellen Monitor …")

    def install_theme(self):
        path = Path(self.theme_path.get())
        if not path.is_file(): return messagebox.showwarning(APP, "Bitte eine .stheme-Datei auswählen.")
        if not path.name.lower().endswith(".stheme"): return messagebox.showerror(APP, "Die Datei muss auf .stheme enden.")
        self.run_bg(self._theme_worker, path, self.theme_kind.get(), self.activate_theme.get())

    def _theme_worker(self, path, kind, activate):
        api = self.api(); route = "/api/themes/install" + ("?category=clock" if kind == "clock" else "")
        self.events.put(("log", f"Lade .stheme hoch: {path.name}"))
        _, _, result = api.multipart(route, "theme", path.name, path.read_bytes(), "application/octet-stream")
        ident = result.get("id")
        if not ident: raise RuntimeError("Firmware hat keine Theme-ID zurückgegeben: " + json.dumps(result))
        self.events.put(("log", f"PASS Installation vom Gerät bestätigt: {ident}"))
        themes = api.json("GET", "/api/themes")[2].get("themes", [])
        found = next((t for t in themes if t.get("id") == ident), None)
        if not found or not found.get("valid"): raise RuntimeError("Theme ist nach Installation nicht als gültig gelistet.")
        self.events.put(("log", f"PASS Theme-Liste: {found.get('name', ident)} · {found.get('bytes', 0)} Bytes"))
        code, headers, preview = api.request("GET", "/api/themes/preview?id=" + urllib.parse.quote(ident))
        if code == 200 and headers.get_content_type() == "image/bmp":
            self.events.put(("log", f"PASS Geräte-Preview: {len(preview)} Bytes BMP"))
        else:
            self.events.put(("log", f"INFO keine Geräte-Vorschaudatei (HTTP {code}); Theme ist trotzdem validiert."))
        if activate:
            api.json("POST", "/api/themes/select", {"id": ident})
            status = api.json("GET", "/api/status")[2]
            if status.get("activeTheme") != ident:
                self.events.put(("log", f"INFO Aktivierung angenommen; Status meldet vor Playlist-Umschaltung {status.get('activeTheme')}"))
            else: self.events.put(("log", f"PASS Theme aktiv am Gerät: {ident}"))
        self.events.put(("done", "Theme-Test abgeschlossen. Die Anzeige am SmallTV und die Theme-Karten in der WebUI prüfen."))

    def list_themes(self):
        result = self.api().json("GET", "/api/themes")[2]
        entries = result.get("themes", [])
        for theme in entries:
            mark = "OK" if theme.get("valid") else "INVALID: " + theme.get("error", "")
            self.events.put(("log", f"Theme {theme.get('id')}: {theme.get('name')} · {theme.get('category')} · {mark}"))
        self.events.put(("done", f"Theme-Liste gelesen: {len(entries)} Einträge; ausgewählt={result.get('selected') or '(keines)'}"))

    def upload_photo(self):
        path = Path(self.photo_path.get())
        if not path.is_file(): return messagebox.showwarning(APP, "Bitte ein Bild auswählen.")
        if Image is None: return messagebox.showerror(APP, "Pillow fehlt. Starte Install-Requirements.bat.")
        self.run_bg(self._photo_worker, path)

    def _photo_worker(self, path):
        with Image.open(path) as source:
            source = ImageOps.exif_transpose(source)
            image = ImageOps.fit(source.convert("RGB"), (240, 240), method=Image.Resampling.LANCZOS, centering=(0.5, 0.5))
            import io
            buffer = io.BytesIO(); image.save(buffer, format="JPEG", quality=90, optimize=True)
            jpeg = buffer.getvalue()
        api = self.api(); self.events.put(("log", f"Foto zugeschnitten: 240x240 JPEG · {len(jpeg)} Bytes"))
        _, _, result = api.multipart("/api/photos/upload", "photo", path.stem + ".jpg", jpeg, "image/jpeg")
        ident = result.get("id")
        if ident is None: raise RuntimeError("Gerät hat keine Foto-ID zurückgegeben.")
        photos = api.json("GET", "/api/photos")[2]
        found = next((p for p in photos.get("photos", []) if p.get("id") == ident), None)
        if not found: raise RuntimeError("Upload kam zurück, Foto fehlt aber in der Gerätebibliothek.")
        self.events.put(("done", f"PASS Foto #{ident} im LittleFS installiert ({found.get('bytes', 0)} Bytes). Öffne die WebUI, um es zu prüfen."))

    def list_photos(self):
        result = self.api().json("GET", "/api/photos")[2]
        self.events.put(("log", f"Fotobibliothek: {result.get('count', 0)} Fotos · belegt {result.get('photoBytes', 0)} B · frei für Fotos {result.get('availableForPhotos', 0)} B"))
        for photo in result.get("photos", []): self.events.put(("log", f"Foto #{photo['id']}: {photo['bytes']} B"))
        self.events.put(("done", "Foto-Liste direkt vom Gerät gelesen."))

    def enable_slideshow(self):
        if not messagebox.askyesno("Foto-Diashow aktivieren", "Diese Aktion ändert und speichert die Geräte-Einstellungen: Fotos aktivieren und die Theme-Playlist einschalten. Fortfahren?"):
            return
        self.run_bg(self._enable_slideshow_worker, int(self.photo_count.get()), int(self.photo_seconds.get()))

    def _enable_slideshow_worker(self, per_theme, dwell):
        api = self.api(); cfg = api.json("GET", "/api/config")[2]
        photos = api.json("GET", "/api/photos")[2]
        if not photos.get("count"): raise RuntimeError("Zuerst mindestens ein Foto hochladen.")
        theme_data = api.json("GET", "/api/themes")[2]
        valid = [t for t in theme_data.get("themes", []) if t.get("valid")]
        if not valid: raise RuntimeError("Zuerst mindestens ein gültiges .stheme installieren.")
        playlist = cfg.get("themePlaylist") or {"enabled": False, "random": False, "clockDefaultSeconds": 300, "items": []}
        items = playlist.get("items") or []
        if any(item.get("scheduled") for item in items):
            raise RuntimeError("Die bestehende Playlist enthält Zeitpläne. Die Firmware priorisiert zeitgesteuerte Themes und spielt dazwischen keine Foto-Batches ab. Zeitpläne zuerst in der WebUI deaktivieren, wenn die Foto-Diashow gewünscht ist.")
        chosen = cfg.get("themeId") or theme_data.get("selected")
        if chosen not in [t["id"] for t in valid]: chosen = valid[0]["id"]
        if not items: items = [{"id": chosen, "seconds": 30, "scheduled": False, "startMinute": 0, "endMinute": 0}]
        playlist["items"] = items; playlist["enabled"] = True
        update = {"mode": "theme", "themeId": chosen, "themePlaylist": playlist,
            "photoPlayback": {"enabled": True, "perTheme": per_theme, "dwellSeconds": dwell}}
        api.json("POST", "/api/config", update)
        verify = api.json("GET", "/api/config")[2]
        if not verify.get("photoPlayback", {}).get("enabled") or not verify.get("themePlaylist", {}).get("enabled"):
            raise RuntimeError("Die Firmware hat die Foto-/Playlist-Einstellung nicht bestätigt.")
        self.events.put(("done", f"PASS Diashow konfiguriert: {per_theme} Fotos/Theme, {dwell or 'zufällige'} s je Foto, Playlist mit {len(items)} Theme(s). Am Gerät nach dem aktuellen Theme-Wechsel prüfen."))

    def close(self):
        self.stop_monitor(log=False)
        try:
            USER_DIR.mkdir(parents=True, exist_ok=True)
            PREFS_PATH.write_text(json.dumps({"url": clean_url(self.url.get())}, indent=2), encoding="utf-8")
        except OSError: pass
        self.destroy()


if __name__ == "__main__":
    Tester().mainloop()
