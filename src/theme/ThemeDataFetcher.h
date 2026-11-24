#pragma once
#include <stddef.h>
#include <stdint.h>

namespace smalltv {
class Engine;
struct Theme;

void themeDataFetcherBegin(size_t sourceCount);
void themeDataFetcherStop();
void themeDataFetcherLoop(const Theme& theme, Engine& engine, uint32_t now);
uint8_t themeDataFetcherSourceCount();
uint8_t themeDataFetcherReadyCount();
uint8_t themeDataFetcherStaleCount();
uint8_t themeDataFetcherFailureCount();
const char* themeDataFetcherError();
}  // namespace smalltv
