# Tonus — Gamification Design

Related: [Product](PRODUCT.md) · [Design guidelines](DESIGN_GUIDELINES.md)

**Core rule:** the digital musician grows only from **real practice**. The
game layer rewards effort and honesty. It must never punish, guilt-trip, or
make grinding worthwhile.

All numbers here are **starting values for tuning**. They live in one
data file (`core/game/balance.hpp` or `assets/balance.json`), never scattered in code.

---

## 1. Onboarding

| Step | Screen | Notes |
|---|---|---|
| 1 | Language | EN / PT-PT, big flag-free text buttons |
| 2 | Choose class | **Guitarist** or **Bassist**. Sets the starter instrument, tuner preset, pitch range, and some exercise content. Can be changed later in Settings (the character stays the same) |
| 3 | Body | Skin tone (6), eyes (4) |
| 4 | Hair | Style (8 incl. bald), colour (8) |
| 5 | Beard | None + 5 styles, colour follows hair (overridable) |
| 6 | Outfit | 4 starter tops, 3 bottoms, 3 shoes |
| 7 | Starter instrument | 3 per class, colour variants |
| 8 | **Stage name** | Generated; 3 free rerolls, then free editing (max 18 chars) |
| 9 | Tutorial | Tune one string (tuner) → play 8 beats with the metronome → **first level-up** with fanfare |

A **Randomise** die button on steps 3–7 generates a whole look at once.

### Stage name generator
- **Seed:** device MAC (read from the ESP32-C6 via ESP-Hosted) XOR first-boot RTC time, so every install is different and the result is reproducible for debugging.
- **Patterns** (weighted):
  - `First + Epithet` (40%): "Johnny Thunderfret"
  - `Title + Noun` (25%): "Lady Reverb", "Captain Fuzz"
  - `Title + Von + Noun` (10%): "Baron Von Riff"
  - `Adjective + Noun` (25%): "Velvet Distortion"
- The language chosen in step 1 selects the pool. Each pool has at least 40 entries per list at launch.

Example seed pools:

| List | EN | PT-PT |
|---|---|---|
| First | Johnny, Lola, Max, Ziggy, Ruby, Duke, Stella, Buddy, Nina, Rex | Zé, Tó, Rita, Quim, Lena, Chico, Bia, Nando, Mimi, Xico |
| Epithet | Thunderfret, Six-String, Overdrive, Slapjack, Blackstrap, Moonpick | Trovoada, das Cordas, Palhetada, Distorção, Riffão, do Baixo |
| Title | Lady, Captain, Baron, Doctor, Sir, Madame, Professor | Dona, Capitão, Barão, Doutor, Mestre, Madame |
| Noun | Reverb, Fuzz, Riff, Wah, Feedback, Groove, Chorus | Riff, Groove, Acorde, Pedal, Eco, Compasso |
| Adjective | Velvet, Electric, Rusty, Cosmic, Neon, Midnight | Elétrico, Cósmico, Néon, Veludo, Ferrugento |

> PT-PT gender agreement: tag adjectives/nouns with gender and only combine matching ones.

---

## 2. Stats (in-game skill tree)

Five stats, each levelled on its own:

| Stat | Icon idea | Fed by |
|---|---|---|
| **Timing** | metronome | Metronome scoring, Gap Click, Tempo Trainer |
| **Rhythm** | drum | Strumming patterns, call-and-response, Jam Room grooves |
| **Ear** | ear | Interval, chord-quality, scale-degree, Play It Back |
| **Harmony** | chord chip | Chord Loop jams, fretboard notes, chord exercises |
| **Technique** | hand | Fretboard Trainer, clean-pitch accuracy, speed drills |

### Level curves
- **Stat level:** `xpToNext(L) = round(100 · L^1.5)` → L1→2: 100, L5→6: 1 118, L10→11: 3 162.
- **Character level:** based on **total XP** across stats, `xpToNext(L) = round(250 · L^1.5)`.
  At about 250 XP/day (≈ 20 min of verified practice): level 2 on day 1, level 5 in about 2 weeks,
  level 10 in about 3½ months. Level cap 50 for v1.

### Unlocks (examples)

| Requirement | Unlocks |
|---|---|
| Timing 2 | Subdivisions: triplets, 16ths |
| Timing 3 | **Gap Click** |
| Timing 5 | Tempo Trainer beyond 200 BPM, polyrhythms |
| Rhythm 3 | Swing control in the drum machine |
| Ear 2 | Harmonic intervals |
| Ear 4 | 7th-chord qualities |
| Harmony 3 | Custom Roman-numeral chords (ii, iii, bVII) |
| Harmony 4 | 7th chords in the Chord Loop builder |
| Technique 3 | Fretboard Trainer: sharps/flats |
| Character 5 / 10 / 20 / 35 | New venue (see §5) |

---

## 3. Economy

### Currencies
- **XP:** progression only, can't be spent.
- **Picks** (palhetas), the soft currency. `+1 Pick per 10 XP earned`, plus quest and level-up bonuses.
- **Gold Records**, rare. Earned from achievements, every 5th character level, and weekly Gig ratings of 4★ or more. Spent only on legendary items.

### XP sources

| Source | XP formula | Notes |
|---|---|---|
| Mic-verified exercise | `10 XP/min × quality` where `quality ∈ [0.5, 1.5]` from the exercise score | Full reward |
| Unverified practice (Jam Room, plain metronome) | `4 XP/min` | **Daily cap 60 min** (240 XP) |
| Daily quest | 30–80 XP + 10–25 Picks | 3 per day |
| First exercise of the day | +25 XP | |
| Comeback bonus | +50 XP, +20 Picks | After 3 or more days away |
| Level-up | `+50 × newLevel` Picks | |

