#!/bin/bash

# AI ALERT >:(

set -euo pipefail

# This script lives in extras/tests/, so the project root is two levels up.
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
cd "$ROOT"

# Compile the test together with the sources it depends on.
# -Isrc lets the test find debug.h / serialization.h (and their nested includes).
gcc -Wall -Wextra -Isrc \
  extras/tests/pack_unpack.c \
  src/serialization.c \
  src/debug.c \
  -o /tmp/btc_seeder_test

/tmp/btc_seeder_test
rm -f /tmp/btc_seeder_test
