#include "Settings.h"
#include "AtomicJson.h"
#include "Platform.h"   // platformChipId() for the unique default hostname
#include <LittleFS.h>
#include <stdio.h>
#include <string.h>

static const char* CONFIG_PATH = "/config.json";
static bool g_storageReady = false;
// Bytes in CONFIG_PATH as last read or written: what the next save needs on
// the filesystem, known without building the settings document again.
static size_t g_storedSize = 0;
static char g_settingsSaveError[176] = "";

// ===========================================================================
// Ticker slice
// ===========================================================================
#if WITH_TICKER
static const char* srcToStr(uint8_t s) {
  return (s == SRC_YAHOO) ? "yahoo"
       : (s == SRC_CASH)  ? "cash"
       : (s == SRC_GHUB)  ? "github" : "webhook";
}
static uint8_t srcFromStr(const String& s) {
  return s.equalsIgnoreCase("yahoo")  ? SRC_YAHOO
       : s.equalsIgnoreCase("cash")   ? SRC_CASH
       : s.equalsIgnoreCase("github") ? SRC_GHUB : SRC_WEBHOOK;
}

void TickerSettings::setDefaults() {
  webhookUrl = "";
  range = DEFAULT_RANGE;
  points = DEFAULT_POINTS;
  pollSec = DEFAULT_POLL_SEC;
  rotateSec = DEFAULT_ROTATE_SEC;
  colorInverted = false;
  changeOnRange = true;

  showName = true;
  showPrice = true;
  showChange = true;
  showChart = true;
  showRangeLabel = true;
  showUpdatedAgo = false;
  showPageDots = true;
  showPortfolio = true;   // only visible once a symbol has qty+cost set

  symbolCount = 0;
  for (uint8_t i = 0; i < MAX_SYMBOLS; i++) {
    symbols[i].symbol[0] = 0;
    symbols[i].name[0] = 0;
    symbols[i].source = DEFAULT_SOURCE;
    symbols[i].qty = 0;
    symbols[i].cost = 0;
  }
}

void TickerSettings::toJson(JsonObject o) const {
  o["webhookUrl"]     = webhookUrl;
  o["range"]          = range;
  o["points"]         = points;
  o["pollSec"]        = pollSec;
  o["rotateSec"]      = rotateSec;
  o["colorInverted"]  = colorInverted;
  o["changeOnRange"]  = changeOnRange;
  o["showName"]       = showName;
  o["showPrice"]      = showPrice;
  o["showChange"]     = showChange;
  o["showChart"]      = showChart;
  o["showRangeLabel"] = showRangeLabel;
  o["showUpdatedAgo"] = showUpdatedAgo;
  o["showPageDots"]   = showPageDots;
  o["showPortfolio"]  = showPortfolio;

  JsonArray arr = o["symbols"].to<JsonArray>();
  for (uint8_t i = 0; i < symbolCount; i++) {
    JsonObject e = arr.add<JsonObject>();
    e["symbol"] = symbols[i].symbol;
    e["name"]   = symbols[i].name;
    e["source"] = srcToStr(symbols[i].source);
    e["qty"]    = symbols[i].qty;
    e["cost"]   = symbols[i].cost;
  }
}

void TickerSettings::fromJson(JsonObjectConst o) {
  // Legacy (pre-2.4) configs carried one global "source"; it becomes the
  // default for any symbol that doesn't carry its own.
  uint8_t legacySrc = DEFAULT_SOURCE;
  if (o["source"].is<const char*>()) legacySrc = srcFromStr(o["source"].as<String>());

  if (o["webhookUrl"].is<const char*>()) webhookUrl = o["webhookUrl"].as<String>();
  if (o["range"].is<const char*>())      range = o["range"].as<String>();
  if (o["points"].is<int>())             points = constrain((int)o["points"], 0, MAX_SPARK_POINTS);
  // Clamped at both ends: these land in a uint16_t, and the web UI's own
  // min/max are only advisory (the page POSTs JSON, it does not submit a form).
  if (o["pollSec"].is<int>())            pollSec = constrain((int)o["pollSec"], 10, 3600);
  if (o["rotateSec"].is<int>())          rotateSec = constrain((int)o["rotateSec"], 2, 3600);
  if (o["colorInverted"].is<bool>())     colorInverted = o["colorInverted"];
  if (o["changeOnRange"].is<bool>())     changeOnRange = o["changeOnRange"];

  if (o["showName"].is<bool>())       showName = o["showName"];
  if (o["showPrice"].is<bool>())      showPrice = o["showPrice"];
  if (o["showChange"].is<bool>())     showChange = o["showChange"];
  if (o["showChart"].is<bool>())      showChart = o["showChart"];
  if (o["showRangeLabel"].is<bool>()) showRangeLabel = o["showRangeLabel"];
  if (o["showUpdatedAgo"].is<bool>()) showUpdatedAgo = o["showUpdatedAgo"];
  if (o["showPageDots"].is<bool>())   showPageDots = o["showPageDots"];
  if (o["showPortfolio"].is<bool>())  showPortfolio = o["showPortfolio"];

  if (o["symbols"].is<JsonArrayConst>()) {
    JsonArrayConst arr = o["symbols"].as<JsonArrayConst>();
    symbolCount = 0;
    for (JsonObjectConst e : arr) {
      if (symbolCount >= MAX_SYMBOLS) break;
      const char* sym = e["symbol"] | "";
      if (!sym[0]) continue;                 // skip blank rows
      SymbolCfg& dst = symbols[symbolCount];
      strlcpy(dst.symbol, sym, MAX_SYMBOL_LEN);
      strlcpy(dst.name, e["name"] | "", MAX_NAME_LEN);
      dst.source = e["source"].is<const char*>()
                     ? srcFromStr(e["source"].as<String>()) : legacySrc;
      dst.qty  = e["qty"].as<float>();     // absent -> 0
      dst.cost = e["cost"].as<float>();
      if (dst.qty < 0)  dst.qty = 0;
      if (dst.cost < 0) dst.cost = 0;
      symbolCount++;
    }
  }
}
#endif

// ===========================================================================
// Usage slice
// ===========================================================================
#if WITH_USAGE
void UsageSettings::setDefaults() {
  usageUrl = "";
  pollSec = DEFAULT_POLL_SEC;
  hide5h = false;
}

void UsageSettings::toJson(JsonObject o) const {
  o["usageUrl"] = usageUrl;
  o["pollSec"]  = pollSec;
  o["hide5h"]   = hide5h;
}

void UsageSettings::fromJson(JsonObjectConst o) {
  if (o["usageUrl"].is<const char*>()) usageUrl = o["usageUrl"].as<String>();
  if (o["pollSec"].is<int>())          pollSec = constrain((int)o["pollSec"], 10, 3600);
  if (o["hide5h"].is<bool>())          hide5h = o["hide5h"];
}
#endif

// ===========================================================================
// Clock / night mode slice
// ===========================================================================
static uint16_t hhmmToMin(const char* s, uint16_t fallback) {
  if (!s || !s[0]) return fallback;
  int h = 0, m = 0;
  if (sscanf(s, "%d:%d", &h, &m) != 2) return fallback;
  if (h < 0 || h > 23 || m < 0 || m > 59) return fallback;
  return (uint16_t)(h * 60 + m);
}
static String minToHhmm(uint16_t v) {
  if (v > 1439) v = 0;
  char b[6];
  snprintf(b, sizeof(b), "%02u:%02u", (unsigned)(v / 60), (unsigned)(v % 60));
  return String(b);
}

