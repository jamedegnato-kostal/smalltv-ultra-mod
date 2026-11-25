#pragma once
#include "Platform.h"
#include "Settings.h"
namespace smalltv { namespace playback {
void begin(Settings& settings, bool displayReady);
void webBegin(WebServerClass& server);
void loop();
void stop();
bool active();
bool autoStartRequested();
bool advanceNow();
bool previousNow();
void redraw();
} }
