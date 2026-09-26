# Tonus — Roadmap

Related: [Product](PRODUCT.md) · [Architecture](ARCHITECTURE.md)

Each phase ends with something that runs on a real Tab5. Acceptance criteria must be met before
moving on.

## Phase 0: Foundations
- Repo skeleton per [Architecture §9](ARCHITECTURE.md#9-repository-layout); CMake host build of `core/` with Catch2.
- LVGL SDL simulator showing a themed test screen (tokens, pixel font with PT-PT accents, pixel button).
- Tab5 bring-up: display + touch, ES8388 sine out on speaker and jack, ES7210 capture level meter.
- **Done when:** `sim` and firmware both show the same test screen; a 1 kHz tone plays; the mic meter moves; host tests pass.

## Phase 1: MVP "First Gig"
- Tuner (guitar + bass presets, cents needle).
- Metronome Pro (BPM, signatures, subdivisions, accents, tap tempo, Tempo Trainer).
- Character creator, stage-name generator (EN + PT-PT), Home screen with an idle character.
- Stats/XP/levels (practice-time XP), save file with A/B writes.
- **Done when:** a new user can create a musician, tune, practise with the metronome, level up, reboot, and keep progress. Tuner is accurate to ±2 cents on test fixtures (82–330 Hz and 31–98 Hz).

## Phase 2: Listen & score
- Onset detection and timing scoring; result/reward screen; latency auto-calibration.
- Gap Click, strumming patterns, call-and-response.
- Picks, Shop v1 (starter catalogue), equip on the paper doll.
- Daily Session and daily quests, streaks.
- **Done when:** timing error is under ±10 ms (median) on fixtures; a full 20-minute Daily Session pays XP/Picks; items can be bought and equipped.

## Phase 3: Jam Room
- Drum machine (16 × 8, swing, 7 style presets), Chord Loop builder, auto bass, synth pad.
- Scale hint on a fretboard.
- **Done when:** a 4-chord loop with drums + bass + pad plays without dropouts for 30 minutes (monitor audio underruns = 0).

## Phase 4: Ear & career
- Ear training (intervals, chord quality, scale degrees, Play It Back), Fretboard Trainer.
- Skill-tree unlocks, venues, achievements, weekly Gig, mood system, Bedroom theme.
- **Done when:** every stat has at least 2 exercises feeding it; venue progression reaches Arena in a test save.

## Phase 5: Connectivity
- `UsbUacSink` (+ capture): USB DACs, **USB Bluetooth transmitter dongles**, USB guitar interfaces. Publish a list of tested dongles.
- BLE (ESP-Hosted + NimBLE): BLE foot pedal (HID page-turner) and BLE-MIDI.
- Optional ESP32 A2DP bridge add-on (`BridgeSink`).
- **Done when:** at least one USB BT dongle plays to BT headphones with calibrated latency; the pedal starts/stops exercises.

## Later / ideas
- Phrase looper (record and loop the user)
- Song-section practice with a speed trainer
- Phone companion app for backup and sharing a character card
- Chord recognition out of beta
