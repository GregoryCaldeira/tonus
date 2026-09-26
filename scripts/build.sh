#!/usr/bin/env bash
# Build Tonus.  Usage: scripts/build.sh [fw|sim|test|all]   (default: all)
set -euo pipefail
. "$(dirname "$0")/env.sh"

target="${1:-all}"
jobs="$(sysctl -n hw.ncpu 2>/dev/null || nproc)"

build_test() {
  tonus_log "Host tests (core/)…"
  tonus_host_tools
  cmake -S "$TONUS_ROOT/core" -B "$TONUS_ROOT/build/core" -G Ninja -DCMAKE_BUILD_TYPE=Debug -DTONUS_BUILD_TESTS=ON
  cmake --build "$TONUS_ROOT/build/core" -j "$jobs"
  ctest --test-dir "$TONUS_ROOT/build/core" --output-on-failure
}

build_sim() {
  tonus_log "Desktop simulator (sim/)…"
  tonus_host_tools
  cmake -S "$TONUS_ROOT/sim" -B "$TONUS_ROOT/build/sim" -G Ninja -DCMAKE_BUILD_TYPE=Debug
  cmake --build "$TONUS_ROOT/build/sim" -j "$jobs"
  tonus_log "Built build/sim/tonus_sim"
}

build_fw() {
  tonus_log "Firmware for Tab5 (firmware/)…"
  tonus_idf_env
  cd "$TONUS_ROOT/firmware"
  [ -f sdkconfig ] || idf.py set-target esp32p4
  idf.py build
}

case "$target" in
  test) build_test ;;
  sim)  build_sim ;;
  fw)   build_fw ;;
  all)  build_test; build_sim; build_fw ;;
  *)    tonus_die "Unknown target '$target' (use fw, sim, test or all)" ;;
esac
