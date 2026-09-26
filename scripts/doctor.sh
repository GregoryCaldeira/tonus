#!/usr/bin/env bash
# Check that the Tonus toolchain is ready. Exits non-zero if something required is missing.
set -uo pipefail
. "$(dirname "$0")/env.sh"

fail=0
ok()   { printf '  \033[32m✓\033[0m %s\n' "$*"; }
bad()  { printf '  \033[31m✗\033[0m %s\n' "$*"; fail=1; }
info() { printf '  \033[33m•\033[0m %s\n' "$*"; }

check_cmd() {
  if command -v "$1" >/dev/null 2>&1; then ok "$1 ($($1 --version 2>&1 | head -1))"; else bad "$1 missing, run: make setup"; fi
}

echo "Host tools"
check_cmd cmake
check_cmd ninja
check_cmd python3
if pkg-config --exists sdl2 2>/dev/null || [ -f /opt/homebrew/include/SDL2/SDL.h ] || [ -f /usr/local/include/SDL2/SDL.h ]; then
  ok "SDL2"
else
  bad "SDL2 missing (needed by the simulator), run: make setup"
fi
if command -v node >/dev/null && [ -d "$TONUS_ROOT/tools/fonts/node_modules/lv_font_conv" ]; then
  ok "lv_font_conv (font regeneration)"
else
  info "lv_font_conv not installed (only needed to regenerate fonts)"
fi

echo "ESP-IDF"
if [ -f "$TONUS_IDF_DIR/export.sh" ]; then
  got="$(git -C "$TONUS_IDF_DIR" describe --tags 2>/dev/null || echo unknown)"
  if [ "$got" = "$TONUS_IDF_VERSION" ]; then ok "ESP-IDF $got at $TONUS_IDF_DIR"; else bad "ESP-IDF is $got, expected $TONUS_IDF_VERSION"; fi
  if [ -d "$HOME/.espressif/tools/riscv32-esp-elf" ]; then ok "RISC-V toolchain installed"; else bad "RISC-V toolchain missing, run: make setup"; fi
else
  bad "ESP-IDF $TONUS_IDF_VERSION not installed, run: make setup"
fi

echo "Device"
ports=$(ls /dev/cu.usbmodem* /dev/ttyACM* 2>/dev/null || true)
if [ -n "$ports" ]; then ok "Serial port(s): $ports"; else info "No Tab5 connected (plug in USB-C to flash)"; fi

exit $fail
