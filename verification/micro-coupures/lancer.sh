#!/bin/sh
# Test des micro-coupures de boutons, sur PC avec g++ : sh verification/micro-coupures/lancer.sh
# Compile le sketch avec la vraie bibliothèque EasyButton (simulateur/vendor/EasyButton) et de fausses
# bibliothèques pour l'écran et les LEDs (dossier inc/), puis rejoue les scénarios de test.cpp.
cd "$(dirname "$0")"
EB=../../simulateur/vendor/EasyButton
TMP=$(mktemp -d)
cp ../../BombeV2_1/BombeV2_1.ino "$TMP/sketch.cpp"
g++ -std=c++17 -w -Iinc -I"$EB" -include Arduino.h -DSRC="\"$TMP/sketch.cpp\"" \
    test.cpp "$EB/EasyButton.cpp" "$EB/EasyButtonBase.cpp" "$EB/Sequence.cpp" -o "$TMP/test" && "$TMP/test"
CODE=$?
rm -rf "$TMP"
exit $CODE
