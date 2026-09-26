#!/usr/bin/env bash
# Install everything needed to build Tonus on macOS: host tools, ESP-IDF (pinned), font tools.
# Safe to run again; finished steps are skipped.
set -euo pipefail
. "$(dirname "$0")/env.sh"

if [ "$(uname -s)" = "Darwin" ]; then
  command -v brew >/dev/null || tonus_die "Homebrew is required: https://brew.sh"
  if ! /usr/bin/xcrun clang --version >/dev/null 2>&1; then
    tonus_die "Xcode command-line tools are not usable. Run 'xcode-select --install', or 'sudo xcodebuild -license accept' if you haven't accepted the licence yet."
  fi
  tonus_log "Installing host tools with Homebrew…"
  brew install cmake ninja dfu-util ccache sdl2 >/dev/null
else
  tonus_warn "Not macOS: install cmake, ninja, dfu-util, ccache and SDL2 with your package manager."
fi

if [ ! -f "$TONUS_IDF_DIR/export.sh" ]; then
  tonus_log "Cloning ESP-IDF $TONUS_IDF_VERSION into $TONUS_IDF_DIR (this takes a while)…"
  mkdir -p "$(dirname "$TONUS_IDF_DIR")"
  git clone --branch "$TONUS_IDF_VERSION" --depth 1 --recursive --shallow-submodules \
    https://github.com/espressif/esp-idf.git "$TONUS_IDF_DIR"
else
  tonus_log "ESP-IDF $TONUS_IDF_VERSION already present."
fi

tonus_log "Installing ESP-IDF tools for esp32p4…"
"$TONUS_IDF_DIR/install.sh" esp32p4
# IDF v6 treats cmake/ninja as optional tools on macOS; install them so idf.py works without Homebrew.
(. "$TONUS_IDF_DIR/export.sh" >/dev/null && python "$TONUS_IDF_DIR/tools/idf_tools.py" install cmake ninja)

if command -v npm >/dev/null; then
  tonus_log "Installing font tools…"
  (cd "$TONUS_ROOT/tools/fonts" && npm ci --silent)
else
  tonus_warn "npm not found: font regeneration (tools/fonts) will be unavailable. Prebuilt fonts still work."
fi

tonus_log "Done. Next: make doctor, then make test / make sim / make flash"
