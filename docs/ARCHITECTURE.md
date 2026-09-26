# Tonus — Architecture

Related: [Product](PRODUCT.md) · [Design guidelines](DESIGN_GUIDELINES.md) · [Roadmap](ROADMAP.md)

## 1. Hardware: M5Stack Tab5

| Part | Detail | Used for |
|---|---|---|
| Main SoC | **ESP32-P4** (dual-core RISC-V, up to 400 MHz, + LP core), no radio | App, UI, audio engine, DSP |
| Radio co-processor | **ESP32-C6-MINI-1U**: Wi-Fi 6, **Bluetooth LE 5**, 802.15.4. Connected to the P4 over **SDIO** running **ESP-Hosted-MCU** | BLE (later phases), MAC for name seed |
| Memory | 16 MB flash, 32 MB PSRAM | Assets, sample bank, frame buffers |
| Display | 5" IPS 1280×720, MIPI-DSI; GT911 capacitive touch | UI |
| Audio out | **ES8388** codec → NS4150B 1 W speaker, and a 3.5 mm stereo headphone jack | Metronome, drums, synth |
| Audio in | **ES7210** 4-ch ADC with AEC front-end, dual microphone | Pitch/onset detection |
| USB | USB-A **host**, USB-C OTG | USB audio (UAC) devices, flashing |
| Other | microSD, BMI270 IMU, RX8130CE RTC, NP-F550 battery (kit), M5-Bus, Grove | Saves/backups, streak dates, battery |