void ClockSettings::setDefaults() {
  tz            = DEFAULT_TZ_NAME;
  tzPosix       = DEFAULT_TZ_POSIX;
  nightEnabled  = DEFAULT_NIGHT_ENABLED;
  nightStartMin = DEFAULT_NIGHT_START_MIN;
  nightEndMin   = DEFAULT_NIGHT_END_MIN;
  nightLevel    = DEFAULT_NIGHT_LEVEL;
  use24Hour = true;
  showSeconds = true;
  showDate = true;
  dateFormat = "{WD}, {DD} {MON} {YYYY}";
  lang = "de";
}

void ClockSettings::toJson(JsonObject o) const {
  o["tz"]           = tz;
  o["tzPosix"]      = tzPosix;
  o["nightEnabled"] = nightEnabled;
  o["nightStart"]   = minToHhmm(nightStartMin);
  o["nightEnd"]     = minToHhmm(nightEndMin);
  o["nightLevel"]   = nightLevel;
  o["use24Hour"] = use24Hour;
  o["showSeconds"] = showSeconds;
  o["showDate"] = showDate;
  o["dateFormat"] = dateFormat;
  o["lang"] = lang;
}

void ClockSettings::fromJson(JsonObjectConst o) {
  if (o["tz"].is<const char*>())          tz = o["tz"].as<String>();
  if (o["tzPosix"].is<const char*>())     tzPosix = o["tzPosix"].as<String>();
  if (o["nightEnabled"].is<bool>())       nightEnabled = o["nightEnabled"];
  if (o["nightStart"].is<const char*>())  nightStartMin = hhmmToMin(o["nightStart"], nightStartMin);
  if (o["nightEnd"].is<const char*>())    nightEndMin   = hhmmToMin(o["nightEnd"], nightEndMin);
  if (o["nightLevel"].is<int>())          nightLevel = constrain((int)o["nightLevel"], 0, 100);
  if (o["use24Hour"].is<bool>()) use24Hour = o["use24Hour"];
  if (o["showSeconds"].is<bool>()) showSeconds = o["showSeconds"];
  if (o["showDate"].is<bool>()) showDate = o["showDate"];
  if (o["dateFormat"].is<const char*>()) { dateFormat = o["dateFormat"].as<String>(); if (dateFormat.length() > 48) dateFormat.remove(48); }
  if (o["lang"].is<const char*>()) {
    const String candidate = o["lang"].as<String>();
    if (candidate == "en" || candidate == "pt" || candidate == "es" || candidate == "fr" || candidate == "de" || candidate == "it") lang = candidate;
  }
}

// ===========================================================================
// Physical button slice
// ===========================================================================
void ButtonSettings::setDefaults() {
  sensitivity = DEFAULT_BTN_SENS;
  shortAction = DEFAULT_BTN_SHORT;
  longAction  = DEFAULT_BTN_LONG;
}

void ButtonSettings::toJson(JsonObject o) const {
  o["sensitivity"] = sensitivity;
  o["shortAction"] = shortAction;
  o["longAction"]  = longAction;
}

void ButtonSettings::fromJson(JsonObjectConst o) {
  if (o["sensitivity"].is<int>()) sensitivity = constrain((int)o["sensitivity"], 50, 99);
  if (o["shortAction"].is<int>()) shortAction = constrain((int)o["shortAction"], 0, BTN_ACT_MAX);
  if (o["longAction"].is<int>())  longAction  = constrain((int)o["longAction"],  0, BTN_ACT_MAX);
}

// ===========================================================================
// Web UI password slice
// ===========================================================================
void AuthSettings::setDefaults() {
  enabled = false;
  user = DEFAULT_AUTH_USER;
  pass = "";
}

void AuthSettings::toJson(JsonObject o, bool includeSecrets) const {
  // Never enable in the saved config without a password to check against; a
  // blank one would lock the page with a credential nobody can supply.
  o["enabled"] = enabled && pass.length() > 0;
  o["user"]    = user;
  o["passSet"] = pass.length() > 0;
  if (includeSecrets) o["pass"] = pass;
}

void AuthSettings::fromJson(JsonObjectConst o) {
  if (o["user"].is<const char*>()) user = o["user"].as<String>();
  // Removing a password is explicit. A blank input still means "keep the
  // stored one", which prevents an unrelated settings save from unlocking
  // the device accidentally.
  if (o["clear"].is<bool>() && o["clear"].as<bool>()) {
    pass = "";
    enabled = false;
  }
  // Blank keeps the stored password, as everywhere else in this file.
  if (o["pass"].is<const char*>()) {
    String p = o["pass"].as<String>();
    if (p.length()) pass = p;
  }
  if (o["enabled"].is<bool>()) enabled = o["enabled"];
  if (user.length() >= MAX_AUTH_USER_LEN) user.remove(MAX_AUTH_USER_LEN - 1);
  if (pass.length() >= MAX_AUTH_PASS_LEN) pass.remove(MAX_AUTH_PASS_LEN - 1);
  if (!user.length()) user = DEFAULT_AUTH_USER;
  // Same guard as toJson, for a config imported by hand.
  if (!pass.length()) enabled = false;
}

// ===========================================================================
// Panel colour slice
// ===========================================================================
void DisplaySettings::setDefaults() {
  colorOrder = DEFAULT_COLOR_ORDER;
  invert     = DEFAULT_COLOR_INVERT;
  rGain = gGain = bGain = DEFAULT_COLOR_GAIN;
  contrast = saturation = 100;
  themeOpacityPct = 100;
  background = 0;
  backgroundEnabled = false;
  refreshFps = 15;
  panelRefreshHz = 60;
  animationsEnabled = true;
  animationSpeedPct = 100;
  themeFadeTransition = false;
  themeTransitionMs = 300;
}

void DisplaySettings::toJson(JsonObject o) const {
  o["colorOrder"] = (colorOrder == COLOR_ORDER_RGB) ? "rgb"
                  : (colorOrder == COLOR_ORDER_BGR) ? "bgr" : "auto";
  o["invert"] = invert;
  o["rGain"]  = rGain;
  o["gGain"]  = gGain;
  o["bGain"]  = bGain;
  o["contrast"] = contrast;
  o["saturation"] = saturation;
  o["themeOpacityPct"] = themeOpacityPct;
  char bg[8]; snprintf(bg, sizeof(bg), "#%02X%02X%02X", ((background >> 11) & 31) * 255 / 31, ((background >> 5) & 63) * 255 / 63, (background & 31) * 255 / 31);
  o["backgroundColor"] = bg;
  o["backgroundEnabled"] = backgroundEnabled;
  o["refreshFps"] = refreshFps;
  o["panelRefreshHz"] = panelRefreshHz;
  o["animationsEnabled"] = animationsEnabled;
  o["animationSpeedPct"] = animationSpeedPct;
  o["themeFadeTransition"] = themeFadeTransition;
  o["themeTransitionMs"] = themeTransitionMs;
}

