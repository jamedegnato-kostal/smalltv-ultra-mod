// Minimal local configuration server, no authentication.
#pragma once
#include "Settings.h"
void webPortalBegin(Settings& settings);
void webPortalLoop();
bool webPortalRebootDue();