**Anti-grind:** repeating the same exercise more than 5 times a day earns 50% XP.
There's no anti-cheat beyond that: it's a personal device, and the mic-verified
bonus is the incentive to be honest.

### Price guide (Picks)

| Tier | Price | Unlock level |
|---|---|---|
| Common | 50–150 | 1–5 |
| Rare | 200–500 | 5–15 |
| Epic | 600–1 500 | 15–30 |
| Legendary | 3–10 Gold Records | 20+ |

---

## 4. Shop and cosmetics

Paper-doll slots (drawing order, back to front): `backdrop` → `companion` →
`amp` → `body` → `bottoms` → `shoes` → `top` → `beard` → `hair` → `hat` →
`instrument` → `accessory`.

**No real brand names or trademarked body shapes by name.** Use made-up names.

### Starter catalogue (v1)

| Id | Slot | Name (EN / PT-PT) | Tier | Class |
|---|---|---|---|---|
| `gtr_strato_red` | instrument | Stratoblaster Red / Stratoblaster Vermelha | starter | Guitar |
| `gtr_goldtop` | instrument | Paulsen Goldtop | rare | Guitar |
| `gtr_vwing` | instrument | V-Wing / Asa-V | epic | Guitar |
| `gtr_acoustic` | instrument | Campfire Dreadnought / Acústica Fogueira | common | Guitar |
| `gtr_lightning` | instrument | Lightning Axe / Machado Relâmpago | legendary | Guitar |
| `bas_jazzy` | instrument | Jazzy 4 | starter | Bass |
| `bas_thunder5` | instrument | Thunder 5-string / Trovão 5 Cordas | rare | Bass |
| `bas_fretless` | instrument | Fretless Ghost / Fantasma Fretless | epic | Bass |
| `hat_beanie` | hat | Beanie / Gorro | common | all |
| `hat_cowboy` | hat | Cowboy Hat / Chapéu de Cowboy | rare | all |
| `hat_crown` | hat | Rock Crown / Coroa do Rock | legendary | all |
| `top_band_tee` | top | Band Tee / T-shirt de Banda | starter | all |
| `top_leather` | top | Leather Jacket / Blusão de Cabedal | rare | all |
| `top_sequin` | top | Sequin Jacket / Casaco de Lantejoulas | epic | all |
| `amp_combo` | amp | Little Combo / Combo Pequeno | common | all |
| `amp_stack` | amp | Full Stack / Stack Completo | epic | all |
| `acc_shades` | accessory | Shades / Óculos Escuros | common | all |
| `cmp_cat` | companion | Amp Cat / Gato do Amp | rare | all |
| `cmp_parrot` | companion | Roadie Parrot / Papagaio Roadie | epic | all |

---

## 5. Career venues

| Venue | Character level | Backdrop | Title |
|---|---|---|---|
| Garage / Garagem | 1 | Garage with a parked car and a string of lights | Garage Hero |
| Bar | 5 | Small stage, neon "BAR" sign | Local Legend |
| Club / Clube | 10 | Purple club, disco ball | Club Headliner |
| Festival | 20 | Open-air stage, sunset | Festival Star |
| Arena | 35 | Lasers and a crowd | Arena God |

Unlocked venues can be picked as the Home backdrop (cosmetic).

### Weekly Gig
Every Sunday, or on the first launch after it, the week's practice is played as a
short animated **gig** at the current venue:
- **Crowd size** comes from total practice minutes.
- **Star rating (1–5)** comes from average exercise quality and practice days.
- 4★ or more pays a Gold Record. It's always positive in tone: a small crowd is still a show.

---

## 6. Retention

- **Daily quests** (3 per day, drawn from the pool, biased toward the weakest stat):
  "Play 50 beats at 100 BPM or faster with ≥ 80% tightness" · "Name 10 intervals" ·
  "Jam 5 min over a funk groove" · "Tune all 6 (4/5) strings" · "Survive 4 bars of Gap Click" ·
  "Find every E on the fretboard".
- **Streaks:** days in a row with ≥ 5 min of practice. **Streak Freeze** tokens (earned
  every 7-day streak, max 2 held) cover a missed day automatically.
- **Achievements** (examples): *First Blood* (first exercise) · *Metronome Monk* (1 000 on-time
  hits) · *Golden Ear* (50 intervals right in a row) · *Low Rider* (tune a low B) ·
  *Night Owl* (practise after midnight) · *Marathon* (60 min in a day).

### Mood (gentle, never punishing)
- **Energy** (0–100) goes up while practising and slowly down while idle (floor 20).
- **Mood** has 5 states: *hyped → happy → chill → bored → sleepy*. It only changes the idle
  animation and speech bubbles ("Miss jamming with you!").
- After 3 or more days away the musician is *sleepy*. Returning triggers the **Comeback bonus**
  and a wake-up animation.
- **Nothing is ever lost:** no XP decay, no item loss, no "leaving the band".

## 7. Live character behaviour
- **Idle:** blink, look around, tune the guitar, stretch (random, every 4–10 s).
- **During practice:** head-bob or foot-tap **locked to the metronome BPM**
  (the animation frame is driven by the beat event, not a timer).
- **Great hit streaks:** the character does a pose. Misses are never shown on the character.
- **Level-up:** jump + confetti + fanfare. **Purchase:** shows off the new item.