void DisplaySettings::fromJson(JsonObjectConst o) {
  if (o["colorOrder"].is<const char*>()) {
    String c = o["colorOrder"].as<String>();
    colorOrder = c.equalsIgnoreCase("rgb") ? COLOR_ORDER_RGB
               : c.equalsIgnoreCase("bgr") ? COLOR_ORDER_BGR : COLOR_ORDER_AUTO;
  }
  if (o["invert"].is<bool>()) invert = o["invert"];
  if (o["rGain"].is<int>()) rGain = constrain((int)o["rGain"], MIN_COLOR_GAIN, MAX_COLOR_GAIN);
  if (o["gGain"].is<int>()) gGain = constrain((int)o["gGain"], MIN_COLOR_GAIN, MAX_COLOR_GAIN);
  if (o["bGain"].is<int>()) bGain = constrain((int)o["bGain"], MIN_COLOR_GAIN, MAX_COLOR_GAIN);
  if (o["contrast"].is<int>()) contrast = constrain((int)o["contrast"], 0, 200);
  if (o["saturation"].is<int>()) saturation = constrain((int)o["saturation"], 0, 200);
  if (o["themeOpacityPct"].is<int>()) themeOpacityPct = constrain((int)o["themeOpacityPct"], 0, 100);
  if (o["backgroundColor"].is<const char*>()) {
    const char* c = o["backgroundColor"];
    unsigned r=0,g=0,b=0;
    if (strlen(c)==7 && c[0]=='#' && sscanf(c+1,"%2x%2x%2x",&r,&g,&b)==3)
      background=static_cast<uint16_t>(((r>>3)<<11)|((g>>2)<<5)|(b>>3));
  }
  if (o["backgroundEnabled"].is<bool>()) backgroundEnabled = o["backgroundEnabled"];
  if (o["refreshFps"].is<int>()) refreshFps = constrain((int)o["refreshFps"], 1, 15);
  if (o["panelRefreshHz"].is<int>()) panelRefreshHz = constrain((int)o["panelRefreshHz"], 39, 60);
  if (o["animationsEnabled"].is<bool>()) animationsEnabled = o["animationsEnabled"];
  if (o["animationSpeedPct"].is<int>()) animationSpeedPct = constrain((int)o["animationSpeedPct"], 25, 200);
  if (o["themeFadeTransition"].is<bool>()) themeFadeTransition = o["themeFadeTransition"];
  if (o["themeTransitionMs"].is<int>()) themeTransitionMs = constrain((int)o["themeTransitionMs"], 100, 1000);
}

// ===========================================================================
// Home Assistant / MQTT slice
// ===========================================================================
#if WITH_HA
void HaSettings::setDefaults() {
  brokerHost = "";
  brokerPort = DEFAULT_HA_BROKER_PORT;
  brokerUser = "";
  brokerPass = "";
  dwellSec   = DEFAULT_HA_DWELL_SEC;
}

void HaSettings::toJson(JsonObject o, bool includeSecrets) const {
  o["brokerHost"] = brokerHost;
  o["brokerPort"] = brokerPort;
  o["brokerUser"] = brokerUser;
  o["passSet"]    = brokerPass.length() > 0;
  if (includeSecrets) o["brokerPass"] = brokerPass;
  o["dwellSec"]   = dwellSec;
}

void HaSettings::fromJson(JsonObjectConst o) {
  if (o["brokerHost"].is<const char*>()) brokerHost = o["brokerHost"].as<String>();
  if (o["brokerPort"].is<int>())         brokerPort = constrain((int)o["brokerPort"], 1, 65535);
  if (o["brokerUser"].is<const char*>()) brokerUser = o["brokerUser"].as<String>();
  // Blank keeps the stored password, as everywhere else in this file.
  if (o["brokerPass"].is<const char*>()) {
    String p = o["brokerPass"].as<String>();
    if (p.length()) brokerPass = p;
  }
  if (o["dwellSec"].is<int>()) dwellSec = constrain((int)o["dwellSec"], HA_DWELL_MIN_SEC, HA_DWELL_MAX_SEC);
  if (brokerHost.length() >= MAX_HA_HOST_LEN) brokerHost.remove(MAX_HA_HOST_LEN - 1);
  if (brokerUser.length() >= MAX_HA_USER_LEN) brokerUser.remove(MAX_HA_USER_LEN - 1);
  if (brokerPass.length() >= MAX_HA_PASS_LEN) brokerPass.remove(MAX_HA_PASS_LEN - 1);
}
#endif

// ===========================================================================
// Radar slice
// ===========================================================================
#if WITH_RADAR
void RadarSettings::setDefaults() {
  lat = DEFAULT_RADAR_LAT;
  lon = DEFAULT_RADAR_LON;
  source = DEFAULT_RADAR_SRC;
  webhookUrl = "";
  rangeKm = DEFAULT_RADAR_RANGE_KM;
  pollSec = DEFAULT_RADAR_POLL_SEC;
  unitsMi = false;
  showLabels = true;
  showVectors = true;
  showRimDots = true;
  uiScale = 1;            // medium
  minAltFt = 0;           // show all
  airportCount = 0;
  for (uint8_t i = 0; i < MAX_AIRPORTS; i++) {
    airports[i].icao[0] = 0;
    airports[i].lat = airports[i].lon = 0;
  }
}

void RadarSettings::toJson(JsonObject o) const {
  o["lat"]         = lat;
  o["lon"]         = lon;
  o["source"]      = (source == RADAR_SRC_WEBHOOK) ? "webhook"
                   : (source == RADAR_SRC_ADSBLOL) ? "adsblol" : "adsbfi";
  o["webhookUrl"]  = webhookUrl;
  o["rangeKm"]     = rangeKm;
  o["pollSec"]     = pollSec;
  o["unitsMi"]     = unitsMi;
  o["showLabels"]  = showLabels;
  o["showVectors"] = showVectors;
  o["showRimDots"] = showRimDots;
  o["uiScale"]     = uiScale;
  o["minAltFt"]    = minAltFt;

  JsonArray arr = o["airports"].to<JsonArray>();
  for (uint8_t i = 0; i < airportCount; i++) {
    JsonObject e = arr.add<JsonObject>();
    e["icao"] = airports[i].icao;
    e["lat"]  = airports[i].lat;
    e["lon"]  = airports[i].lon;
  }
}

