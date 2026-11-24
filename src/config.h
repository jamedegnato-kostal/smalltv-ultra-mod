// SmallTV Ultra Clean: web configuration, recovery and display status.
#pragma once
#include "Target.h"

#define FW_NAME "smalltv-clean"
#define FW_VERSION "0.3.0"
#define FW_VARIANT "esp8266-web-display-stheme"
#define REPO_URL "https://github.com/jamedegnato-kostal/smalltv-ultra-mod"
#define REPO_OWNER "jamedegnato-kostal"
#define REPO_NAME "smalltv-ultra-mod"
#define UPDATE_ASSET "smalltv-clean-firmware.bin"
#define GH_API_HOST "api.github.com"

#define MAX_WIFI_NETS 4
#define DEFAULT_AP_SSID "SmallTV-Setup"
#define DEFAULT_AP_PASS ""
#define DEFAULT_HOSTNAME "smalltv"
#define DEFAULT_HTTP_TIMEOUT 8000

// NTP is started only when the selected .stheme uses time-bound layers.
#define NTP_SERVER1 "pool.ntp.org"
#define NTP_SERVER2 "time.nist.gov"
#define DEFAULT_TIMEZONE "CET-1CEST,M3.5.0,M10.5.0/3"
