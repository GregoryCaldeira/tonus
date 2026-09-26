#!/usr/bin/env bash
# Build and flash the firmware to the Tab5.
# Usage: scripts/flash.sh [-p PORT] [--monitor]
set -euo pipefail
. "$(dirname "$0")/env.sh"

port=""
monitor=0
while [ $# -gt 0 ]; do
  case "$1" in
    -p|--port) port="$2"; shift 2 ;;
    --monitor|-m) monitor=1; shift ;;
    *) tonus_die "Unknown option $1" ;;
  esac
done

tonus_idf_env
[ -n "$port" ] || port="$(tonus_find_port)"
cd "$TONUS_ROOT/firmware"
[ -f sdkconfig ] || idf.py set-target esp32p4

tonus_log "Flashing to $port…"
if [ "$monitor" = 1 ]; then
  idf.py -p "$port" flash monitor
else
  idf.py -p "$port" flash
  tonus_log "Flashed. Watch the log with: make monitor"
fi