void RadarSettings::fromJson(JsonObjectConst o) {
  if (o["lat"].is<float>() || o["lat"].is<int>()) lat = o["lat"].as<float>();
  if (o["lon"].is<float>() || o["lon"].is<int>()) lon = o["lon"].as<float>();
  if (o["source"].is<const char*>()) {
    String src = o["source"].as<String>();
    // "direct" is the pre-2.11 name for "whichever direct feed we default to".
    // Configs written back then predate the adsb.fi Cloudflare move, so they
    // resolve to the platform default rather than pinning adsb.fi.
    if      (src.equalsIgnoreCase("webhook")) source = RADAR_SRC_WEBHOOK;
    else if (src.equalsIgnoreCase("adsblol")) source = RADAR_SRC_ADSBLOL;
    else if (src.equalsIgnoreCase("adsbfi"))  source = RADAR_SRC_ADSBFI;
    else                                      source = DEFAULT_RADAR_SRC;
  }
  if (o["webhookUrl"].is<const char*>()) webhookUrl = o["webhookUrl"].as<String>();
  if (o["rangeKm"].is<int>())    rangeKm = constrain((int)o["rangeKm"], 1, 500);
  if (o["pollSec"].is<int>())    pollSec = constrain((int)o["pollSec"], 3, 3600);
  if (o["unitsMi"].is<bool>())   unitsMi = o["unitsMi"];
  if (o["showLabels"].is<bool>())  showLabels = o["showLabels"];
  if (o["showVectors"].is<bool>()) showVectors = o["showVectors"];
  if (o["showRimDots"].is<bool>()) showRimDots = o["showRimDots"];
  if (o["uiScale"].is<int>())      uiScale = constrain((int)o["uiScale"], 0, 2);
  if (o["minAltFt"].is<int>())     minAltFt = constrain((int)o["minAltFt"], 0, 60000);

  if (o["airports"].is<JsonArrayConst>()) {
    JsonArrayConst arr = o["airports"].as<JsonArrayConst>();
    airportCount = 0;
    for (JsonObjectConst e : arr) {
      if (airportCount >= MAX_AIRPORTS) break;
      const char* ic = e["icao"] | "";
      if (!ic[0]) continue;                  // skip blank rows
      Airport& dst = airports[airportCount];
      strlcpy(dst.icao, ic, MAX_ICAO_LEN);
      dst.lat = e["lat"].as<float>();
      dst.lon = e["lon"].as<float>();
      airportCount++;
    }
  }
}
#endif

// ===========================================================================
// Top-level settings
// ===========================================================================
void Settings::setDefaults() {
  wifiCount = 0;
  for (uint8_t i = 0; i < MAX_WIFI_NETS; i++) {
    wifi[i].ssid = "";
    wifi[i].pass = "";
  }
  apSsid  = DEFAULT_AP_SSID;
  apPass  = DEFAULT_AP_PASS;
  // Unique per device so several SmallTVs on one network don't collide on
  // mDNS out of the box. A hostname saved in config.json overrides this.
  hostname = String(DEFAULT_HOSTNAME) + "-" + String(platformChipId() & 0xFFFF, HEX);

  mode = DEFAULT_MODE;
  themeId = "";
#if WITH_THEME
  themeAliasCount = 0;
  memset(themeAliases, 0, sizeof(themeAliases));
#endif
  carouselSec = DEFAULT_CAROUSEL_SEC;
#if WITH_TICKER
  carouselTicker = true;
#endif
#if WITH_USAGE
  carouselUsage = true;
#endif
#if WITH_RADAR
  carouselRadar = true;
#endif
#if WITH_HA
  carouselHa = true;
#endif
#if WITH_THEME
  carouselTheme = false;   // opt in: needs an installed theme, else the slot shows an error
#endif
#if WITH_THEME_PLAYLIST
  showflowEnabled = false;
  showflowRandom = false;
  showflowCount = 0;
  memset(showflow, 0, sizeof(showflow));
#endif
  httpTimeout = DEFAULT_HTTP_TIMEOUT;

  brightness = DEFAULT_BRIGHTNESS;
  backlightInverted = TFT_BL_DEFAULT_INVERTED;
  rotation = 0;

#if WITH_TICKER
  ticker.setDefaults();
#endif
#if WITH_USAGE
  usage.setDefaults();
#endif
#if WITH_RADAR
  radar.setDefaults();
#endif
#if WITH_HA
  ha.setDefaults();
#endif
  clock.setDefaults();
  display.setDefaults();
  auth.setDefaults();
  button.setDefaults();
}

// ---------------------------------------------------------------------------
bool settingsBegin() {
#if defined(SMALLTV_FAMILY_ESP8266)
  // ESP8266 LittleFS defaults to autoFormat=true. A transient mount failure
  // must never erase the user's photo, theme and settings partition.
  if (!LittleFS.setConfig(LittleFSConfig(false))) {
    g_storageReady = false;
    return false;
  }
  g_storageReady = LittleFS.begin();
#elif defined(SMALLTV_FAMILY_ESP32)
  // ESP32 exposes formatOnFail directly; keep it disabled as well.
  g_storageReady = LittleFS.begin(false);
#else
  g_storageReady = LittleFS.begin();
#endif
  return g_storageReady;
}

bool settingsStorageReady() {
  return g_storageReady;
}

bool settingsFormatStorage() {
  // This path is intentionally explicit/destructive and only called by the
  // user-requested recovery endpoint. Mount itself still never auto-formats.
  if (g_storageReady || !LittleFS.format()) return false;
  return settingsBegin();
}

bool loadSettings(Settings& s) {
  s.setDefaults();
  if (!g_storageReady) return false;
  File f = LittleFS.open(CONFIG_PATH, "r");
  if (!f) return false;
  g_storedSize = f.size();

  JsonDocument doc;
  DeserializationError err = deserializeJson(doc, f);
  f.close();
  if (err) return false;

  settingsApplyJson(s, doc.as<JsonObjectConst>());
  return true;
}

namespace {
uint32_t settingsFilesystemFreeBytes() {
#if defined(SMALLTV_ESP8266)
  FSInfo info;
  if (!LittleFS.info(info) || info.totalBytes < info.usedBytes) return UINT32_MAX;
  return info.totalBytes - info.usedBytes;
#else
  const uint32_t total = LittleFS.totalBytes();
  const uint32_t used = LittleFS.usedBytes();
  return total >= used ? total - used : UINT32_MAX;
#endif
}
void setSettingsSaveError(AtomicJsonResult result, size_t expected, uint32_t freeBytes) {
  if (result == AtomicJsonResult::DocumentOverflow) {
    strlcpy(g_settingsSaveError, "not enough heap to build the complete settings JSON", sizeof(g_settingsSaveError));
    return;
  }
  if (result == AtomicJsonResult::TemporaryOpenFailed || result == AtomicJsonResult::ShortWrite) {
    if (freeBytes != UINT32_MAX && freeBytes < expected) {
      snprintf(g_settingsSaveError, sizeof(g_settingsSaveError),
               "LittleFS space too low for atomic save: %lu B free, %lu B JSON",
               (unsigned long)freeBytes, (unsigned long)expected);
    } else if (result == AtomicJsonResult::TemporaryOpenFailed) {
      strlcpy(g_settingsSaveError, "cannot create /config.json.tmp; check LittleFS capacity and I/O", sizeof(g_settingsSaveError));
    } else {
      strlcpy(g_settingsSaveError, "short write to /config.json.tmp; check LittleFS capacity and flash I/O", sizeof(g_settingsSaveError));
    }
    return;
  }
  switch (result) {
    case AtomicJsonResult::FlushFailed:
      strlcpy(g_settingsSaveError, "LittleFS flush failed; flash write error", sizeof(g_settingsSaveError));
      break;
    case AtomicJsonResult::SizeMismatch:
      strlcpy(g_settingsSaveError, "temporary settings file has an unexpected size", sizeof(g_settingsSaveError));
      break;
    case AtomicJsonResult::RenameFailed:
      strlcpy(g_settingsSaveError, "LittleFS atomic rename failed; old configuration was kept", sizeof(g_settingsSaveError));
      break;
    default:
      strlcpy(g_settingsSaveError, "unknown atomic settings storage error", sizeof(g_settingsSaveError));
      break;
  }
}
}

