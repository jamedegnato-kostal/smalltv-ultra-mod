#pragma once
#include "../Platform.h"

// Headless .stheme package management: upload, validate, list and delete.
// There is deliberately no theme activation or display renderer.
void themeWebBegin(WebServerClass& server);
