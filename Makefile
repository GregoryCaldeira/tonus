# Tonus — convenience targets. The real work lives in scripts/.
# PORT=/dev/cu.usbmodemXXXX can be passed to flash/monitor.

PORT_ARG := $(if $(PORT),-p $(PORT),)

.PHONY: help setup doctor test sim run-sim build flash monitor fonts strings clean distclean

help:
	@echo "make setup      install toolchain (Homebrew tools, ESP-IDF, font tools)"
	@echo "make doctor     check the toolchain and connected device"
	@echo "make test       build and run host unit tests"
	@echo "make sim        build and run the desktop simulator"
	@echo "make build      build the Tab5 firmware"
	@echo "make flash      build, flash and open the serial monitor  (PORT=...)"
	@echo "make monitor    serial monitor only                       (PORT=...)"
	@echo "make fonts      regenerate LVGL pixel fonts"
	@echo "make clean      remove build outputs"

setup:
	@scripts/setup.sh

doctor:
	@scripts/doctor.sh

test:
	@scripts/build.sh test

sim:
	@scripts/build.sh sim
	@./build/sim/tonus_sim

build:
	@scripts/build.sh fw

flash:
	@scripts/flash.sh $(PORT_ARG) --monitor

monitor:
	@scripts/monitor.sh $(PORT_ARG)

fonts:
	@tools/fonts/convert.sh

clean:
	@scripts/clean.sh

distclean:
	@scripts/clean.sh --all
