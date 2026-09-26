---
name: pixel-ui-review
description: Review a Tonus UI screen or component against docs/DESIGN_GUIDELINES.md — tokens, touch targets, integer scaling, i18n, colour-blind safety, motion. Use after creating or changing any LVGL screen, or when the user asks for a UI/UX review.
---

# Pixel UI review

Read `docs/DESIGN_GUIDELINES.md` first, then check the changed UI files (and a simulator screenshot
if one is available).

## Checks
| # | Check | How |
|---|---|---|
| 1 | Colours come only from tokens | grep for `lv_color_hex`, `0x` colour literals, `#` hex in UI code |
| 2 | Strings come only from i18n | no string literals passed to label/text setters |
| 3 | Touch targets ≥ 96 px; exercise primary ≥ 160 px tall; gaps ≥ 16 px | read sizes and layout |
| 4 | Integer scaling of art (×2/×3/×4/×6/×8), 4 px grid for spacing | image zoom and position values |
| 5 | No colour-only meaning | early/late/in-tune have icons or text as well |
| 6 | PT-PT fits | longest PT-PT string in each label fits without clipping |
| 7 | Hands-busy flow | count-in present, pause reachable, no swipe-only actions |
| 8 | Motion | ≤ 150 ms transitions; sprite fps 8–12; Reduce motion respected; practice animation driven by beat events |
| 9 | Button states | default / pressed / disabled / active all styled |
| 10 | Tone | encouraging copy, no "bad" or "fail" |

## Output
A table of findings with `file:line`, severity (must-fix / should-fix / nit), and the fix. Finish
with a pass/fail verdict.
