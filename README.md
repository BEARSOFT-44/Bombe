# La Bombe – Bearsoft 44

Code et documentation de la « bombe » d'airsoft de l'asso : un boîtier Arduino avec écran, bandeau de LEDs, buzzer et trois boutons, qui propose plusieurs modes de jeu (BOMBE, CTF, MINE, PATATE).

Site officiel de l'asso : **[bearsoft44.fr](https://bearsoft44.fr)**

Version actuelle : **V2.1.2**.

## Essayer sans la bombe

Le simulateur fait tourner le vrai code dans le navigateur, avec l'écran, les LEDs, le buzzer et les boutons :

👉 **https://claude.ai/artifact/VVS2Nbts8f463BBhZvCEnE**

On peut aussi télécharger `simulateur/simulateur-bombe.html` et l'ouvrir directement dans un navigateur, sans connexion.

## Ce qu'il y a dans chaque dossier

| Dossier | À quoi ça sert |
|---|---|
| `BombeV2_1/` | Le code à téléverser dans la bombe (`BombeV2_1.ino`). La liste des changements de chaque version est en haut du fichier. |
| `notice/` | Le mode d'emploi pour les joueurs et les orgas, en Word (modifiable) et en PDF (à imprimer). |
| `simulateur/` | Le simulateur web et ce qui sert à le fabriquer. Détails dans [`simulateur/LISEZMOI.md`](simulateur/LISEZMOI.md). |
| `verification/simulation/` | Des tests automatiques qui rejouent des parties sur PC pour vérifier que les modes se comportent comme prévu. |

## Téléverser le code dans la bombe

1. Installer l'[IDE Arduino](https://www.arduino.cc/en/software).
2. Dans **Outils > Gérer les bibliothèques**, installer :
   - **U8g2** (écran)
   - **Adafruit NeoPixel** (LEDs)
   - **EasyButton** (boutons)
3. Télécharger ce dépôt (bouton vert **Code > Download ZIP**) et le dézipper.
4. Ouvrir `BombeV2_1/BombeV2_1.ino` dans l'IDE Arduino. Le fichier doit rester dans un dossier qui porte le même nom, `BombeV2_1`.
5. Brancher la bombe en USB, puis choisir :
   - **Outils > Type de carte > Arduino Mega or Mega 2560**
   - **Outils > Port** : le port qui apparaît quand on branche la bombe
6. Cliquer sur la flèche **Téléverser**. Quand c'est fini, la bombe redémarre avec la nouvelle version.

## Modifier le code

- Avant de téléverser une modification, l'essayer dans le simulateur (voir [`simulateur/LISEZMOI.md`](simulateur/LISEZMOI.md) pour le reconstruire).
- Pour les tests automatiques, sur un PC avec `g++` : `sh verification/simulation/lancer.sh`.
- Penser à noter les changements en haut du `.ino` et à mettre à jour la notice.
