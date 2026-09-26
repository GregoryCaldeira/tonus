#!/usr/bin/env bash
# Regenerate the LVGL pixel fonts in ui/fonts/ from the OFL sources.
#   Display/buttons: Silkscreen Bold   (wide, all-caps pixel face, matches the button references)
#   Body/labels:     Pixelify Sans     (mixed case, readable at small sizes)
# Fonts are 1 bpp so pixels stay crisp. Range = ASCII + Latin-1 + typographic punctuation (PT-PT needs ç ã õ …).
set -euo pipefail
here="$(cd "$(dirname "$0")" && pwd)"
root="$(cd "$here/../.." && pwd)"
src="$here/src"
out="$root/ui/fonts"
conv="$here/node_modules/.bin/lv_font_conv"

[ -x "$conv" ] || (cd "$here" && npm ci --silent)
mkdir -p "$src" "$out"

fetch() { # <google/fonts path> <local name>
  [ -f "$src/$2" ] || curl -fsSL "https://raw.githubusercontent.com/google/fonts/main/ofl/$1" -o "$src/$2"
}
fetch silkscreen/Silkscreen-Bold.ttf        Silkscreen-Bold.ttf
fetch silkscreen/OFL.txt                    Silkscreen-OFL.txt
fetch "pixelifysans/PixelifySans%5Bwght%5D.ttf" PixelifySans.ttf
fetch pixelifysans/OFL.txt                  PixelifySans-OFL.txt

RANGE="0x20-0x7E,0xA0-0xFF,0x2013-0x2014,0x2018-0x201D,0x2026,0x20AC"

gen() { # <ttf> <size> <name>   (relative paths keep the generated header machine-independent)
  (cd "$root" && "$conv" --font "tools/fonts/src/$1" --size "$2" --bpp 1 --range "$RANGE" --format lvgl \
    --no-compress --lv-include lvgl.h --lv-font-name "$3" -o "ui/fonts/$3.c")
  echo "  ui/fonts/$3.c"
}

echo "Generating fonts:"
gen Silkscreen-Bold.ttf 48 tonus_font_display_48
gen Silkscreen-Bold.ttf 32 tonus_font_display_32
gen Silkscreen-Bold.ttf 24 tonus_font_display_24
gen PixelifySans.ttf    32 tonus_font_body_32
gen PixelifySans.ttf    24 tonus_font_body_24
gen PixelifySans.ttf    16 tonus_font_body_16

cp "$src/Silkscreen-OFL.txt"   "$out/LICENSE-Silkscreen-OFL.txt"
cp "$src/PixelifySans-OFL.txt" "$out/LICENSE-PixelifySans-OFL.txt"
