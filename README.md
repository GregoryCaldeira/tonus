# Tonus

**Your practice, their career.** A pocket practice studio for guitarists and bassists on the
[M5Stack Tab5](https://docs.m5stack.com/en/core/Tab5): tuner, pro metronome, timing and rhythm
scoring through the microphone, ear training, and a jam room with drums, bass and chord loops.

Every real minute you practise grows your pixel-art musician: XP, levels, Picks to spend on guitars,
hats and stages, and a career from the garage to the arena.

## Quick start (macOS)

```sh
sudo xcodebuild -license accept   # once, if you have never accepted it
make setup                        # Homebrew tools, ESP-IDF v6.1 (~/esp), font tools
make doctor                       # check everything is ready

make flash                        # build + flash the Tab5 over USB-C + serial monitor (Ctrl+] quits)
make sim                          # run the desktop simulator (1280×720 window)
make test                         # host unit tests
```

Pass `PORT=/dev/cu.usbmodemXXXX` to `make flash` or `make monitor` if more than one board is connected.
On the device: tap the splash to continue, or **long-press the logo for 1 s** to open Diagnostics.

## Docs
- [Product spec](docs/PRODUCT.md)
- [Gamification](docs/GAMIFICATION.md)
- [Design guidelines](docs/DESIGN_GUIDELINES.md)
- [Architecture](docs/ARCHITECTURE.md), including Tab5 bring-up notes
- [Roadmap](docs/ROADMAP.md)

Status: **Phase 0** (foundations + splash) running on the Tab5.
