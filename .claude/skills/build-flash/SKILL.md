---
name: build-flash
description: Build and run Tonus — host unit tests, the desktop SDL simulator, or the ESP-IDF firmware flashed to the M5Stack Tab5 with serial monitor. Use when the user asks to build, test, run, flash or monitor the app.
---

# Build and flash

Pick the target from the request (default: host tests, then the simulator).

## Host tests (`core/`)
```sh
cmake -S core -B build/core -DCMAKE_BUILD_TYPE=Debug
cmake --build build/core -j
ctest --test-dir build/core --output-on-failure
```

## Simulator (`sim/`, SDL2 + PortAudio)
```sh
cmake -S sim -B build/sim -DCMAKE_BUILD_TYPE=Debug
cmake --build build/sim -j
./build/sim/tonus_sim
```

## Firmware (Tab5)
Needs ESP-IDF v5.4+ exported (`. $IDF_PATH/export.sh`).
```sh
cd firmware
idf.py set-target esp32p4        # first time only
idf.py build
idf.py -p <PORT> flash monitor   # macOS port: /dev/cu.usbmodem*
```
- Find the port with `ls /dev/cu.usbmodem*`. If there's more than one, ask the user.
- If flashing fails to connect: hold the Tab5 boot/reset combination described in the M5Stack docs, then retry.
- Watch the monitor for `audio underrun` counters and task watchdog warnings, and report them.

## Report
State which targets were built, test results (pass/fail counts with failing test names), and
any warnings from the monitor. Don't claim a device run succeeded unless monitor output shows it.
