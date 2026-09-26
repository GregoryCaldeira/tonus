#!/usr/bin/env bash
# Remove build outputs. Usage: scripts/clean.sh [--all]   (--all also drops firmware/sdkconfig and managed components)
set -euo pipefail
. "$(dirname "$0")/env.sh"

rm -rf "$TONUS_ROOT/build" "$TONUS_ROOT/firmware/build"
if [ "${1:-}" = "--all" ]; then
  rm -rf "$TONUS_ROOT/firmware/sdkconfig" "$TONUS_ROOT/firmware/sdkconfig.old" "$TONUS_ROOT/firmware/managed_components"
fi
tonus_log "Clean."
