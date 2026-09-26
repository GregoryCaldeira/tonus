---
name: new-exercise
description: Scaffold a new Tonus practice exercise end to end — definition, scoring, stat/XP mapping, UI screen, EN + PT-PT strings and unit tests. Use when the user asks to add or create an exercise, drill or training mode.
---

# New exercise

Ask for (or infer) the following: **name**, **module** (Timing, Rhythm, Ear, Fretboard, Jam), **class**
(guitar / bass / both), **mic-verified?**, **stats fed** (1–2 of Timing, Rhythm, Ear, Harmony,
Technique), **unlock requirement**.

## Steps
1. **Definition** in `core/game/exercises/<id>.hpp/.cpp`: id, module, stats + weights, unlock
   requirement, parameters (BPM range, bars, etc.) and whether it is mic-verified.
2. **Scoring** in `core/`: a pure function from the analysis events to `quality ∈ [0.5, 1.5]`
   and a result summary. No ESP-IDF includes.
3. **XP**: use the balance file (`10 XP/min × quality` verified, `4 XP/min` unverified with the daily cap).
   Never hardcode rates.
4. **UI screen** in `firmware/components/ui/`: follow the "Exercise run" wireframe in
   `docs/DESIGN_GUIDELINES.md` §12. Hide the rail, add the 4-beat count-in, a primary action ≥ 160 px,
   and colours from tokens only.
5. **Strings**: add keys to both `assets/strings/en.json` and `assets/strings/pt-PT.json`
   (title, instructions, result phrases). Encouraging tone, never "bad".
6. **Tests** in `core/tests/`: scoring edge cases (perfect, all early, all late, silence), XP amount,
   and an unlock check. For mic exercises, add a WAV fixture if one is available.
7. **Registration**: add it to the exercise registry and to the Daily Session pool.

## Done checklist
- [ ] Host tests pass
- [ ] `/i18n-check` is clean
- [ ] `/pixel-ui-review` on the new screen passes
- [ ] Audio-thread rules from CLAUDE.md respected