bool saveSettings(const Settings& s) {
  PlatformMemoryPhaseScope saveMemory(PLATFORM_MEM_SETTINGS_SAVE, true);
  g_settingsSaveError[0] = '\0';
  if (!g_storageReady) {
    strlcpy(g_settingsSaveError, "LittleFS is not mounted", sizeof(g_settingsSaveError));
    return false;
  }
  JsonDocument doc;
  JsonObject root = doc.to<JsonObject>();
  settingsToJson(s, root, /*includeSecrets=*/true);
  const size_t expected = measureJson(doc);
  const uint32_t freeBytes = settingsFilesystemFreeBytes();
  const AtomicJsonResult result =
      saveJsonAtomicallyDetailed(LittleFS, doc, CONFIG_PATH, "/config.json.tmp");
  if (result != AtomicJsonResult::Ok) {
    setSettingsSaveError(result, expected, freeBytes);
    return false;
  }
  g_storedSize = expected;
  return true;
}

const char* settingsSaveError() { return g_settingsSaveError; }

// Arduino String copy constructors can fail their internal malloc without
// making the containing Settings allocation return nullptr. Verify every
// persisted String before allowing a staged copy to be mutated or saved.
bool settingsCopyValid(const Settings& source, const Settings& candidate) {
  for (uint8_t i = 0; i < MAX_WIFI_NETS; ++i) {
    if (source.wifi[i].ssid != candidate.wifi[i].ssid ||
        source.wifi[i].pass != candidate.wifi[i].pass) return false;
  }
  if (source.apSsid != candidate.apSsid || source.apPass != candidate.apPass ||
      source.hostname != candidate.hostname || source.themeId != candidate.themeId ||
      source.clock.tz != candidate.clock.tz || source.clock.tzPosix != candidate.clock.tzPosix ||
      source.clock.dateFormat != candidate.clock.dateFormat || source.clock.lang != candidate.clock.lang ||
      source.auth.user != candidate.auth.user || source.auth.pass != candidate.auth.pass) return false;
#if WITH_TICKER
  if (source.ticker.webhookUrl != candidate.ticker.webhookUrl ||
      source.ticker.range != candidate.ticker.range) return false;
#endif
#if WITH_USAGE
  if (source.usage.usageUrl != candidate.usage.usageUrl) return false;
#endif
#if WITH_RADAR
  if (source.radar.webhookUrl != candidate.radar.webhookUrl) return false;
#endif
#if WITH_HA
  if (source.ha.brokerHost != candidate.ha.brokerHost ||
      source.ha.brokerUser != candidate.ha.brokerUser ||
      source.ha.brokerPass != candidate.ha.brokerPass) return false;
#endif
  return true;
}

namespace {
bool settingsJsonStringEquals(const String& actual, const char* expected, size_t maxLength = SIZE_MAX) {
  if (!expected) return false;
  size_t length = strlen(expected);
  if (length > maxLength) length = maxLength;
  return actual.length() == length &&
         (length == 0 || memcmp(actual.c_str(), expected, length) == 0);
}
bool validSettingsThemeId(const char* id) {
  if (!id) return false;
  const size_t length = strlen(id);
  if (length > 48) return false;
  for (size_t i = 0; i < length; ++i) {
    const char c = id[i];
    if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
          (c >= '0' && c <= '9') || c == '-' || c == '_')) return false;
  }
  return true;
}
bool validClockLanguage(const char* lang) {
  return lang && (!strcmp(lang, "en") || !strcmp(lang, "pt") ||
      !strcmp(lang, "es") || !strcmp(lang, "fr") ||
      !strcmp(lang, "de") || !strcmp(lang, "it"));
}
bool settingsTextEqualsNoCase(const char* actual, const char* expected) {
  if (!actual || !expected) return false;
  while (*actual && *expected) {
    char a = *actual++, b = *expected++;
    if (a >= 'A' && a <= 'Z') a = static_cast<char>(a + ('a' - 'A'));
    if (b >= 'A' && b <= 'Z') b = static_cast<char>(b + ('a' - 'A'));
    if (a != b) return false;
  }
  return *actual == 0 && *expected == 0;
}
#if WITH_TICKER
uint8_t settingsTickerSource(const char* source) {
  if (settingsTextEqualsNoCase(source, "yahoo")) return SRC_YAHOO;
  if (settingsTextEqualsNoCase(source, "cash")) return SRC_CASH;
  if (settingsTextEqualsNoCase(source, "github")) return SRC_GHUB;
  return SRC_WEBHOOK;
}
#endif
#if WITH_RADAR
uint8_t settingsRadarSource(const char* source) {
  if (settingsTextEqualsNoCase(source, "webhook")) return RADAR_SRC_WEBHOOK;
  if (settingsTextEqualsNoCase(source, "adsblol")) return RADAR_SRC_ADSBLOL;
  if (settingsTextEqualsNoCase(source, "adsbfi")) return RADAR_SRC_ADSBFI;
  return DEFAULT_RADAR_SRC;
}
#endif
}

