// SmallTV Stable: ESP8266 web configuration, recovery, themes, photos and night mode.
#pragma once
#include "Target.h"

#define FW_NAME "smalltv-stable"
#define FW_VERSION "1.0.0"
#define FW_VARIANT "esp8266-stable"
#define REPO_URL "https://github.com/jamedegnato-kostal/smalltv-ultra-mod"
#define REPO_OWNER "jamedegnato-kostal"
#define REPO_NAME "smalltv-ultra-mod"
#define UPDATE_ASSET "smalltv-stable-firmware.bin"
#define GH_API_HOST "api.github.com"

#define MAX_WIFI_NETS 4
#define DEFAULT_AP_SSID "SmallTV-Setup"
#define DEFAULT_AP_PASS ""
#define DEFAULT_HOSTNAME "smalltv"
#define DEFAULT_HTTP_TIMEOUT 8000

// NTP is started for either an active time-bound .stheme or the night-mode schedule.
#define NTP_SERVER1 "pool.ntp.org"
#define NTP_SERVER2 "time.nist.gov"
#define DEFAULT_TIMEZONE "CET-1CEST,M3.5.0,M10.5.0/3"
