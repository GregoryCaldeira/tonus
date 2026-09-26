---
name: build-flash
description: Build and run Tonus — host unit tests, the desktop SDL simulator, or the ESP-IDF firmware flashed to the M5Stack Tab5 with serial monitor. Use when the user asks to build, test, run, flash or monitor the app.
---

# Build and flash

Everything goes through the root `Makefile`, which calls the `scripts/`. Pick the target from the
request (default: `make test`, then `make sim`).

| Goal | Command |
|---|---|
| Check the toolchain and device | `make doctor` |
| Host unit tests (`core/`) | `make test` |
| Desktop simulator | `make sim` |
| Firmware build only | `make build` |
| Flash + interactive monitor | `make flash` (`PORT=/dev/cu.usbmodemXXXX` if needed) |

## Non-interactive use (from Claude)
`idf.py monitor` needs a TTY, so don't use `make flash` / `make monitor` from a tool call. Instead:
```sh
. ~/esp/esp-idf-v6.1/export.sh
cd firmware && idf.py build && idf.py -p <PORT> flash
python ../scripts/capture_log.py <PORT> 12     # watchdog-resets into the app and prints 12 s of log
```
- Find the port with `ls /dev/cu.usbmodem*`. If there's more than one, ask the user.
- Never use an RTS/USB reset to start the app: it leaves the ESP32-P4 in download mode (`boot:0x204`).
- Decode a crash with `riscv32-esp-elf-addr2line -pfiaC -e build/tonus.elf <addrs>`.

## Simulator screenshots (checking layouts without the device)
```sh
./build/sim/tonus_sim --screenshot /tmp/splash.bmp --at 3500
./build/sim/tonus_sim --screen diag --lang pt --theme bedroom --screenshot /tmp/diag.bmp --at 800 --settings /tmp/sim.txt
```
Then Read the BMP (convert it to PNG with Pillow if needed) and compare it against docs/DESIGN_GUIDELINES.md.

## Report
State which targets were built, the test results (pass/fail counts, with failing test names), and any
errors, panics or warnings from the log. Don't claim a device run succeeded unless the log shows
`tonus: Boot finished`.
