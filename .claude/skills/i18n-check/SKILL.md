---
name: i18n-check
description: Find hardcoded user-visible strings and missing, unused or mismatched keys between assets/strings/en.json and assets/strings/pt-PT.json in Tonus. Use before committing UI or content changes, or when the user asks about translations.
---

# i18n check

## Steps
1. Load `assets/strings/en.json` and `assets/strings/pt-PT.json`.
2. Report:
   - keys in `en` missing from `pt-PT`, and the reverse
   - keys that are empty or identical to EN in PT-PT (check whether that's intended)
   - placeholder mismatches (`{bpm}`, `{n}` must match in both)
   - keys not referenced anywhere in `firmware/`, `sim/`, `core/` (unused)
3. Look for **hardcoded UI text** in `firmware/components/ui/` and `sim/`: string literals passed to
   LVGL label/text APIs or to toast/popup helpers.
4. PT-PT quality: European Portuguese, not Brazilian (e.g. "ecrã" not "tela", "guardar" not "salvar",
   "telemóvel" not "celular", "afinador" is fine in both). Flag Brazilian forms.
5. Check that every glyph used appears in the font's converted range (Latin-1).

## Output
Grouped lists with `file:line` for code findings, and a suggested PT-PT translation for each missing key.
