---
name: dsp-bench
description: Benchmark Tonus pitch and onset detection against WAV fixtures in core/tests/fixtures — reports tuner cents error, note accuracy, onset timing error and CPU cost per block. Use after changing anything in core/dsp or when tuning detection parameters.
---

# DSP bench

## Fixtures
`core/tests/fixtures/` holds WAV files (48 kHz mono) with a sidecar `<name>.json` of ground truth:
- `pitch/*.wav` → `{ "f0_hz": 82.41, "instrument": "guitar" }` (include bass low E 41.2 Hz and low B 30.9 Hz)
- `onset/*.wav` → `{ "onsets_ms": [ ... ], "bpm": 100 }` (strums, single notes, palm-mutes, with and without metronome leakage)

## Steps
1. Build the host bench target: `cmake --build build/core --target dsp_bench`.
2. Run `./build/core/dsp_bench core/tests/fixtures --json build/dsp_report.json`.
3. Summarise:
   - **Pitch:** median / p95 absolute cents error per instrument; octave errors count
   - **Onsets:** precision, recall, median / p95 |Δ| ms (match window ±50 ms)
   - **Cost:** µs per 128-frame block (host; note that the P4 will be slower, so flag anything > 30% of the 2.7 ms budget after a 5× scale estimate)
4. Compare against the previous report if `build/dsp_report.prev.json` exists, and highlight regressions.

## Targets (from docs/ROADMAP.md)
Tuner ±2 cents median · onset median |Δ| < 10 ms · recall ≥ 95% on clean fixtures.
