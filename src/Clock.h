#pragma once
#include <stddef.h>
#include <time.h>

namespace smalltv {
void clockBegin(const char* timezone);
void clockReconfigure(const char* timezone);
void clockService();
bool clockSynced();
bool clockNow(tm& out);
void clockFormat(char* buffer, size_t size);
}
