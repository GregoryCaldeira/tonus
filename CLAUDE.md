# Tonus

Practice device for guitarists and bassists on the **M5Stack Tab5** (ESP32-P4 + ESP32-C6), with a
pixel-art musician that grows from real practice.

Read first: [docs/PRODUCT.md](docs/PRODUCT.md) · [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md) ·
[docs/DESIGN_GUIDELINES.md](docs/DESIGN_GUIDELINES.md) · [docs/GAMIFICATION.md](docs/GAMIFICATION.md) ·
[docs/ROADMAP.md](docs/ROADMAP.md)

## Hard rules

### Audio thread (`audio_task`, `analysis_task`, anything called from the I²S callback)
- No heap allocation, no mutexes/semaphores, no `ESP_LOG*`/`printf`, no file I/O, no LVGL calls.
- Talk to other tasks **only** through the preallocated lock-free SPSC queues.
- Samples and tables are loaded before playback starts.

### Layering
- `core/` is platform-free C++17: **no ESP-IDF, FreeRTOS, or LVGL includes**. Hardware goes behind the
  HAL interfaces (`AudioSink`, `AudioSource`, `Storage`, `Clock`, `DeviceInfo`).
- `firmware/components/hal_tab5` and `sim/` are the only HAL implementations.

### UI
- Every colour comes from **theme tokens**. No hex literals in UI code.
- Every user-visible string comes from **i18n keys** with both `en` and `pt-PT` entries. Design for PT-PT length.
- Touch targets ≥ 96 px; exercise primary action ≥ 160 px tall. Pixel art uses integer scaling only.
- Never show information by colour alone (early/late use arrows as well).

### Game
- All balance numbers (XP rates, prices, curves, caps) live in the single balance file, never inline.
- Game math (XP, levels, economy, name generator) has unit tests.
- Nothing the user owns can be lost: no XP decay, no item loss.
- No real brand or trademark names for gear.

### Persistence
- Any change to the save schema bumps `schemaVersion` and adds a tested migration.

## Hardware facts to remember
- The ESP32-C6 is **BLE-only**. It has no A2DP (Classic) and no LE Audio (no ISO). Bluetooth audio goes through
  a USB BT transmitter dongle (`UsbUacSink`) or the optional ESP32 bridge. See ARCHITECTURE §2.
- Bass pitch detection needs a 4096-sample window (low B ≈ 31 Hz).

## Project skills
`/new-exercise` · `/new-shop-item` · `/pixel-ui-review` · `/i18n-check` · `/build-flash` · `/dsp-bench`
