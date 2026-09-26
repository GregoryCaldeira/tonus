# Tonus — Product Spec

> **Your practice, their career.**
> A pocket practice studio for guitarists and bassists, running on the M5Stack Tab5.

Related: [Gamification](GAMIFICATION.md) · [Design guidelines](DESIGN_GUIDELINES.md) · [Architecture](ARCHITECTURE.md) · [Roadmap](ROADMAP.md)

---

## 1. Vision

Tonus is a dedicated practice device for guitarists and bassists. Pick a class
(**Guitarist** or **Bassist**), design a pixel-art musician, and get a randomly
generated stage name ("Baron Von Riff", "Lady Reverb", "Zé Trovoada").

Every real minute you practise turns into **XP** and **Picks** (the in-game
currency), and exercises the microphone can verify pay the most. Your musician
levels up, buys gear, and moves from the garage to the arena.

Tonus listens to you through the microphone, keeps time, plays the drums and
chords you jam over, and trains your ear. **The musician grows only when you do.**

### Why a dedicated device?
- **No phone distractions.** No notifications or social feeds, so you just practise.
- **Always on the music stand.** The 5" screen can be read from the stool, and it
  runs on a battery.
- **Instant on.** No app to open and no tabs to find: tap Start and play.

## 2. Product pillars

| # | Pillar | What it means in practice |
|---|---|---|
| 1 | **Practice that feels like a game** | Every session pays XP, coins and progress you can see. The character reacts to your practice. |
| 2 | **Honest feedback from listening** | The mic checks pitch and timing. Rewards track real skill, not screen taps. |
| 3 | **Always something to jam to** | Drums, bass and chord loops are one tap away. Practice never has to be silent and boring. |
| 4 | **Usable with a guitar in your hands** | Big targets, count-ins, readable from 1.5 m, and minimal typing. |

## 3. Personas

**Ana, 16, beginner guitarist.** Learning from YouTube. She gets bored with
the metronome and can't tell whether she's in time. She needs a tuner,
simple strumming patterns, and small wins every day.

**Rui, 34, intermediate bassist.** Plays in a weekend cover band and has
little time. He wants focused 15-minute sessions: timing tightness, groove
over drum patterns, and learning the fretboard notes.

**Marta, 27, advanced guitarist.** Wants to improvise better. She needs
chord loops in any key, ear training for intervals and chord qualities, and
tempo pushing (Tempo Trainer).

## 4. Feature modules

| Module | v1 content | Mic-verified? | Stats fed |
|---|---|---|---|
| **Tuner** | Chromatic; presets for Guitar E standard, Drop D, DADGAD, Open G, Bass 4-string, Bass 5-string (low B); cents needle and strobe mode; A4 reference 432–446 Hz | yes | — (utility, tiny daily XP) |
| **Metronome Pro** | 20–300 BPM; time signatures 2/4–7/8 and 12/8; subdivisions (8ths, triplets, 16ths); accents per beat; tap tempo; **Tempo Trainer** (+N BPM every M bars up to a target); **Gap Click** (click plays X bars, then silent for Y bars; keep time); polyrhythm 3:2 and 4:3 | in scoring mode | Timing |
| **Timing & Rhythm** | Play along with the click; onset detection scores each hit early/late in ms; histogram and a "tightness" score; strumming pattern reader (↓↑ notation); rhythm call-and-response | yes | Timing, Rhythm |
| **Ear Training** | Intervals (asc/desc/harmonic); chord quality (maj, min, 7, maj7, m7, dim, sus2/4); scale degrees in a key; **Play It Back** (hear a note, play it; the mic checks the pitch) | partly | Ear |
| **Fretboard Trainer** | "Play the A on string 5": the mic verifies. Covers naturals, sharps/flats, and octave shapes | yes | Technique, Harmony |
| **Jam Room** | **Drum machine**: 16 steps × 8 tracks (kick, snare, closed hat, open hat, tom, crash, ride, clap), swing, style presets (rock, funk, blues shuffle, bossa, metal, reggae, pop). **Chord Loop builder**: key + Roman-numeral chips (I–V–vi–IV…), auto bass line, synth pad/keys voices. **Scale hint**: the fitting scale drawn on a fretboard | no (practice-time XP) | Harmony, Rhythm |
| **Daily Session** | "10 / 20 / 30 min" playlist built automatically from your weakest stat, so there's no decision fatigue | mixed | all |

### Sound engine
Built in-house and small:
- a sample-accurate **sequencer** (drives metronome, drums and chord loops)
- a **drum sampler** (one-shot 48 kHz / 16-bit samples)
- a **6-voice subtractive/wavetable synth** (pads, keys)
- a **mono bass** voice
- **Karplus–Strong** plucked tones for reference notes (ear training, tuner reference)
- a click set: wood, beep, cowbell, hi-hat, 8-bit
- short 8-bit **UI sounds** (can be muted separately)

All samples must be **CC0** or synthesised by us, and their licences are
recorded in `assets/sounds/LICENSES.md`.

### Audio outputs
The speaker, the 3.5 mm jack, and **USB audio** (USB-A), which also covers
**USB Bluetooth transmitter dongles** for Bluetooth speakers and headphones.
The Tab5's own radio can't stream Bluetooth audio; see
[Architecture §2](ARCHITECTURE.md#2-bluetooth-audio-what-is-possible).
Each output has a lag-offset setting and auto-calibration.

## 5. Core user flows

1. **First launch** (about 90 s): language → class → character creator → name
   generator → short tuner tutorial → first metronome exercise → first
   level-up. Details in [Gamification §1](GAMIFICATION.md#1-onboarding).
2. **Daily practice:** Home → "Today's Session 20 min" → exercises run
   back-to-back with count-ins → reward screen (XP fill, Picks drop, stat
   gains) → character reacts.
3. **Free jam:** Jam → pick a style preset → pick a chord loop → play for as long
   as you like (practice-time XP accrues, capped per day).
4. **Spend:** Shop → browse by slot → buy → equip → the character shows off the new
   gear on Home.

## 6. Settings
Language (EN / PT-PT) · Theme (Stage / Bedroom) · Audio output + lag offset ·
Mic input gain + calibration · Master / Music / Click / UI volumes · Handedness
(left-handed fretboard) · A4 reference · Reset save (with confirmation).

## 7. Out of scope for v1
- Cloud accounts, sync, leaderboards, social sharing
- Phone companion app
- In-app purchases of any kind (all items are earned)
- Direct Bluetooth audio from the Tab5 radio (the hardware can't do it)
- Tablature and sheet-music display, song library
- Recording and looping the user's own audio (candidate for later)

## 8. Success metrics (on-device, local only)
- Onboarding completion rate (reached first level-up)
- Practice days per week, median session length
- Share of XP from mic-verified exercises (target > 50%)
- Improvement in timing tightness for the same exercise over 4 weeks
