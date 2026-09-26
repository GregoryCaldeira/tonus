#!/usr/bin/env bash
# Shared environment for Tonus scripts. Source it, don't run it.
#   source scripts/env.sh          # also exports ESP-IDF into the current shell

TONUS_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
TONUS_IDF_VERSION="$(tr -d '[:space:]' < "$TONUS_ROOT/tools/idf-version.txt")"
TONUS_IDF_DIR="${TONUS_IDF_DIR:-$HOME/esp/esp-idf-$TONUS_IDF_VERSION}"
export TONUS_ROOT TONUS_IDF_VERSION TONUS_IDF_DIR

tonus_log()  { printf '\033[35m[tonus]\033[0m %s\n' "$*"; }
tonus_warn() { printf '\033[33m[tonus]\033[0m %s\n' "$*" >&2; }
tonus_die()  { printf '\033[31m[tonus]\033[0m %s\n' "$*" >&2; exit 1; }

# Load ESP-IDF (idempotent). Prefers the pinned install, falls back to $IDF_PATH.
tonus_idf_env() {
  if command -v idf.py >/dev/null 2>&1 && [ -n "${IDF_PATH:-}" ]; then
    return 0
  fi
  local export_sh="$TONUS_IDF_DIR/export.sh"
  if [ ! -f "$export_sh" ] && [ -n "${IDF_PATH:-}" ]; then
    export_sh="$IDF_PATH/export.sh"
  fi
  [ -f "$export_sh" ] || tonus_die "ESP-IDF not found at $TONUS_IDF_DIR. Run: make setup"
  # shellcheck disable=SC1090
  . "$export_sh" >/dev/null || tonus_die "Failed to export ESP-IDF from $export_sh"
}

# Host builds need cmake + ninja; fall back to the copies ESP-IDF installs if Homebrew's are missing.
tonus_host_tools() {
  if ! command -v cmake >/dev/null 2>&1 || ! command -v ninja >/dev/null 2>&1; then
    tonus_idf_env
  fi
  /usr/bin/xcrun clang --version >/dev/null 2>&1 ||
    tonus_die "The host C++ compiler is not usable. Run 'sudo xcodebuild -license accept' (or 'xcode-select --install')."
}

# Print the Tab5 serial port, or fail. Honours $TONUS_PORT.
tonus_find_port() {
  if [ -n "${TONUS_PORT:-}" ]; then echo "$TONUS_PORT"; return 0; fi
  local ports=()
  for p in /dev/cu.usbmodem* /dev/ttyACM*; do [ -e "$p" ] && ports+=("$p"); done
  case "${#ports[@]}" in
    0) tonus_die "No Tab5 found. Plug it in over USB-C, or pass -p PORT." ;;
    1) echo "${ports[0]}" ;;
    *) tonus_die "Several serial ports found (${ports[*]}). Pass -p PORT or set TONUS_PORT." ;;
  esac
}
