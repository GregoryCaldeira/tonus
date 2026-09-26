#!/usr/bin/env python3
"""Reset the Tab5 into the app and print its serial log for N seconds (non-interactive).

Usage: capture_log.py PORT [SECONDS]
Uses a watchdog reset: a plain RTS/USB reset leaves the ESP32-P4 in download mode.
Run inside the ESP-IDF environment (needs esptool and pyserial).
"""
import subprocess
import sys
import time

import serial


def main() -> int:
    port = sys.argv[1]
    seconds = float(sys.argv[2]) if len(sys.argv) > 2 else 12.0
    subprocess.run([sys.executable, "-m", "esptool", "--chip", "esp32p4", "-p", port,
                    "--after", "watchdog-reset", "read-mac"], capture_output=True, check=False)
    ser = serial.Serial()
    ser.port, ser.baudrate, ser.timeout = port, 115200, 0.2
    ser.dtr = ser.rts = False  # don't touch the reset/boot lines
    deadline = time.time() + 5
    while True:
        try:
            ser.open()
            break
        except serial.SerialException:
            if time.time() > deadline:
                raise
            time.sleep(0.1)
    end = time.time() + seconds
    while time.time() < end:
        try:
            data = ser.read(4096)
        except serial.SerialException:
            time.sleep(0.1)
            continue
        if data:
            sys.stdout.write(data.decode("utf-8", "replace"))
            sys.stdout.flush()
    return 0


if __name__ == "__main__":
    sys.exit(main())
