// Target.h — this firmware targets the ESP8266 SmallTV / SmallTV Ultra.
// The loader env (src/loader.cpp, LOADER_BUILD) does not include this header.
#pragma once

#if !defined(SMALLTV_ESP8266)
#error "Define SMALLTV_ESP8266 (see platformio.ini)"
#endif

#define SMALLTV_FAMILY_ESP8266 1