Sources: [M5Stack Tab5 docs](https://docs.m5stack.com/en/core/Tab5) ·
[CNX Software review](https://www.cnx-software.com/2025/05/14/m5stack-tab5-review-part-1-unboxing-teardown-and-first-try-of-the-esp32-p4-and-esp32-c6-5-inch-iot-devkit/) ·
[espp/m5stack-tab5](https://components.espressif.com/components/espp/m5stack-tab5)

> Check against the schematic before relying on them: whether the ES7210 has an AEC
> **reference** channel wired to the ES8388 output, whether the headphone jack has insert
> detection, and the exact I²S/I²C pin map (take it from the BSP).

## 2. Bluetooth audio: what is possible

**The Tab5 has Bluetooth, but only Bluetooth LE**, through the ESP32-C6. With ESP-Hosted-MCU
the P4 runs the Bluetooth *host* stack (NimBLE, or Bluedroid in BLE mode) and sends HCI
packets over SDIO to the C6, which acts as the *controller* (radio + link layer).
Which Bluetooth features are available therefore depends on **what the C6 controller supports**,
and no software on the P4 can add features the controller lacks.

| Path | Needs | ESP32-C6 | Result |
|---|---|---|---|
| **A2DP** (almost every BT speaker and headphone) | Bluetooth Classic BR/EDR radio | ✗ BLE-only chip | **Not possible** |
| **LE Audio** (LC3, BAP unicast/broadcast) | LE isochronous channels (CIS/BIS) in the controller | ✗ Espressif closed the request as *Won't Do*; ESP-BLE-ISO is available only on newer chips (e.g. ESP32-S31) | **Not possible** |
| BLE GATT (MIDI, pedals, companion app) | BLE 5 | ✓ | Possible (phase 5) |

Sources: [esp-idf#12277 "Support LE Audio on ESP32-C6" (Won't Do)](https://github.com/espressif/esp-idf/issues/12277) ·
[ESP-IDF Bluetooth API for ESP32-C6](https://docs.espressif.com/projects/esp-idf/en/stable/esp32c6/api-reference/bluetooth/index.html) ·
[ESP-IDF LE Audio intro (ESP32-S31)](https://docs.espressif.com/projects/esp-idf/en/stable/esp32s31/api-guides/esp-ble-audio/ble-audio-introduction.html) ·
[ESP32 forum: BT Classic on C6](https://esp32.com/viewtopic.php?t=39603)

### How Tonus gets Bluetooth audio anyway
All output goes through one `AudioSink` interface. There are three kinds of sink:

```
                 ┌──────────────────────── ESP32-P4 ────────────────────────┐
 sequencer ─┐    │                                                          │
 synth ─────┼──▶ │ Mixer (48 kHz, float) ──▶ AudioSink ─┬─▶ I2SCodecSink ───┼─▶ ES8388 ─▶ speaker / 3.5 mm jack
 drums ─────┤    │                                      ├─▶ UsbUacSink ─────┼─▶ USB-A ─▶ USB BT transmitter dongle ))) BT speaker
 clicks ────┘    │                                      │                   │          (or any USB DAC / interface)
                 │                                      └─▶ BridgeSink ─────┼─▶ I²S on M5-Bus ─▶ ESP32 (Classic) A2DP ))) BT speaker
                 └──────────────────────────────────────────────────────────┘          (optional add-on, phase 5)
```

1. **`I2SCodecSink`** (v1 default): ES8388 via `esp_codec_dev`. Lag well under 10 ms.
2. **`UsbUacSink`**: ESP-IDF USB host UAC driver (`usb_host_uac`) on the USB-A port.
   A **USB Bluetooth transmitter dongle** that shows up to the host as a standard USB sound card
   (UAC 1.0/2.0; e.g. Creative BT-W5-class, prefer aptX Low Latency ≈ 40 ms) handles pairing and
   A2DP itself. **We check candidate dongles for UAC host compatibility on real hardware** and
   keep a list in this doc. The same sink supports USB DACs and guitar interfaces (with capture).
3. **`BridgeSink`** (optional): a small add-on board with an original ESP32 (which has BT Classic)
   running ESP-ADF's A2DP source, fed by I²S from the Tab5. Adds custom hardware and a second
   firmware. Expect about 150–250 ms lag.

### Latency compensation
- Each sink stores `outputLatencyMs`. The sequencer schedules on the **audible** timeline: scoring
  compares the detected onset time against `scheduledTime + outputLatency + inputLatency`.
- **Auto-calibrate:** play 8 clicks and detect them on the mic. The median round trip gives the combined
  latency. Manual ± slider as a fallback (a BT speaker far from the mic may need it).
- Bluetooth sinks show a notice: *"For scored timing exercises, use the headphone jack."*

## 3. Software stack

| Layer | Choice |
|---|---|
| Framework | **ESP-IDF v6.1** (pinned in `tools/idf-version.txt`; C++17), CMake, `idf.py` |
| BSP | Espressif BSP for Tab5 (`espressif/m5stack_tab5`; check the exact name and version in the Component Registry when scaffolding). `espp/m5stack-tab5` as a reference |
| UI | **LVGL 9** (MIPI-DSI, double-buffered in PSRAM, PPA/2D-DMA where available) |
| Audio codec | `esp_codec_dev` (ES8388 out, ES7210 in) |
| USB | `usb_host` + `usb_host_uac` (phase 5) |
| BLE | ESP-Hosted-MCU + NimBLE (phase 5) |
| Storage | NVS (settings), LittleFS (save + assets), FATFS on microSD (optional) |
| Host tests | CMake + Catch2 |
| Simulator | LVGL 9.6 SDL2 port on macOS/Linux; SDL audio for out and capture |

## 4. Runtime model

```
Core 1 (real-time)                          Core 0 (app)
┌───────────────────────────┐               ┌────────────────────────────┐
│ audio_task  prio 22       │  cmd queue    │ ui_task (LVGL)  prio 5     │
│  I²S DMA callback, 128 fr │ ◀──────────── │ app/game logic             │
│  sequencer → synth → mix  │               │ storage_task    prio 3     │
│  capture ring buffer ──┐  │  event queue  │                            │
├────────────────────────┼──┤ ────────────▶ │ (beat, onset, pitch,       │
│ analysis_task prio 20  ▼  │               │  exercise progress)        │
│  YIN/MPM pitch, onsets    │               └────────────────────────────┘
└───────────────────────────┘
```
- 48 kHz, **128-frame blocks** (≈ 2.7 ms). Output buffer depth kept to 2–3 blocks.
- **Lock-free SPSC queues** (fixed-size, preallocated) connect the cores. Messages are small POD structs.
- **Audio-thread rules:** no heap allocation, no mutexes, no logging, no file I/O, no LVGL calls.
  Samples are preloaded into PSRAM at boot or when a Jam style is chosen.
- Beat events carry a sample-clock timestamp, so the UI can animate the character on the beat.

## 5. Audio engine (`core/audio`)
- **Sequencer:** sample-accurate event scheduling from a musical clock (PPQ 960). Handles tempo ramps
  (Tempo Trainer), gap bars, swing and polyrhythm lanes.
- **Voices:** drum sampler (one-shot, per-track gain/pitch), 6-voice poly synth (2 osc + noise,
  state-variable filter, ADSR), mono bass, Karplus–Strong pluck, click generator.
- **Mixer:** per-bus gain (Click, Drums, Bass, Pad, UI), soft-clip limiter on the master.
- **Harmony:** Roman numeral → chord voicing in the chosen key; automatic bass line (root/fifth/approach patterns).

## 6. DSP / listening (`core/dsp`)
| Feature | Method | Notes |
|---|---|---|
| Pitch | YIN or MPM, 2048-sample window for guitar (low E 82 Hz), **4096 for bass** (low B 31 Hz, low E 41 Hz), 50% hop | Median filter over 3 frames; confidence threshold |
| Onsets | Spectral flux (512 FFT, hop 128) + adaptive threshold + 30 ms refractory | Guitar and bass plucks and strums |
| Timing score | Per hit `Δ = onset − expected` (latency-compensated); tightness = % within ±30 ms; mean shows rush/drag | Histogram for the result screen |
| Note check | Pitch → MIDI note ± 30 cents, held ≥ 150 ms | Fretboard, Play It Back |
| Chord check (*beta*) | Chroma vector + template match | Phase 4+, not required for v1 |
| Click leakage | Gating the onset detector around our own click times; ES7210 AEC reference if it is wired | Headphones recommended while scoring |

Accuracy is measured with WAV fixtures in `core/tests/fixtures/` (the `dsp-bench` skill).

## 7. Persistence
- **Settings:** NVS key/values.
- **Save file:** `/save/tonus_a.bin` and `/save/tonus_b.bin` on LittleFS, **written A/B** (write the other slot,
  fsync, then flip a pointer in NVS). Each file has a header `{magic, schemaVersion, length, crc32}` and a
  CBOR body (character, inventory, stats, economy, streak, quest state, achievement flags).
- **Migrations:** `migrate_vN_to_vN+1()` per schema bump, with a test for each.
- Optional export/backup to microSD.

## 8. Assets
- **Sprites:** layered PNGs per slot (see [Gamification §4](GAMIFICATION.md#4-shop-and-cosmetics)), drawn at
  art-px scale. `tools/assets` checks size, palette and transparency, then converts to LVGL image
  data (indexed colour where possible) plus a pack file.
- **Items:** `assets/items.json` → `{id, slot, class, tier, price, currency, unlockLevel, sprite, nameKey}`.
- **Strings:** `assets/strings/en.json`, `assets/strings/pt-PT.json` → generated key header, so a missing key fails the build.
- **Sounds:** 48 kHz/16-bit mono WAV, CC0 or synthesised; licences in `assets/sounds/LICENSES.md`.
- **Fonts:** pixel font converted with the LVGL font converter, including the Latin-1 range.

## 9. Repository layout

```
tonus/
├── core/                 # platform-free C++17, no ESP-IDF/LVGL includes (IDF component + CMake lib)
│   ├── app/              # boot sequence runner            ✓ Phase 0
│   ├── art/              # indexed bitmaps, wordmark, sprites ✓ Phase 0
│   ├── audio/            # chiptune renderer, test tones ✓; later: sequencer, voices, mixer
│   ├── dsp/              # later: pitch, onset, timing scoring
│   ├── game/             # later: stats, XP, economy, shop, quests, mood, name generator
│   ├── theory/           # later: notes, intervals, scales, chords, tunings
│   ├── i18n/             # string lookup (tables generated from assets/strings)
│   ├── device_hal.hpp    # the HAL interface
│   └── tests/            # Catch2
├── ui/                   # LVGL 9 screens + widgets, shared by firmware and sim (IDF component + CMake lib)
│   ├── theme.*           # colour tokens (only place with hex values), fonts
│   ├── widgets/          # pixel_art, pixel_box (buttons, windows), segment_bar
│   ├── screens/          # splash, home (placeholder), diagnostics
│   └── fonts/            # generated 1-bpp LVGL fonts + OFL licences
├── firmware/             # ESP-IDF project for Tab5 (main/: app_main, hal_tab5)
├── sim/                  # SDL2 desktop app (hal_sim, lv_conf.h, screenshot mode)
├── assets/strings/       # en.json, pt-PT.json
├── tools/                # i18n/gen_strings.py, fonts/convert.sh, idf-version.txt
├── scripts/              # setup, doctor, build, flash, monitor, clean, capture_log.py
├── docs/
└── .claude/skills/
```

**HAL:** `core/device_hal.hpp` (`DeviceHal`: info, heap, PCM/tone playback, mic level, settings)
is implemented by `firmware/main/hal_tab5.cpp` and `sim/hal_sim.cpp`. It gets split into
`AudioSink` / `AudioSource` / `Storage` / `Clock` when the real-time audio engine arrives (Phase 1).

## 10. Tab5 bring-up notes (learned in Phase 0)

- **Board revisions.** The BSP detects three variants from the touch controller: v1 ILI9881C + GT911,
  v2 ST7123, v3 ST7121 + ST712x touch. The development unit is **v3** (chip rev **v1.3**).
- **Panel power-cycle before the BSP.** The LCD and touch enable lines are on an I/O expander that keeps
  its state across a chip reset. If the LCD was left in reset, the BSP's probe finds no touch controller
  and **asserts ("Unsupported board version") in a boot loop**. `powerCyclePanel()` in `main.cpp` toggles
  both lines before `bsp_display_start_with_config()`.
- **Chip revision.** `CONFIG_ESP32P4_SELECTS_REV_LESS_V3=y` is required. Don't use `esp_audio_codec` ≥ 2.6,
  which needs rev 3.0.
- **Landscape.** The panel is 720×1280 portrait. `sw_rotate` + `CONFIG_LVGL_PORT_ENABLE_PPA=y` rotates by 90°
  on the PPA. The draw buffers (720×160, double) live in PSRAM, which leaves internal DMA RAM free for SDIO.
- **Resetting into the app.** An RTS/USB reset leaves the P4 in download mode (`boot:0x204`). Use esptool's
  `--after watchdog-reset`, as `scripts/capture_log.py` does, or press the reset button.
- **Audio.** The BSP runs I²S1 full-duplex, 48 kHz / 16-bit, mono slots. Speaker (ES8388) and mic (ES7210)
  are opened with the same format. The mic gain is 30 dB.
- **LVGL 9.6.** The generic flag APIs are deprecated (use `lv_obj_set_hidden/clickable/scrollable`),
  and the image-cache API is private. `CONFIG_LV_CACHE_DEF_SIZE=0` keeps the in-place pixel-art buffers safe.
