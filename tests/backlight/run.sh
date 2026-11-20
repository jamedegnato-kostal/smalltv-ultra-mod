#!/bin/sh
set -eu
cd "$(dirname "$0")/../.."
c++ -std=c++11 -Wall -Wextra -Werror -g -Isrc tests/backlight/test_backlight.cpp -o /tmp/smalltv-backlight-tests
/tmp/smalltv-backlight-tests
c++ -std=c++11 -Wall -Wextra -Werror -g tests/backlight/test_wiring.cpp -o /tmp/smalltv-backlight-wiring
/tmp/smalltv-backlight-wiring