// Apply-side validation catches a second low-heap failure mode: Arduino String
// assignments can fail after the Settings copy has passed its initial check.
// Compare the candidate directly with the still-live JSON input before the
// document is released or the candidate is persisted.
bool settingsAppliedStringsValid(const Settings& original, const Settings& candidate,
                                 JsonObjectConst root) {
  if (root["mode"].is<const char*>()) {
    const char* mode = root["mode"].as<const char*>();
#if defined(SMALLTV_CLEAN)
    const uint8_t expectedMode = settingsTextEqualsNoCase(mode, "carousel")
        ? MODE_CAROUSEL : MODE_THEME;
    if (candidate.mode != expectedMode) return false;
#else
    if (settingsTextEqualsNoCase(mode, "theme") && candidate.mode != MODE_THEME) return false;
#if WITH_RADAR
    if (settingsTextEqualsNoCase(mode, "radar") && candidate.mode != MODE_RADAR) return false;
#endif
#if WITH_USAGE
    if (settingsTextEqualsNoCase(mode, "usage") && candidate.mode != MODE_USAGE) return false;
#endif
#if WITH_HA
    if (settingsTextEqualsNoCase(mode, "ha") && candidate.mode != MODE_HA) return false;
#endif
    if (settingsTextEqualsNoCase(mode, "carousel") && candidate.mode != MODE_CAROUSEL) return false;
    if (settingsTextEqualsNoCase(mode, "stocks") && candidate.mode != MODE_STOCKS) return false;
#endif
  }
  if (root["hostname"].is<const char*>() &&
      !settingsJsonStringEquals(candidate.hostname, root["hostname"].as<const char*>())) return false;
  if (root["apSsid"].is<const char*>() &&
      !settingsJsonStringEquals(candidate.apSsid, root["apSsid"].as<const char*>())) return false;
  if (root["apPass"].is<const char*>() &&
      !settingsJsonStringEquals(candidate.apPass, root["apPass"].as<const char*>())) return false;

  if (root["wifi"].is<JsonArrayConst>()) {
    uint8_t expectedCount = 0;
    for (JsonObjectConst row : root["wifi"].as<JsonArrayConst>()) {
      if (expectedCount >= MAX_WIFI_NETS) break;
      const char* ssid = row["ssid"] | "";
      if (!ssid[0]) continue;
      if (expectedCount >= candidate.wifiCount ||
          !settingsJsonStringEquals(candidate.wifi[expectedCount].ssid, ssid)) return false;

      const char* pass = row["pass"] | "";
      if (pass[0]) {
        if (!settingsJsonStringEquals(candidate.wifi[expectedCount].pass, pass)) return false;
      } else {
        const char* retained = "";
        for (uint8_t old = 0; old < original.wifiCount; ++old) {
          if (original.wifi[old].ssid == ssid) {
            retained = original.wifi[old].pass.c_str();
            break;
          }
        }
        if (!settingsJsonStringEquals(candidate.wifi[expectedCount].pass, retained)) return false;
      }
      ++expectedCount;
    }
    if (candidate.wifiCount != expectedCount) return false;
  } else if (root["staSsid"].is<const char*>()) {
    const char* ssid = root["staSsid"].as<const char*>();
    if (ssid && ssid[0]) {
      if (!candidate.wifiCount || !settingsJsonStringEquals(candidate.wifi[0].ssid, ssid)) return false;
      const char* pass = root["staPass"] | "";
      if (pass[0] && !settingsJsonStringEquals(candidate.wifi[0].pass, pass)) return false;
      if (!pass[0]) {
        const char* retained = original.wifiCount && original.wifi[0].ssid == ssid
            ? original.wifi[0].pass.c_str() : "";
        if (!settingsJsonStringEquals(candidate.wifi[0].pass, retained)) return false;
      }
    }
  }

  if (root["themeId"].is<const char*>()) {
    const char* id = root["themeId"].as<const char*>();
    if (validSettingsThemeId(id) && !settingsJsonStringEquals(candidate.themeId, id)) return false;
  }
  if (root["display"].is<JsonObjectConst>()) {
    JsonObjectConst display = root["display"].as<JsonObjectConst>();
    if (display["colorOrder"].is<const char*>()) {
      const char* order = display["colorOrder"].as<const char*>();
      const uint8_t expected = settingsTextEqualsNoCase(order, "rgb") ? COLOR_ORDER_RGB
          : settingsTextEqualsNoCase(order, "bgr") ? COLOR_ORDER_BGR : COLOR_ORDER_AUTO;
      if (candidate.display.colorOrder != expected) return false;
    }
  }
  if (root["clock"].is<JsonObjectConst>()) {
    JsonObjectConst clock = root["clock"].as<JsonObjectConst>();
    if (clock["tz"].is<const char*>() &&
        !settingsJsonStringEquals(candidate.clock.tz, clock["tz"].as<const char*>())) return false;
    if (clock["tzPosix"].is<const char*>() &&
        !settingsJsonStringEquals(candidate.clock.tzPosix, clock["tzPosix"].as<const char*>())) return false;
    if (clock["dateFormat"].is<const char*>() &&
        !settingsJsonStringEquals(candidate.clock.dateFormat, clock["dateFormat"].as<const char*>(), 48)) return false;
    if (clock["lang"].is<const char*>()) {
      const char* lang = clock["lang"].as<const char*>();
      if (validClockLanguage(lang) &&
          !settingsJsonStringEquals(candidate.clock.lang, lang)) return false;
    }
  }
  if (root["auth"].is<JsonObjectConst>()) {
    JsonObjectConst auth = root["auth"].as<JsonObjectConst>();
    if (auth["user"].is<const char*>()) {
      const char* user = auth["user"].as<const char*>();
      if (!user || !user[0]) user = DEFAULT_AUTH_USER;
      if (!settingsJsonStringEquals(candidate.auth.user, user, MAX_AUTH_USER_LEN - 1)) return false;
    }
    if (auth["clear"].is<bool>() && auth["clear"].as<bool>()) {
      const char* pass = auth["pass"] | "";
      if (pass[0]) {
        if (!settingsJsonStringEquals(candidate.auth.pass, pass, MAX_AUTH_PASS_LEN - 1)) return false;
      } else if (candidate.auth.pass.length() != 0) return false;
    } else if (auth["pass"].is<const char*>()) {
      const char* pass = auth["pass"].as<const char*>();
      if (pass && pass[0] &&
          !settingsJsonStringEquals(candidate.auth.pass, pass, MAX_AUTH_PASS_LEN - 1)) return false;
      if ((!pass || !pass[0]) &&
          !settingsJsonStringEquals(candidate.auth.pass, original.auth.pass.c_str())) return false;
    }
  }

#if WITH_TICKER
  {
    JsonObjectConst ticker = root["ticker"].is<JsonObjectConst>()
        ? root["ticker"].as<JsonObjectConst>() : root;
    if (root["ticker"].is<JsonObjectConst>() || ticker["symbols"].is<JsonArrayConst>() ||
        ticker["source"].is<const char*>()) {
      if (ticker["webhookUrl"].is<const char*>() &&
          !settingsJsonStringEquals(candidate.ticker.webhookUrl, ticker["webhookUrl"].as<const char*>())) return false;
      if (ticker["range"].is<const char*>() &&
          !settingsJsonStringEquals(candidate.ticker.range, ticker["range"].as<const char*>())) return false;
      const uint8_t legacySource = ticker["source"].is<const char*>()
          ? settingsTickerSource(ticker["source"].as<const char*>()) : DEFAULT_SOURCE;
      if (ticker["symbols"].is<JsonArrayConst>()) {
        uint8_t index = 0;
        for (JsonObjectConst symbol : ticker["symbols"].as<JsonArrayConst>()) {
          if (index >= MAX_SYMBOLS) break;
          const char* name = symbol["symbol"] | "";
          if (!name[0]) continue;
          const uint8_t expectedSource = symbol["source"].is<const char*>()
              ? settingsTickerSource(symbol["source"].as<const char*>()) : legacySource;
          if (index >= candidate.ticker.symbolCount ||
              candidate.ticker.symbols[index].source != expectedSource) return false;
          ++index;
        }
        if (candidate.ticker.symbolCount != index) return false;
      }
    }
  }
#endif
#if WITH_USAGE
  if (root["usage"].is<JsonObjectConst>() &&
      root["usage"]["usageUrl"].is<const char*>() &&
      !settingsJsonStringEquals(candidate.usage.usageUrl, root["usage"]["usageUrl"].as<const char*>())) return false;
#endif
#if WITH_RADAR
  if (root["radar"].is<JsonObjectConst>()) {
    JsonObjectConst radar = root["radar"].as<JsonObjectConst>();
    if (radar["webhookUrl"].is<const char*>() &&
        !settingsJsonStringEquals(candidate.radar.webhookUrl, radar["webhookUrl"].as<const char*>())) return false;
    if (radar["source"].is<const char*>() &&
        candidate.radar.source != settingsRadarSource(radar["source"].as<const char*>())) return false;
  }
#endif
#if WITH_HA
  if (root["ha"].is<JsonObjectConst>()) {
    JsonObjectConst ha = root["ha"].as<JsonObjectConst>();
    if (ha["brokerHost"].is<const char*>() &&
        !settingsJsonStringEquals(candidate.ha.brokerHost, ha["brokerHost"].as<const char*>(), MAX_HA_HOST_LEN - 1)) return false;
    if (ha["brokerUser"].is<const char*>() &&
        !settingsJsonStringEquals(candidate.ha.brokerUser, ha["brokerUser"].as<const char*>(), MAX_HA_USER_LEN - 1)) return false;
    if (ha["brokerPass"].is<const char*>() && ha["brokerPass"].as<const char*>()[0] &&
        !settingsJsonStringEquals(candidate.ha.brokerPass, ha["brokerPass"].as<const char*>(), MAX_HA_PASS_LEN - 1)) return false;
  }
#endif
  return true;
}

