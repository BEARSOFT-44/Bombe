# Simulateur de la bombe

Page web qui fait tourner le vrai code `BombeV2_1/BombeV2_1.ino` : écran OLED, bandeau LED, buzzer et les trois boutons, avec une vitesse du temps réglable.

## Comment c'est fait

Le `.ino` est compilé tel quel en WebAssembly avec clang, comme le ferait l'IDE Arduino, et lié aux vraies bibliothèques :

- **U8g2** (`vendor/u8g2`, version d6c8499) : même rendu au pixel près, mêmes polices. Seul le pilote de l'écran est remplacé, il recopie l'image dans la page au lieu de l'envoyer en SPI.
- **EasyButton 2.0.3** (`vendor/EasyButton`) : même anti-rebond (35 ms), appui court déclenché au relâchement, appui long à 2 s.
- **Arduino, NeoPixel, sprintf** : imitations dans `shim/` (temps `millis()`, broches, couleurs des LEDs).

Différences avec la vraie carte : `int` fait 32 bits ici au lieu de 16 sur le Mega, et la durée d'un tour de `loop()` est un réglage de la page (20 ms par défaut) au lieu de dépendre du processeur.

## Mettre à jour après une modification du code

1. Demander à Claude dans le projet : « reconstruis le simulateur et republie-le ». Il lance :
   ```sh
   sh simulateur/construire.sh
   ```
   puis republie `simulateur-bombe.html` sur le même lien.
2. Vérifier dans l'en-tête de la page que l'empreinte et la date de compilation ont changé.

Sur un PC avec clang, wasm-ld et python3 : `sh construire.sh chemin/vers/sketch.ino sortie.html`, puis ouvrir le fichier HTML dans un navigateur.

Les polices utilisées par le code sont retrouvées automatiquement. Si une nouvelle police apparaît dans le `.ino`, le script la télécharge une fois depuis GitHub et la garde dans `vendor/polices_cache.c`. Les boutons (`EasyButton NOM(broche)`) et la broche du buzzer (constante dont le nom contient « buzz ») sont lus dans le code.

Si le code ne compile pas, le script affiche l'erreur de clang. Comme ici il n'y a pas de génération automatique des prototypes comme dans l'IDE Arduino, une fonction utilisée avant sa définition doit avoir son prototype en haut du fichier (c'est déjà le cas dans le code actuel).
