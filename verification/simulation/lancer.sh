#!/bin/sh
# Compile le sketch sur PC avec les fausses bibliothèques (dossier inc/) et lance les scénarios de test.
cd "$(dirname "$0")"
cp ../../BombeV2_1/BombeV2_1.ino sketch.cpp
g++ -std=c++17 -w -Iinc -include Arduino.h -DSRC='"sketch.cpp"' test.cpp -o test && ./test