void factoryReset(Settings& s) {
  // Keep the persisted file intact until the caller has successfully written
  // the defaults. saveSettings() publishes through an atomic temp-file rename.
  s.setDefaults();
}

size_t settingsStoredSize() { return g_storedSize; }

// ---------------------------------------------------------------------------
void settingsToJson(const Settings& s, JsonObject root, bool includeSecrets) {
  root["hostname"]   = s.hostname;

  // WiFi networks. Passwords only reach the config file, never the web API.
  JsonArray wf = root["wifi"].to<JsonArray>();
  for (uint8_t i = 0; i < s.wifiCount; i++) {
    JsonObject e = wf.add<JsonObject>();
    e["ssid"]    = s.wifi[i].ssid;
    e["passSet"] = s.wifi[i].pass.length() > 0;
    if (includeSecrets) e["pass"] = s.wifi[i].pass;
  }
  // Legacy mirror of the primary network, kept for one release so a firmware
  // downgrade still finds its WiFi in config.json.
  root["staSsid"]    = s.wifiCount ? s.wifi[0].ssid : "";
  root["staPassSet"] = s.wifiCount && s.wifi[0].pass.length() > 0;
  root["apSsid"]     = s.apSsid;
  root["apPassSet"]  = s.apPass.length() > 0;
  if (includeSecrets) {
    root["staPass"]  = s.wifiCount ? s.wifi[0].pass : "";
    root["apPass"]   = s.apPass;
  }

  // Mode + shared HTTP/display
  root["themeId"] = s.themeId;
#if WITH_THEME
  JsonArray aliases = root["themeAliases"].to<JsonArray>();
  for (uint8_t i = 0; i < s.themeAliasCount; ++i) {
    JsonObject alias = aliases.add<JsonObject>();
    alias["id"] = s.themeAliases[i].id;
    alias["name"] = s.themeAliases[i].name;
  }
#endif
#if defined(SMALLTV_CLEAN)
  // Older config files can still name a feature this build no longer ships.
  // Report the first available display mode so the web UI selects Theme clock.
  root["mode"] = (s.mode == MODE_CAROUSEL) ? "carousel" : "theme";
#else
  root["mode"]              = (s.mode == MODE_THEME)    ? "theme"
                            : (s.mode == MODE_RADAR)    ? "radar"
                            : (s.mode == MODE_USAGE)    ? "usage"
                            : (s.mode == MODE_HA)       ? "ha"
                            : (s.mode == MODE_CAROUSEL) ? "carousel" : "stocks";
#endif
  root["carouselSec"]       = s.carouselSec;
#if WITH_TICKER
  root["carouselTicker"]    = s.carouselTicker;
#endif
#if WITH_USAGE
  root["carouselUsage"]     = s.carouselUsage;
#endif
#if WITH_RADAR
  root["carouselRadar"]     = s.carouselRadar;
#endif
#if WITH_HA
  root["carouselHa"]        = s.carouselHa;
#endif
#if WITH_THEME
  root["carouselTheme"]     = s.carouselTheme;
#endif
#if WITH_THEME_PLAYLIST
  JsonObject flow = root["showflow"].to<JsonObject>();
  flow["enabled"] = s.showflowEnabled;
  flow["random"] = s.showflowRandom;
  JsonArray steps = flow["steps"].to<JsonArray>();
  for (uint8_t i = 0; i < s.showflowCount; ++i) {
    const auto& step = s.showflow[i];
    JsonObject item = steps.add<JsonObject>();
    item["type"] = step.type == Settings::ShowflowPhoto ? "photo" : "theme";
    item["seconds"] = step.seconds;
    if (step.type == Settings::ShowflowPhoto) {
      item["random"] = step.randomPhoto;
      if (!step.randomPhoto) item["photoId"] = step.photoId;
    } else {
      item["id"] = step.themeId;
    }
  }
#endif
  root["httpTimeout"]       = s.httpTimeout;
  root["brightness"]        = s.brightness;
  root["backlightInverted"] = s.backlightInverted;
  root["rotation"]          = s.rotation;

  // Feature slices
#if WITH_TICKER
  s.ticker.toJson(root["ticker"].to<JsonObject>());
#endif
#if WITH_USAGE
  s.usage.toJson(root["usage"].to<JsonObject>());
#endif
#if WITH_RADAR
  s.radar.toJson(root["radar"].to<JsonObject>());
#endif
#if WITH_HA
  s.ha.toJson(root["ha"].to<JsonObject>(), includeSecrets);
#endif
  s.clock.toJson(root["clock"].to<JsonObject>());
  s.display.toJson(root["display"].to<JsonObject>());
  s.auth.toJson(root["auth"].to<JsonObject>(), includeSecrets);
  s.button.toJson(root["button"].to<JsonObject>());
}

// Apply only the keys that are present (partial update friendly). Accepts both
// the nested layout and the legacy flat layout (feature keys at the top level).
void settingsApplyJson(Settings& s, JsonObjectConst root) {
  if (root["hostname"].is<const char*>()) s.hostname = root["hostname"].as<String>();

  if (root["wifi"].is<JsonArrayConst>()) {
    // The list is authoritative when present (order = try priority, missing
    // row = deletion). A blank password keeps the stored one, matched by SSID
    // so rows survive reordering.
    WifiCred old[MAX_WIFI_NETS];
    uint8_t oldCount = s.wifiCount;
    for (uint8_t i = 0; i < oldCount; i++) old[i] = s.wifi[i];

    s.wifiCount = 0;
    for (JsonObjectConst e : root["wifi"].as<JsonArrayConst>()) {
      if (s.wifiCount >= MAX_WIFI_NETS) break;
      const char* ssid = e["ssid"] | "";
      if (!ssid[0]) continue;                // skip blank rows
      WifiCred& dst = s.wifi[s.wifiCount];
      dst.ssid = ssid;
      const char* pass = e["pass"] | "";
      dst.pass = pass;
      if (!pass[0])
        for (uint8_t i = 0; i < oldCount; i++)
          if (old[i].ssid == dst.ssid) { dst.pass = old[i].pass; break; }
      s.wifiCount++;
    }
  } else if (root["staSsid"].is<const char*>()) {
    // Legacy single-network layout (pre-2.4 config.json or an old cached web
    // page): it becomes/updates the primary network, extras stay untouched.
    String ssid = root["staSsid"].as<String>();
    if (ssid.length()) {
      s.wifi[0].ssid = ssid;
      if (root["staPass"].is<const char*>()) {
        String p = root["staPass"].as<String>();
        if (p.length() > 0) s.wifi[0].pass = p;   // blank = keep
      }
      if (s.wifiCount < 1) s.wifiCount = 1;
    }
  }
  if (root["apSsid"].is<const char*>()) s.apSsid = root["apSsid"].as<String>();
  // AP password: apply as-is when present (empty allowed => open AP).
  if (root["apPass"].is<const char*>()) s.apPass = root["apPass"].as<String>();

  if (root["mode"].is<const char*>()) {
    String m = root["mode"].as<String>();
#if defined(SMALLTV_CLEAN)
    s.mode = m.equalsIgnoreCase("carousel") ? MODE_CAROUSEL : MODE_THEME;
#else
    s.mode = m.equalsIgnoreCase("theme")    ? MODE_THEME
           : m.equalsIgnoreCase("radar")    ? MODE_RADAR
           : m.equalsIgnoreCase("usage")    ? MODE_USAGE
           : m.equalsIgnoreCase("ha")       ? MODE_HA
           : m.equalsIgnoreCase("carousel") ? MODE_CAROUSEL : MODE_STOCKS;
#endif
  }
  if (root["themeId"].is<const char*>()) {
    String id = root["themeId"].as<String>();
    bool valid = id.length() <= 48;
    for (size_t i = 0; i < id.length(); ++i) {
      char c = id[i];
      if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '-' || c == '_')) valid = false;
    }
    if (valid) s.themeId = id;
  }
