#!/usr/bin/env bash
# Open the serial monitor for the Tab5 (Ctrl+] to quit).  Usage: scripts/monitor.sh [-p PORT]
set -euo pipefail
. "$(dirname "$0")/env.sh"

port=""
[ "${1:-}" = "-p" ] && port="${2:-}"
tonus_idf_env
[ -n "$port" ] || port="$(tonus_find_port)"
cd "$TONUS_ROOT/firmware"
idf.py -p "$port" monitor
