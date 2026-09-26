---
name: new-shop-item
description: Add a cosmetic item (instrument, hat, top, amp, backdrop, companion, accessory) to the Tonus shop — validates sprite size and palette, registers it in items.json with price and unlock level, and adds EN + PT-PT names. Use when the user wants a new guitar, bass, outfit, hat or other gear for the virtual musician.
---

# New shop item

## Inputs
`id` (snake_case with slot prefix, e.g. `hat_beanie`), slot, class (`guitar` / `bass` / `all`),
tier (common / rare / epic / legendary), sprite PNG path, EN + PT-PT names.

## Steps
1. **Name check:** no real brands, models or trademarks. Invent a name ("Stratoblaster", not the real one).
2. **Sprite check** (`tools/assets` validator when it exists, otherwise inspect the PNG):
   - canvas matches the slot size (character layers are 32 × 48 art-px, aligned to the base body)
   - only colours from the art palette, 1 art-px `outline`, no semi-transparent pixels
   - check it on both the guitarist and bassist base poses
3. **Price and unlock:** pick them from the tier table in `docs/GAMIFICATION.md` §3. Legendary items cost Gold Records.
4. **Register** in `assets/items.json`: `{id, slot, class, tier, price, currency, unlockLevel, sprite, nameKey}`.
5. **Strings:** add `item.<id>.name` to `en.json` and `pt-PT.json`.
6. **Starter items** also go in the onboarding list in `docs/GAMIFICATION.md` §4.

## Done checklist
- [ ] Asset validator passes
- [ ] `/i18n-check` is clean
- [ ] The item shows correctly in the Shop preview and on Home
