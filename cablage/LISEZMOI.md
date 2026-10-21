# Schéma de câblage de la bombe

Schéma électronique de principe de la bombe, réalisé par **Skitus** ([@Skitusss](https://github.com/Skitusss)) avec [KiCad](https://www.kicad.org/) (version 10).

On y trouve la carte Arduino Mega 2560 et ce qui s'y branche :
- l'écran OLED 2,42 pouces ;
- le bandeau de LEDs WS2812B ;
- le buzzer, commandé par un transistor 2N3906 ;
- les trois boutons GAUCHE, OK et DROITE ;
- l'alimentation par batterie.

📄 **[Voir le schéma en PDF](cablage_bombe.pdf)**, sans installer KiCad. C'est un export du schéma : le projet KiCad ci-dessous reste la version à modifier.

## Ouvrir le schéma dans KiCad

1. Installer [KiCad](https://www.kicad.org/download/) (gratuit), version 10 ou plus récente.
2. Ouvrir `Schéma de principe.kicad_pro`.
3. Double-cliquer sur le schéma (`.kicad_sch`) ou sur le circuit imprimé (`.kicad_pcb`).

Tous les symboles sont enregistrés dans le schéma lui-même : il s'ouvre sans installer de bibliothèque en plus.

## Contenu du dossier

| Fichier | Rôle |
|---|---|
| `cablage_bombe.pdf` | Le schéma en PDF (A3), à consulter ou imprimer |
| `Schéma de principe.kicad_pro` | Le projet KiCad |
| `Schéma de principe.kicad_sch` | Le schéma de câblage |
| `Schéma de principe.kicad_pcb` | Le circuit imprimé (carte « shield » qui se pose sur la Mega) |
| `New_Library.kicad_sym` | Symbole de l'écran OLED 2,42 pouces, dessiné pour ce projet |
| `Arduino_MountingHole.pretty/` | Empreintes des trous de fixation de la carte Arduino |
| `fp-lib-table`, `sym-lib-table` | Indiquent à KiCad où trouver les deux bibliothèques ci-dessus |

Les sauvegardes automatiques de KiCad et la bibliothèque Digi-Key, qui n'est pas utilisée dans le schéma, ne sont pas incluses.