#if WITH_THEME
  if (root["themeAliases"].is<JsonArrayConst>()) {
    s.themeAliasCount = 0;
    memset(s.themeAliases, 0, sizeof(s.themeAliases));
    for (JsonObjectConst alias : root["themeAliases"].as<JsonArrayConst>()) {
      if (s.themeAliasCount >= Settings::MaxThemeAliases) break;
      const char* id = alias["id"] | "";
      const char* name = alias["name"] | "";
      size_t idLen = strlen(id), nameLen = strlen(name);
      if (!idLen || idLen > 48 || !nameLen || nameLen > 96) continue;
      bool valid = true;
      for (size_t j = 0; j < idLen; ++j) {
        char c = id[j];
        if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '-' || c == '_')) valid = false;
      }
      for (size_t j = 0; j < nameLen; ++j) if ((uint8_t)name[j] < 32) valid = false;
      if (!valid) continue;
      bool duplicate = false;
      for (uint8_t j = 0; j < s.themeAliasCount; ++j) if (!strcmp(s.themeAliases[j].id, id)) duplicate = true;
      if (duplicate) continue;
      auto& dst = s.themeAliases[s.themeAliasCount++];
      memcpy(dst.id, id, idLen + 1);
      memcpy(dst.name, name, nameLen + 1);
    }
  }
#endif
  if (root["carouselSec"].is<int>())      s.carouselSec = constrain((int)root["carouselSec"], 5, 3600);
#if WITH_TICKER
  if (root["carouselTicker"].is<bool>())  s.carouselTicker = root["carouselTicker"];
#endif
#if WITH_USAGE
  if (root["carouselUsage"].is<bool>())   s.carouselUsage = root["carouselUsage"];
#endif
#if WITH_RADAR
  if (root["carouselRadar"].is<bool>())   s.carouselRadar = root["carouselRadar"];
#endif
#if WITH_HA
  if (root["carouselHa"].is<bool>())      s.carouselHa = root["carouselHa"];
#endif
#if WITH_THEME
  if (root["carouselTheme"].is<bool>())   s.carouselTheme = root["carouselTheme"];
#endif
#if WITH_THEME_PLAYLIST
  if (root["showflow"].is<JsonObjectConst>()) {
    JsonObjectConst flow = root["showflow"].as<JsonObjectConst>();
    if (flow["enabled"].is<bool>()) s.showflowEnabled = flow["enabled"];
    if (flow["random"].is<bool>()) s.showflowRandom = flow["random"];
    if (flow["steps"].is<JsonArrayConst>()) {
      s.showflowCount = 0;
      memset(s.showflow, 0, sizeof(s.showflow));
      for (JsonObjectConst item : flow["steps"].as<JsonArrayConst>()) {
        if (s.showflowCount >= Settings::MaxShowflowSteps) break;
        const char* type = item["type"] | "";
        auto& dst = s.showflow[s.showflowCount];
        if (!strcmp(type, "theme")) {
          const char* id = item["id"] | "";
          const size_t len = strlen(id);
          if (!len || len > 48) continue;
          bool valid = true;
          for (size_t c = 0; c < len; ++c) {
            const char ch = id[c];
            if (!((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') ||
                  (ch >= '0' && ch <= '9') || ch == '-' || ch == '_')) valid = false;
          }
          if (!valid) continue;
          memcpy(dst.themeId, id, len);
          dst.themeId[len] = '\0';
          dst.type = Settings::ShowflowTheme;
        } else if (!strcmp(type, "photo")) {
          dst.type = Settings::ShowflowPhoto;
          dst.randomPhoto = item["random"].is<bool>() ? item["random"].as<bool>() : true;
          const int photoId = item["photoId"] | -1;
          if (!dst.randomPhoto && (photoId < 0 || photoId >= 240)) continue;
          dst.photoId = dst.randomPhoto ? 0 : static_cast<uint8_t>(photoId);
        } else continue;
        const int seconds = item["seconds"] | 10;
        dst.seconds = static_cast<uint16_t>(constrain(seconds, 5, 3600));
        ++s.showflowCount;
      }
      if (!s.showflowCount) s.showflowEnabled = false;
    }
  }
#endif

  if (root["httpTimeout"].is<int>())        s.httpTimeout = constrain((int)root["httpTimeout"], 1000, 20000);
  if (root["brightness"].is<int>())         s.brightness = constrain((int)root["brightness"], 0, 100);
  if (root["backlightInverted"].is<bool>()) s.backlightInverted = root["backlightInverted"];
  if (root["rotation"].is<int>())           s.rotation = (uint8_t)(((int)root["rotation"]) & 3);

  // Feature slices: prefer the nested object; fall back to the top level so a
  // legacy flat config.json (or a legacy POST) still applies. The old shared
  // "pollSec" thus seeds both ticker and usage cadence on first upgrade.
#if WITH_TICKER
  JsonObjectConst t = root["ticker"].is<JsonObjectConst>() ? root["ticker"].as<JsonObjectConst>() : root;
  s.ticker.fromJson(t);
#endif
#if WITH_USAGE
  JsonObjectConst u = root["usage"].is<JsonObjectConst>() ? root["usage"].as<JsonObjectConst>() : root;
  s.usage.fromJson(u);
#endif
  // Radar has no legacy flat layout; only apply when its nested object is present.
#if WITH_RADAR
  if (root["radar"].is<JsonObjectConst>()) s.radar.fromJson(root["radar"].as<JsonObjectConst>());
#endif
  // HA has no legacy flat layout either; only apply when its object is present.
#if WITH_HA
  if (root["ha"].is<JsonObjectConst>()) s.ha.fromJson(root["ha"].as<JsonObjectConst>());
#endif
  if (root["clock"].is<JsonObjectConst>()) s.clock.fromJson(root["clock"].as<JsonObjectConst>());
  if (root["display"].is<JsonObjectConst>()) s.display.fromJson(root["display"].as<JsonObjectConst>());
  if (root["auth"].is<JsonObjectConst>()) s.auth.fromJson(root["auth"].as<JsonObjectConst>());
  if (root["button"].is<JsonObjectConst>()) s.button.fromJson(root["button"].as<JsonObjectConst>());
}
