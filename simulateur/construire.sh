#!/bin/sh
# Construit la page du simulateur a partir du code Arduino.
# Usage : sh construire.sh [chemin/vers/sketch.ino] [page_sortie.html]
# Necessite : clang (cible wasm32), wasm-ld, python3. Aucune bibliotheque a installer.
set -e
ICI="$(cd "$(dirname "$0")" && pwd)"
INO="${1:-$ICI/../BombeV2_1/BombeV2_1.ino}"
SORTIE="${2:-$ICI/simulateur-bombe.html}"
TMP="$(mktemp -d)"
trap 'rm -rf "$TMP"' EXIT
U="$ICI/vendor/u8g2"; E="$ICI/vendor/EasyButton"
python3 "$ICI/outils/extraire_polices.py" "$INO" "$TMP/polices.c"
CFLAGS="--target=wasm32 -O2 -ffreestanding -nostdinc -isystem $(clang -print-resource-dir)/include -I$ICI/shim -I$U -I$E -DARDUINO=10819 -DARDUINO_AVR_MEGA2560 -DU8X8_NO_HW_SPI -DU8X8_NO_HW_I2C -ffunction-sections -fdata-sections -w"
CXXFLAGS="$CFLAGS -std=gnu++17 -fno-exceptions -fno-rtti -fno-threadsafe-statics"
for f in "$U"/*.c "$TMP/polices.c"; do
  nom=$(basename "$f" .c)
  extra=""; [ "$nom" = "u8x8_d_ssd1306_128x64_noname" ] && extra="-Du8x8_d_ssd1306_128x64_noname=u8x8_d_ssd1306_128x64_noname_reel"
  clang $CFLAGS $extra -c "$f" -o "$TMP/$nom.o" &
done
wait
clang $CFLAGS -c "$ICI/shim/libc.c" -o "$TMP/libc.o"
for f in "$U"/U8x8lib_extrait.cpp "$E"/EasyButton.cpp "$E"/EasyButtonBase.cpp "$E"/Sequence.cpp "$ICI/shim/simulateur.cpp"; do
  clang++ $CXXFLAGS -c "$f" -o "$TMP/$(basename "$f" .cpp)_cpp.o"
done
# Le sketch est compile tel quel, comme le fait l'IDE Arduino (avec #include <Arduino.h> ajoute en tete)
cp "$INO" "$TMP/sketch.cpp"
clang++ $CXXFLAGS -include Arduino.h -c "$TMP/sketch.cpp" -o "$TMP/sketch_cpp.o" -Wno-everything 2> "$TMP/err.txt" || { cat "$TMP/err.txt"; echo "ERREUR : le sketch ne compile pas."; exit 1; }
wasm-ld --no-entry --gc-sections -z stack-size=65536 --initial-memory=1048576 \
  "$TMP"/*.o -o "$TMP/bombe.wasm"
python3 "$ICI/outils/assembler_page.py" "$INO" "$TMP/bombe.wasm" "$ICI/page.html" "$SORTIE"
