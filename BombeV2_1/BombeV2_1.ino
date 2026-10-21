/*FONCTIONNEMENT DU CODE DE LA BOMBE:
Cette bombe base son architecture autour d'un squelette de "switchs" (d'états).
Cette structure, retrouvable à la fin du code, sert à coordonner les actions, et à ne
déclencher ces actions que lorsque la bombe est dans l'état requis.

AJOUTS V2.1 :
  - CTF : paramètre "Délai de début" (en minutes) avec bip buzzer au lancement effectif
  - Nouveau mode MINE :
      * Toutes les X minutes un diamant apparaît (paramétrable)
      * Capture en maintenant le bouton gauche (ROUGE) ou droite (BLEU) X secondes
      * Bip buzzer à chaque apparition de diamant
      * Fin de partie au bout du timer : affichage du score (nb diamants par équipe)
      * LEDs : clignotement blanc = diamant dispo, rouge/bleu = capture en cours,
               barre proportionnelle = score entre les deux apparitions

Correction V2.1.1
  - Bibliothèques plus à jour
  - Correction du bug qui empeche le redémarrage du mode de jeu dans la foulée quand la bombe est sans code et réamorcable

Correction V2.1.2
  - CTF : le temps de contrôle est mesuré depuis la prise de la zone (DebutControle) et non plus depuis
    le dernier appui adverse (DebutCapture), qui faussait les compteurs de fin de partie
  - MINE : la capture est annulée si le bouton est relâché ; le délai de début fonctionne (écran "DEBUT DANS")
  - BOMBE : passage en fin de partie après l'explosion
  - PATATE : durée jamais négative (latitude > timer) et vraiment aléatoire (randomSeed)
  - Écran des réglages : le titre "SETTINGS BOMBE" ne se superpose plus aux autres modes
  - Bips : départ et fin de partie CTF (toujours actifs, avec ou sans délai de départ), départ, diamant et fin
    de partie MINE (nouveau réglage "Bip mine"), armement de la bombe ("Bip bombe").
    Le réglage "Bip CTF", qui ne servait à rien, est supprimé : le menu CTF passe à 3 paramètres
  - Buzzer : nouvelle fonction LancerBip(durée) pour les bips ponctuels, prioritaire sur les bips propres aux modes
    (remplace BipDebutCTF et NouveauDiamant)
  - MINE : le premier diamant est compté à partir du vrai départ (après le délai)
  - Suppression des prototypes RelacheGauche/RelacheDroite, jamais définis ni utilisés
  - PATATE : sous 5 min, le bip ne saute plus de 500 ms à 1000 ms (durée = temps restant / 600 au lieu de / 300)*/
// ======================================================
// IMPORTATION DES BIBLIOTHÈQUES
// ======================================================
#include <Arduino.h>
#include <U8g2lib.h>
#include <Adafruit_NeoPixel.h>
#include <Wire.h>
#include <EasyButton.h>

// ======================================================
// CONFIGURATION DES PINS
// ======================================================
#define BAUDRATE 115200

#define LED_PIN     3
#define NUM_PIXELS  10

#define OLED_SCK    27
#define OLED_SDA    29
#define OLED_RES    31
#define OLED_DC     33
#define OLED_CS     35

EasyButton BPGauche(22);
EasyButton BPDroite(26);
EasyButton BPOK(30);
const int Buzzer = 32;

unsigned long AncienTemps = 0;
const long intervalle = 100;

// ======================================================
// CRÉATION DES OBJETS
// ======================================================
U8G2_SSD1306_128X64_NONAME_F_4W_SW_SPI MonEcran(U8G2_R0, OLED_SCK, OLED_SDA, OLED_CS, OLED_DC, OLED_RES);
Adafruit_NeoPixel BandeLed(NUM_PIXELS, LED_PIN, NEO_GRB + NEO_KHZ800);

enum Etats { MENU_JEUX, MENU_SETTINGS, EN_JEU, FIN_DE_JEU };
enum Mode  { BOMBE, CTF, PATATE_CHAUDE, MINE };
Etats EtatActuel = MENU_JEUX;
Mode  ModeJeu    = BOMBE;

// ======================================================
// VARIABLES PAR DÉFAUT – SETTINGS MODES DE JEU
// ======================================================

// ── BOMBE ─────────────────────────────────────────────
int  TimerBombe        = 15;
int  TimerDesamorcage  = 30;
bool Reamorcable       = false;
int  PenaliteFauxCode  = 10;
bool BipBomb           = true;
bool BesoinCode        = true;
int  NbParamBomb       = 6; //A modifier si de nouveaux paramètres réglables sont ajoutés
int  TimerExplosion    = 5000;

// ── CTF ───────────────────────────────────────────────
int  TimerCTF     = 30;
int  TimerCapture = 20;
int  DelaiDebutCTF = 5;
int  NbParamCTF   = 3; //A modifier si de nouveaux paramètres réglables sont ajoutés

// ── PATATE CHAUDE ─────────────────────────────────────
int  TimerPatate    = 30;
int  LatitudeRandom = 5;
bool BipPatate      = true;
int  NbParamPatate  = 3; //A modifier si de nouveaux paramètres réglables sont ajoutés

// ── MINE ──────────────────────────────────────────────
int TimerMine                   = 30;
int IntervalleApparitionDiamant = 3;
int TimerCaptureMine            = 15;
int DelaiDebutMine 				      = 5;
bool BipMine                    = true;
int NbParamMine                 = 5; //A modifier si de nouveaux paramètres réglables sont ajoutés

// ======================================================
// DÉCLARATION DES VARIABLES
// ======================================================
int           PositionMenu = 1;
unsigned long TempsActuel  = millis();
long          TempsRestant = 0;
enum teams { BLEU, NEUTRE, ROUGE };

// Variables BOMBE
bool          BombeAmorcee   = false;
unsigned long DebutAmorcage  = 0;
unsigned long DebutExplosion = 0;
bool          Explosion      = false;
unsigned long TotalPenalites = 0;
unsigned long statTempsRestant;
bool          BombeExplosee  = false;
unsigned long DebutDesamorcage = 0;
bool DesamorcageEnCours = false;

const int TAILLE_CODE = 4;
int CodeSecret[TAILLE_CODE] = {0, 0, 0, 0};
int CodeSaisi[TAILLE_CODE]  = {0, 0, 0, 0};
int IndexCode = 0;

// Variables CTF
bool          EnCaptureGauche  = false;
bool          EnCaptureDroite  = false;
teams         EquipeEnControle = NEUTRE;
unsigned long DebutCTF         = 0;
unsigned long CompteurBleu     = 0;
unsigned long CompteurRouge    = 0;
unsigned long DebutCapture     = 0; // début de l'appui en cours (capture)
unsigned long DebutControle    = 0; // CORRECTIF : début du contrôle de la zone par l'équipe actuelle
int           statCTFRouge     = 0;
int           statCTFBleu      = 0;
long          secRestantes     = 0;
bool          CTFEnAttente     = false;
unsigned long DebutAttenteCTF  = 3;

// Variables PATATE_CHAUDE
unsigned long DebutPatate      = 0;
unsigned long minPartie        = 0;
unsigned long maxPartie        = 0;
unsigned long DureeCiblePatate = 0;

// Variables MINE
bool          DiamantActif          = false;
unsigned long DebutMine             = 0;
unsigned long DernierDiamantTemps   = 0;
bool          EnCaptureMineGauche   = false;
bool          EnCaptureMineDroite   = false;
unsigned long DebutCaptureMine      = 0;
int           CompteurDiamantsRouge = 0;
int           CompteurDiamantsBleu  = 0;
int           statDiamantsRouge     = 0;
int           statDiamantsBleu      = 0;
bool          MineEnAttente     = false;
unsigned long DebutAttenteMine  = 3;

// Variables BUZZER
bool          Bip          = false;
int           DureeBip     = 0;
int           IntervalleBip = 0;
unsigned long DebutBip     = 0;
unsigned long FinBip       = 0;
// Bip ponctuel (départ, fin, armement, diamant) : prioritaire sur les bips propres aux modes
bool          BipEvenement      = false;
unsigned long DebutBipEvenement = 0;
unsigned long DureeBipEvenement = 0;

void LancerBip(unsigned long duree) {
  BipEvenement      = true;
  DebutBipEvenement = TempsActuel;
  DureeBipEvenement = duree;
}

// Variables LEDs
unsigned long DernierClignotement = 0;
bool          EtatClignotement    = false;
bool majLeds = true; // Drapeau d'autorisation de rafraîchissement
Etats memoEtatActuel = MENU_JEUX;
Mode memoModeJeu = BOMBE;
bool memoExplosion = false;
bool memoEnCaptureGauche = false;
bool memoEnCaptureDroite = false;
teams memoEquipeEnControle = NEUTRE;
bool memoDiamantActif = false;
bool memoEnCaptureMineGauche = false;
bool memoEnCaptureMineDroite = false;
int memoCompteurDiamantsRouge = 0;
int memoCompteurDiamantsBleu = 0;

// ======================================================
// PROTOTYPES
// ======================================================
void AppuiGauche();
void AppuiDroite();
void AppuiOK();
void AppuiOKLong();
void GererBuzzer();
void rafraichirEcran();
void Leds();
void DetecterChangementsLeds();
void GererClignotementsLeds();

// ======================================================
// SETUP
// ======================================================
void setup() {
  MonEcran.begin();
  MonEcran.clearBuffer();
  BandeLed.begin();
  BandeLed.show();

  MonEcran.setFont(u8g2_font_logisoso20_tr);
  int largeurtexte1 = MonEcran.getStrWidth("BEARSOFT");
  int x1 = (128 - largeurtexte1) / 2;
  MonEcran.drawStr(x1, 22, "BEARSOFT");

  int largeurtexte2 = MonEcran.getStrWidth("44");
  int x2 = (128 - largeurtexte2) / 2;
  MonEcran.drawStr(x2, 44, "44");

  int hauteurTrait = 36;
  int espace = 8;
  MonEcran.drawHLine(0, hauteurTrait, x2 - espace);
  int departTraitDroit   = x2 + largeurtexte2 + espace;
  int longueurTraitDroit = 128 - departTraitDroit;
  MonEcran.drawHLine(departTraitDroit, hauteurTrait, longueurTraitDroit);

  MonEcran.setFont(u8g2_font_pressstart2p_8f);
  int largeurtexte3 = MonEcran.getStrWidth("V2.1 by SKITUS");
  int x3 = (128 - largeurtexte3) / 2;
  MonEcran.drawStr(x3, 60, "V2.1 by SKITUS");
  MonEcran.sendBuffer();
  delay(3000);

  Serial.begin(115200);
  pinMode(Buzzer, OUTPUT);
  randomSeed(analogRead(A0)); // broche analogique non branchée = graine différente à chaque allumage

  BPGauche.begin();
  BPGauche.onPressed(AppuiGauche);
  BPDroite.begin();
  BPDroite.onPressed(AppuiDroite);
  BPOK.begin();
  BPOK.onPressed(AppuiOK);
  BPOK.onPressedFor(2000, AppuiOKLong);
}

// ======================================================
// BOUCLE LOOP
// ======================================================
void loop() {
  BPGauche.read();
  BPDroite.read();
  BPOK.read();

  TempsActuel = millis();

  switch (EtatActuel) {

    // ────────────────────────────────────────────────────
    case MENU_JEUX:
    break;
    case MENU_SETTINGS:
    break;

    // ────────────────────────────────────────────────────
    case EN_JEU:
      switch (ModeJeu) {

        // ── BOMBE ──────────────────────────────────────
        case BOMBE:
          if (BombeAmorcee) {
            TempsRestant = (long)(DebutAmorcage + TimerBombe * 60UL * 1000UL - TempsActuel - TotalPenalites);
            if (TempsRestant <= 0) {
              Explosion      = true;
              TotalPenalites = 0;
              DebutExplosion = TempsActuel;
              BombeAmorcee   = false;
              BombeExplosee  = true;
            }
            if (!BesoinCode) {
              if (!BPOK.isPressed()) {
                DesamorcageEnCours = false;
              } else if (!DesamorcageEnCours) {
                DesamorcageEnCours = true;
                DebutDesamorcage   = TempsActuel;
              } else if (TempsActuel >= DebutDesamorcage + TimerDesamorcage * 1000UL) {
                DesamorcageEnCours = false;
                BombeAmorcee       = false;
                statTempsRestant   = TempsRestant;
                EtatActuel         = FIN_DE_JEU;
              }
            }
          }
          if (Explosion && TempsActuel >= DebutExplosion + TimerExplosion) {
            Explosion  = false;
            EtatActuel = FIN_DE_JEU;
          }
        break;

        // ── CTF ────────────────────────────────────────
        case CTF:
          if (CTFEnAttente) {
            // ---- Countdown avant démarrage effectif ----
            TempsRestant = (long)(DebutAttenteCTF + (unsigned long)DelaiDebutCTF * 1000UL * 60UL - TempsActuel);
            if (TempsActuel >= DebutAttenteCTF + (unsigned long)DelaiDebutCTF * 1000UL * 60UL) {
              CTFEnAttente  = false;
              DebutCTF      = TempsActuel;
              LancerBip(1000);
            }
          } else {
            // ---- Partie CTF normale ----
            TempsRestant = (long)(DebutCTF + TimerCTF * 60UL * 1000UL - TempsActuel);

            if (TempsActuel <= DebutCTF + TimerCTF * 60UL * 1000UL) {
              if (EquipeEnControle != ROUGE) {
                if (BPGauche.isPressed() && !EnCaptureGauche && !EnCaptureDroite) {
                  DebutCapture    = TempsActuel;
                  EnCaptureGauche = true;
                } else if (EnCaptureGauche && TempsActuel >= DebutCapture + TimerCapture * 1000UL) {
                  if (EquipeEnControle == NEUTRE) {
                    EquipeEnControle = ROUGE;
                    DebutControle    = TempsActuel;
                    EnCaptureGauche  = false;
                  } else if (EquipeEnControle == BLEU) {
                    EquipeEnControle = NEUTRE;
                    CompteurBleu    += TempsActuel - DebutControle;
                    EnCaptureGauche  = true;
                    DebutCapture     = TempsActuel;
                  }
                }
                if (!BPGauche.isPressed()) {
                  EnCaptureGauche = false;
                }
              }

              if (EquipeEnControle != BLEU) {
                if (BPDroite.isPressed() && !EnCaptureGauche && !EnCaptureDroite) {
                  DebutCapture    = TempsActuel;
                  EnCaptureDroite = true;
                } else if (EnCaptureDroite && TempsActuel >= DebutCapture + TimerCapture * 1000UL) {
                  if (EquipeEnControle == NEUTRE) {
                    EquipeEnControle = BLEU;
                    DebutControle    = TempsActuel;
                    EnCaptureDroite  = false;
                  } else if (EquipeEnControle == ROUGE) {
                    EquipeEnControle = NEUTRE;
                    CompteurRouge   += TempsActuel - DebutControle;
                    EnCaptureDroite  = true;
                    DebutCapture     = TempsActuel;
                  }
                }
                if (!BPDroite.isPressed()) {
                  EnCaptureDroite = false;
                }
              }
            } else {
              // Fin de partie CTF
              if      (EquipeEnControle == ROUGE) CompteurRouge += TempsActuel - DebutControle;
              else if (EquipeEnControle == BLEU)  CompteurBleu  += TempsActuel - DebutControle;
              statCTFRouge = CompteurRouge / 1000;
              statCTFBleu  = CompteurBleu  / 1000;
              LancerBip(2000);
              EtatActuel   = FIN_DE_JEU;
            }
          }
        break;

        // ── PATATE CHAUDE ──────────────────────────────
        case PATATE_CHAUDE:
          TempsRestant = (long)(DebutPatate + DureeCiblePatate - TempsActuel);
          if (TempsActuel >= DebutPatate + DureeCiblePatate && !Explosion) {
            Explosion      = true;
            DebutExplosion = TempsActuel;
          }
          if (Explosion && TempsActuel >= DebutExplosion + TimerExplosion) {
            Explosion  = false;
            EtatActuel = FIN_DE_JEU;
          }
        break;

        // ── MINE ───────────────────────────────────────
        case MINE:
          TempsRestant = (long)(DebutMine + (unsigned long)TimerMine * 60UL * 1000UL - TempsActuel);
          if (MineEnAttente) {
            // ---- Countdown avant démarrage effectif ----
            TempsRestant = (long)(DebutAttenteMine + (unsigned long)DelaiDebutMine * 1000UL * 60UL - TempsActuel);
            if (TempsActuel >= DebutAttenteMine + (unsigned long)DelaiDebutMine * 1000UL * 60UL) {
              MineEnAttente       = false;
              DebutMine           = TempsActuel;
              DernierDiamantTemps = TempsActuel;
              if (BipMine) LancerBip(1000);
            }
          } else if (TempsActuel <= DebutMine + (unsigned long)TimerMine * 60UL * 1000UL) { // a éditer pour coller à la syntaxe du CTF (cohérence)

            // ---- Apparition périodique du diamant ----
            if (!DiamantActif && TempsActuel >= DernierDiamantTemps + (unsigned long)IntervalleApparitionDiamant * 60UL * 1000UL) {
              DiamantActif         = true;
              DernierDiamantTemps  = TempsActuel;
              EnCaptureMineGauche  = false;
              EnCaptureMineDroite  = false;
              if (BipMine) LancerBip(300);
            }

            // ---- Logique de capture ----
            if (DiamantActif) {
              if (BPGauche.isPressed() && !EnCaptureMineGauche && !EnCaptureMineDroite) {
                EnCaptureMineGauche = true;
                DebutCaptureMine    = TempsActuel;
              }
              if (BPDroite.isPressed() && !EnCaptureMineDroite && !EnCaptureMineGauche) {
                EnCaptureMineDroite = true;
                DebutCaptureMine    = TempsActuel;
              }
              // Relâcher le bouton annule la capture
              if (EnCaptureMineGauche && !BPGauche.isPressed()) EnCaptureMineGauche = false;
              if (EnCaptureMineDroite && !BPDroite.isPressed()) EnCaptureMineDroite = false;

              if (EnCaptureMineGauche && TempsActuel >= DebutCaptureMine + (unsigned long)TimerCaptureMine * 1000UL) {
                CompteurDiamantsRouge++;
                DiamantActif        = false;
                EnCaptureMineGauche = false;
              }
              if (EnCaptureMineDroite && TempsActuel >= DebutCaptureMine + (unsigned long)TimerCaptureMine * 1000UL) {
                CompteurDiamantsBleu++;
                DiamantActif        = false;
                EnCaptureMineDroite = false;
              }
            }
          } else {
            // Fin de partie MINE
            statDiamantsRouge = CompteurDiamantsRouge;
            statDiamantsBleu  = CompteurDiamantsBleu;
            if (BipMine) LancerBip(2000);
            EtatActuel        = FIN_DE_JEU;
          }
        break;
      }
    break;

    // ────────────────────────────────────────────────────
    case FIN_DE_JEU:
      
    break;
  }
  DetecterChangementsLeds();
  GererClignotementsLeds();

  if (majLeds) {
    Leds();
    majLeds = false; // On reset le drapeau une fois mis à jour
  }

  rafraichirEcran();
  GererBuzzer();
}

// ======================================================
// APPUI COURT – BOUTON GAUCHE
// ======================================================
void AppuiGauche() {
  switch (EtatActuel) {

    case MENU_JEUX:
      PositionMenu = 1;
      ModeJeu = (ModeJeu > BOMBE) ? (Mode)(ModeJeu - 1) : MINE;
    break;

    case MENU_SETTINGS:
      switch (ModeJeu) {
        case BOMBE:
          switch (PositionMenu) {
            case 1: TimerBombe       = max(5,  TimerBombe - 5);       break;
            case 2: TimerDesamorcage = max(10, TimerDesamorcage - 10);break;
            case 3: Reamorcable      = !Reamorcable;                  break;
            case 4: PenaliteFauxCode = max(0,  PenaliteFauxCode - 5); break;
            case 5: BipBomb          = !BipBomb;                      break;
            case 6: BesoinCode       = !BesoinCode;                   break;
          }
        break;
        case CTF:
          switch (PositionMenu) {
            case 1: TimerCTF      = max(5,  TimerCTF - 5);       break;
            case 2: TimerCapture  = max(10, TimerCapture - 10);   break;
            case 3: DelaiDebutCTF = max(0,  DelaiDebutCTF - 1);   break;
          }
        break;
        case PATATE_CHAUDE:
          switch (PositionMenu) {
            case 1: TimerPatate    = max(5,  TimerPatate - 5);    break;
            case 2: LatitudeRandom = max(0,  LatitudeRandom - 1); break;
            case 3: BipPatate      = !BipPatate;                   break;
          }
        break;
        case MINE:
          switch (PositionMenu) {
            case 1: TimerMine                   = max(5,  TimerMine - 5);                    break;
            case 2: IntervalleApparitionDiamant = max(1,  IntervalleApparitionDiamant - 1); break;
            case 3: TimerCaptureMine            = max(5,  TimerCaptureMine - 5);             break;
			case 4: DelaiDebutMine            	= max(0,  DelaiDebutMine - 1);               break;
            case 5: BipMine                     = !BipMine;                                  break;
          }
        break;
      }
    break;

    case EN_JEU:
      if (ModeJeu == BOMBE) {
        CodeSaisi[IndexCode]--;
        if (CodeSaisi[IndexCode] < 0) CodeSaisi[IndexCode] = 9;
      }
    break;

    case FIN_DE_JEU:
    break;
  }
}

// ======================================================
// APPUI COURT – BOUTON DROITE
// ======================================================
void AppuiDroite() {
  switch (EtatActuel) {

    case MENU_JEUX:
      PositionMenu = 1;
      ModeJeu = (ModeJeu < MINE) ? (Mode)(ModeJeu + 1) : BOMBE;
    break;

    case MENU_SETTINGS:
      switch (ModeJeu) {
        case BOMBE:
          switch (PositionMenu) {
            case 1: TimerBombe       = min(60,  TimerBombe + 5);        break;
            case 2: TimerDesamorcage = min(120, TimerDesamorcage + 10); break;
            case 3: Reamorcable      = !Reamorcable;                    break;
            case 4: PenaliteFauxCode = min(60,  PenaliteFauxCode + 5);  break;
            case 5: BipBomb          = !BipBomb;                        break;
            case 6: BesoinCode       = !BesoinCode;                     break;
          }
        break;
        case CTF:
          switch (PositionMenu) {
            case 1: TimerCTF      = min(60,  TimerCTF + 5);        break;
            case 2: TimerCapture  = min(120, TimerCapture + 10);    break;
            case 3: DelaiDebutCTF = min(30,  DelaiDebutCTF + 1);    break;
          }
        break;
        case PATATE_CHAUDE:
          switch (PositionMenu) {
            case 1: TimerPatate    = min(60, TimerPatate + 5);     break;
            case 2: LatitudeRandom = min(15, LatitudeRandom + 1);  break;
            case 3: BipPatate      = !BipPatate;                    break;
          }
        break;
        case MINE:
          switch (PositionMenu) {
            case 1: TimerMine                   = min(60, TimerMine + 5);                    break;
            case 2: IntervalleApparitionDiamant = min(10, IntervalleApparitionDiamant + 1); break;
            case 3: TimerCaptureMine            = min(60, TimerCaptureMine + 5);             break;
			case 4: DelaiDebutMine            	= min(30,  DelaiDebutMine + 1);               break;
            case 5: BipMine                     = !BipMine;                                  break;
          }
        break;
      }
    break;

    case EN_JEU:
      if (ModeJeu == BOMBE) {
        CodeSaisi[IndexCode]++;
        if (CodeSaisi[IndexCode] > 9) CodeSaisi[IndexCode] = 0;
      }
    break;

    case FIN_DE_JEU:
    break;
  }
}

// ======================================================
// APPUI COURT – BOUTON OK
// ======================================================
void AppuiOK() {
  switch (EtatActuel) {

    case MENU_JEUX:
      switch (ModeJeu) {
        case BOMBE:
          EtatActuel = EN_JEU;
        break;
        case CTF:
          CompteurBleu     = 0;
          CompteurRouge    = 0;
          EquipeEnControle = NEUTRE;
          EnCaptureGauche  = false;
          EnCaptureDroite  = false;
          if (DelaiDebutCTF > 0) {
            CTFEnAttente    = true;
            DebutAttenteCTF = TempsActuel;
          } else {
            CTFEnAttente = false;
            DebutCTF     = TempsActuel;
            LancerBip(1000);
          }
          EtatActuel = EN_JEU;
        break;
        case PATATE_CHAUDE:
          // Si la latitude dépasse le timer, on borne la durée minimale à 1 min (sinon valeur négative = partie infinie)
          minPartie        = (TimerPatate > LatitudeRandom) ? (unsigned long)(TimerPatate - LatitudeRandom) * 60UL * 1000UL : 60UL * 1000UL;
          maxPartie        = TimerPatate * 60UL * 1000UL + LatitudeRandom * 60UL * 1000UL;
          DureeCiblePatate = random(minPartie, maxPartie + 1);
          DebutPatate      = TempsActuel;
          EtatActuel       = EN_JEU;
        break;
        case MINE:
          DebutMine             = TempsActuel;
          DernierDiamantTemps   = TempsActuel;
          DiamantActif          = false;
          CompteurDiamantsRouge = 0;
          CompteurDiamantsBleu  = 0;
          EnCaptureMineGauche   = false;
          EnCaptureMineDroite   = false;
          if (DelaiDebutMine > 0) {
            MineEnAttente    = true;
            DebutAttenteMine = TempsActuel;
          } else {
            MineEnAttente = false;
            if (BipMine) LancerBip(1000);
          }
          EtatActuel            = EN_JEU;
        break;
      }
    break;

    case MENU_SETTINGS:
      switch (ModeJeu) {
        case BOMBE:       PositionMenu = (PositionMenu < NbParamBomb)   ? PositionMenu + 1 : 1; break;
        case CTF:         PositionMenu = (PositionMenu < NbParamCTF)    ? PositionMenu + 1 : 1; break;
        case PATATE_CHAUDE: PositionMenu = (PositionMenu < NbParamPatate) ? PositionMenu + 1 : 1; break;
        case MINE:        PositionMenu = (PositionMenu < NbParamMine)   ? PositionMenu + 1 : 1; break;
      }
    break;

    case EN_JEU:
      if (ModeJeu == BOMBE) {
        if (BesoinCode) {
          IndexCode++;
          if (IndexCode >= TAILLE_CODE) {
            if (!BombeAmorcee) {
              for (int i = 0; i < TAILLE_CODE; i++) CodeSecret[i] = CodeSaisi[i];
              BombeAmorcee   = true;
              DebutAmorcage  = TempsActuel;
              TotalPenalites = 0;
              if (BipBomb) LancerBip(1000);
            } else {
              bool CodeEstBon = true;
              for (int i = 0; i < TAILLE_CODE; i++) {
                if (CodeSecret[i] != CodeSaisi[i]) { CodeEstBon = false; break; }
              }
              if (CodeEstBon) {
                BombeAmorcee     = false;
                if (!Reamorcable) EtatActuel = FIN_DE_JEU;
                TotalPenalites   = 0;
                statTempsRestant = TempsRestant;
              } else {
                TotalPenalites += PenaliteFauxCode * 1000UL;
              }
            }
            IndexCode = 0;
            for (int i = 0; i < TAILLE_CODE; i++) CodeSaisi[i] = 0;
          }
        } else if (!BombeAmorcee){
          BombeAmorcee = true;
          DebutAmorcage = TempsActuel;
          if (BipBomb) LancerBip(1000);
        }
      }
    break;

    case FIN_DE_JEU:
      if (ModeJeu == BOMBE) {
        if (Reamorcable){
          BombeAmorcee     = false;
          BombeExplosee    = false;
          TotalPenalites   = 0;
          DesamorcageEnCours = false;
          IndexCode        = 0;
          for (int i = 0; i < TAILLE_CODE; i++) CodeSaisi[i] = 0;
          EtatActuel       = EN_JEU;
        }
      }
    break;
  }
}

// ======================================================
// APPUI LONG – BOUTON OK
// ======================================================
void AppuiOKLong() {
  switch (EtatActuel) {
    case MENU_JEUX:
      EtatActuel = MENU_SETTINGS;
    break;
    case MENU_SETTINGS:
      EtatActuel = MENU_JEUX;
    break;
    case EN_JEU:
      if (ModeJeu == BOMBE && IndexCode > 0) IndexCode--;
    break;
    case FIN_DE_JEU:
      switch (ModeJeu) {
        case BOMBE:
          BombeExplosee = false;
        break;
        case CTF:
          statCTFBleu      = 0;
          statCTFRouge     = 0;
          CompteurBleu     = 0;
          CompteurRouge    = 0;
          EquipeEnControle = NEUTRE;
          CTFEnAttente     = false;
        break;
        case PATATE_CHAUDE:
        break;
        case MINE:
          statDiamantsRouge     = 0;
          statDiamantsBleu      = 0;
          CompteurDiamantsRouge = 0;
          CompteurDiamantsBleu  = 0;
          DiamantActif          = false;
          MineEnAttente         = false;
        break;
      }
      EtatActuel = MENU_JEUX;
    break;
  }
}

// ======================================================
// GESTION DU BUZZER
// ======================================================
void GererBuzzer() {
  // Bip ponctuel en cours : il passe avant tout le reste
  if (BipEvenement) {
    if (TempsActuel < DebutBipEvenement + DureeBipEvenement) {
      digitalWrite(Buzzer, HIGH);
      return;
    }
    BipEvenement = false;
    Bip          = false;
    FinBip       = TempsActuel;
  }

  switch (EtatActuel) {

    case EN_JEU:
      switch (ModeJeu) {

        case BOMBE:
          if (BombeAmorcee && !Explosion && BipBomb) {
            if      (TempsRestant >= 10L * 60 * 1000) { DureeBip = 500; IntervalleBip = 15000; }
            else if (TempsRestant >=  5L * 60 * 1000) { DureeBip = 500; IntervalleBip =  5000; }
            else { DureeBip = TempsRestant / 600UL; IntervalleBip = TempsRestant / 60UL; }
            if  (Bip && TempsActuel >= DebutBip + DureeBip)    { Bip = false; FinBip = TempsActuel; }
            else if (!Bip && TempsActuel >= FinBip + IntervalleBip) { Bip = true; DebutBip = TempsActuel; }
          } else if (Explosion) { Bip = true;  }
          else                   { Bip = false; }
        break;

        case CTF:
          Bip = false;
        break;

        case PATATE_CHAUDE:
          if (Explosion) {
            Bip = true;
          } else if (BipPatate) {
            if      (TempsRestant >= 10L * 60 * 1000) { DureeBip = 500; IntervalleBip = 30000; }
            else if (TempsRestant >=  5L * 60 * 1000) { DureeBip = 500; IntervalleBip = 10000; }
            else { DureeBip = TempsRestant / 600UL; IntervalleBip = TempsRestant / 60UL; } // /600 : 500 ms à 5 min, sans saut (comme la bombe)
            if  (Bip && TempsActuel >= DebutBip + DureeBip)    { Bip = false; FinBip = TempsActuel; }
            else if (!Bip && TempsActuel >= FinBip + IntervalleBip) { Bip = true; DebutBip = TempsActuel; }
          } else {
            Bip = false;
          }
        break;

        case MINE:
          Bip = false;
        break;
      }
    break;

    case FIN_DE_JEU:
      Bip = false;
    break;

    default:
      Bip = false;
    break;
  }

  digitalWrite(Buzzer, Bip ? HIGH : LOW);
}

// ======================================================
// COMMANDE DE L'ÉCRAN OLED
// ======================================================
void rafraichirEcran() {
  MonEcran.clearBuffer();
  char buffer[24];

  switch (EtatActuel) {

    // ────────────────────────────────────────────────────
    case MENU_JEUX:
      MonEcran.setFont(u8g2_font_6x12_tr);
      MonEcran.drawStr(0, 10, "SELECT. MODE :");
      MonEcran.setFont(u8g2_font_logisoso16_tr);
      if      (ModeJeu == BOMBE)         MonEcran.drawStr(0, 45, "BOMBE");
      else if (ModeJeu == CTF)           MonEcran.drawStr(0, 45, "C.T.F.");
      else if (ModeJeu == PATATE_CHAUDE) MonEcran.drawStr(0, 45, "P. CHAUDE");
      else if (ModeJeu == MINE)          MonEcran.drawStr(0, 45, "MINE");
      MonEcran.setFont(u8g2_font_6x10_tr);
      MonEcran.drawStr(0, 60, "> OK pour lancer");
    break;

    // ────────────────────────────────────────────────────
    case MENU_SETTINGS:
      MonEcran.setFont(u8g2_font_6x12_tr);

      switch (ModeJeu) {

        case BOMBE:
          MonEcran.drawStr(0, 10, "SETTINGS BOMBE");
          MonEcran.drawHLine(0, 13, 128);
          switch (PositionMenu) {
            case 1: MonEcran.drawStr(0,30,"Timer bombe:");
                    sprintf(buffer,"%d min",TimerBombe);
                    MonEcran.setFont(u8g2_font_7x14_tr); MonEcran.drawStr(20,50,buffer); break;
            case 2: MonEcran.drawStr(0,30,"Timer desamorcage:");
                    sprintf(buffer,"%d sec",TimerDesamorcage);
                    MonEcran.setFont(u8g2_font_7x14_tr); MonEcran.drawStr(20,50,buffer); break;
            case 3: MonEcran.drawStr(0,30,"Rearmable:");
                    MonEcran.setFont(u8g2_font_7x14_tr);
                    MonEcran.drawStr(20,50,Reamorcable?"OUI":"NON"); break;
            case 4: MonEcran.drawStr(0,30,"Penalite faux code:");
                    sprintf(buffer,"%d sec",PenaliteFauxCode);
                    MonEcran.setFont(u8g2_font_7x14_tr); MonEcran.drawStr(20,50,buffer); break;
            case 5: MonEcran.drawStr(0,30,"Bip bombe:");
                    MonEcran.setFont(u8g2_font_7x14_tr);
                    MonEcran.drawStr(20,50,BipBomb?"OUI":"NON"); break;
            case 6: MonEcran.drawStr(0,30,"Besoin d'un code?");
                    MonEcran.setFont(u8g2_font_7x14_tr);
                    MonEcran.drawStr(20,50,BesoinCode?"OUI":"NON"); break;
          }
          MonEcran.setFont(u8g2_font_6x12_tr);
          MonEcran.drawStr(0, 62, "< > pour changer");
          sprintf(buffer,"%d/%d",PositionMenu,NbParamBomb);
          MonEcran.setFont(u8g2_font_6x10_tr); MonEcran.drawStr(110,10,buffer);
        break;

        case CTF:
          MonEcran.drawStr(0,10,"SETTINGS CTF");
          MonEcran.drawHLine(0,13,128);
          switch (PositionMenu) {
            case 1: MonEcran.drawStr(0,30,"Timer partie:");
                    sprintf(buffer,"%d min",TimerCTF);
                    MonEcran.setFont(u8g2_font_7x14_tr); MonEcran.drawStr(20,50,buffer); break;
            case 2: MonEcran.drawStr(0,30,"Timer capture:");
                    sprintf(buffer,"%d sec",TimerCapture);
                    MonEcran.setFont(u8g2_font_7x14_tr); MonEcran.drawStr(20,50,buffer); break;
            case 3: MonEcran.drawStr(0,30,"Delai debut:");
                    sprintf(buffer,"%d min",DelaiDebutCTF);
                    MonEcran.setFont(u8g2_font_7x14_tr); MonEcran.drawStr(20,50,buffer); break;
          }
          MonEcran.setFont(u8g2_font_6x12_tr);
          MonEcran.drawStr(0, 62, "< > pour changer");
          sprintf(buffer,"%d/%d",PositionMenu,NbParamCTF);
          MonEcran.setFont(u8g2_font_6x10_tr); MonEcran.drawStr(110,10,buffer);
        break;

        case PATATE_CHAUDE:
          MonEcran.drawStr(0,10,"SETTINGS PATATE");
          MonEcran.drawHLine(0,13,128);
          switch (PositionMenu) {
            case 1: MonEcran.drawStr(0,30,"Timer partie:");
                    sprintf(buffer,"%d min",TimerPatate);
                    MonEcran.setFont(u8g2_font_7x14_tr); MonEcran.drawStr(20,50,buffer); break;
            case 2: MonEcran.drawStr(0,30,"Latitude random:");
                    sprintf(buffer,"+/- %d min",LatitudeRandom);
                    MonEcran.setFont(u8g2_font_7x14_tr); MonEcran.drawStr(20,50,buffer); break;
            case 3: MonEcran.drawStr(0,30,"Bip patate:");
                    MonEcran.setFont(u8g2_font_7x14_tr);
                    MonEcran.drawStr(20,50,BipPatate?"OUI":"NON"); break;
          }
          MonEcran.setFont(u8g2_font_6x12_tr);
          MonEcran.drawStr(0, 62, "< > pour changer");
          sprintf(buffer,"%d/%d",PositionMenu,NbParamPatate);
          MonEcran.setFont(u8g2_font_6x10_tr); MonEcran.drawStr(110,10,buffer);
        break;

        case MINE:
          MonEcran.drawStr(0,10,"SETTINGS MINE");
          MonEcran.drawHLine(0,13,128);
          switch (PositionMenu) {
            case 1: MonEcran.drawStr(0,30,"Timer partie:");
                    sprintf(buffer,"%d min",TimerMine);
                    MonEcran.setFont(u8g2_font_7x14_tr); MonEcran.drawStr(20,50,buffer); break;
            case 2: MonEcran.drawStr(0,30,"Interv. diamant:");
                    sprintf(buffer,"%d min",IntervalleApparitionDiamant);
                    MonEcran.setFont(u8g2_font_7x14_tr); MonEcran.drawStr(20,50,buffer); break;
            case 3: MonEcran.drawStr(0,30,"Timer capture:");
                    sprintf(buffer,"%d sec",TimerCaptureMine);
                    MonEcran.setFont(u8g2_font_7x14_tr); MonEcran.drawStr(20,50,buffer); break;
			case 4: MonEcran.drawStr(0,30,"Delai debut:");
					sprintf(buffer,"%d min",DelaiDebutMine);
					MonEcran.setFont(u8g2_font_7x14_tr); MonEcran.drawStr(20,50,buffer); break;
            case 5: MonEcran.drawStr(0,30,"Bip mine:");
                    MonEcran.setFont(u8g2_font_7x14_tr);
                    MonEcran.drawStr(20,50,BipMine?"OUI":"NON"); break;
          }
          MonEcran.setFont(u8g2_font_6x12_tr);
          MonEcran.drawStr(0, 62, "< > pour changer");
          sprintf(buffer,"%d/%d",PositionMenu,NbParamMine);
          MonEcran.setFont(u8g2_font_6x10_tr); MonEcran.drawStr(110,10,buffer);
        break;
      }
    break;

    // ────────────────────────────────────────────────────
    case EN_JEU:
      switch (ModeJeu) {

        case BOMBE:
          if (Explosion) {
            MonEcran.setFont(u8g2_font_logisoso32_tr);
            MonEcran.drawStr(15, 45, "BOOM");
          } else if (!BombeAmorcee && BesoinCode) {
            MonEcran.setFont(u8g2_font_6x12_tr);
            MonEcran.drawStr(0, 15, "CODE ARMEMENT:");
            for (int i = 0; i < TAILLE_CODE; i++) {
              sprintf(buffer, "%d", CodeSaisi[i]);
              MonEcran.setFont(u8g2_font_logisoso16_tr);
              MonEcran.drawStr(30 + i * 20, 45, buffer);
              if (i == IndexCode) MonEcran.drawHLine(30 + i * 20, 48, 12);
            }
          } else if (!BombeAmorcee && !BesoinCode) {
            MonEcran.setFont(u8g2_font_6x12_tr);
            MonEcran.drawStr(0, 15, "BOMBE A ARMER");
            MonEcran.drawStr(0, 62, "OK pour armer");
          } else if (BesoinCode) {
            long minutes = TempsRestant / 60000;
            long secondes = (TempsRestant % 60000) / 1000;
            sprintf(buffer, "%02ld:%02ld", minutes, secondes);
            MonEcran.setFont(u8g2_font_logisoso32_tr);
            MonEcran.drawStr(10, 45, buffer);
            MonEcran.setFont(u8g2_font_6x12_tr);
            for (int i = 0; i < TAILLE_CODE; i++) {
              sprintf(buffer, "%d", CodeSaisi[i]);
              MonEcran.drawStr(45 + i * 10, 60, buffer);
              if (i == IndexCode) MonEcran.drawHLine(45 + i * 10, 62, 7);
            }
          } else {
            long minutes = TempsRestant / 60000;
            long secondes = (TempsRestant % 60000) / 1000;
            sprintf(buffer, "%02ld:%02ld", minutes, secondes);
            MonEcran.setFont(u8g2_font_logisoso32_tr);
            MonEcran.drawStr(10, 45, buffer);
            MonEcran.setFont(u8g2_font_6x12_tr);
            MonEcran.drawStr(0, 62, DesamorcageEnCours ? "Desamorcage..." : "Maintenir OK");
          }
        break;

        case CTF:
          if (CTFEnAttente) {
            MonEcran.setFont(u8g2_font_6x12_tr);
            MonEcran.drawStr(10, 20, "DEBUT DANS :");
            long secAvant = (TempsRestant + 999L) / 1000L;
            if (secAvant < 0) secAvant = 0;
            sprintf(buffer, "%ld", secAvant);
            MonEcran.setFont(u8g2_font_logisoso32_tr);
            int larg = MonEcran.getStrWidth(buffer);
            MonEcran.drawStr((128 - larg) / 2, 56, buffer);
          } else {
            MonEcran.setFont(u8g2_font_6x12_tr);
            secRestantes = TempsRestant / 1000;
            sprintf(buffer, "Tps: %ld s", secRestantes);
            MonEcran.drawStr(0, 10, buffer);
            if      (EnCaptureGauche) MonEcran.drawStr(0, 30, "Capture ROUGE...");
            else if (EnCaptureDroite) MonEcran.drawStr(0, 30, "Capture BLEU...");
            MonEcran.setFont(u8g2_font_7x14_tr);
            if      (EquipeEnControle == ROUGE) MonEcran.drawStr(0, 50, "ZONE: ROUGE");
            else if (EquipeEnControle == BLEU)  MonEcran.drawStr(0, 50, "ZONE: BLEU");
            else                               MonEcran.drawStr(0, 50, "ZONE: NEUTRE");
          }
        break;

        case PATATE_CHAUDE:
          if (Explosion) {
            MonEcran.setFont(u8g2_font_logisoso32_tr);
            MonEcran.drawStr(15, 45, "BOOM");
          } else {
            MonEcran.setFont(u8g2_font_logisoso16_tr);
            MonEcran.drawStr(10, 40, "SURVIE...");
          }
        break;

        case MINE: {
          if (MineEnAttente) {
            MonEcran.setFont(u8g2_font_6x12_tr);
            MonEcran.drawStr(10, 20, "DEBUT DANS :");
            long secAvantM = (TempsRestant + 999L) / 1000L;
            if (secAvantM < 0) secAvantM = 0;
            sprintf(buffer, "%ld", secAvantM);
            MonEcran.setFont(u8g2_font_logisoso32_tr);
            int largM = MonEcran.getStrWidth(buffer);
            MonEcran.drawStr((128 - largM) / 2, 56, buffer);
            break;
          }
          MonEcran.setFont(u8g2_font_6x12_tr);
          long mM = TempsRestant / 60000;
          long sM = (TempsRestant % 60000) / 1000;
          sprintf(buffer, "%02ld:%02ld", mM, sM);
          MonEcran.drawStr(0, 11, buffer);
          sprintf(buffer, "R:%d  B:%d", CompteurDiamantsRouge, CompteurDiamantsBleu);
          MonEcran.drawStr(70, 11, buffer);
          MonEcran.drawHLine(0, 14, 128);

          if (DiamantActif) {
            if (EnCaptureMineGauche || EnCaptureMineDroite) {
              unsigned long elapsed = TempsActuel - DebutCaptureMine;
              unsigned long total   = (unsigned long)TimerCaptureMine * 1000UL;
              int prog = (int)min((unsigned long)128, (elapsed * 128UL) / total);
              MonEcran.setFont(u8g2_font_7x14_tr);
              MonEcran.drawStr(0, 35, EnCaptureMineGauche ? "< ROUGE" : "BLEU >");
              MonEcran.drawFrame(0, 43, 128, 10);
              MonEcran.drawBox(0, 43, prog, 10);
              MonEcran.setFont(u8g2_font_6x12_tr);
              sprintf(buffer, "%d / %d sec", (int)(elapsed / 1000), TimerCaptureMine);
              MonEcran.drawStr(28, 63, buffer);
            } else {
              MonEcran.setFont(u8g2_font_7x14_tr);
              MonEcran.drawStr(8, 37, "** DIAMANT **");
              MonEcran.setFont(u8g2_font_6x12_tr);
              MonEcran.drawStr(3, 55, "< ROUGE   BLEU >");
            }
          } else {
            long prochainSec = ((long)(DernierDiamantTemps + (unsigned long)IntervalleApparitionDiamant * 60UL * 1000UL)
                                - (long)TempsActuel) / 1000L;
            if (prochainSec < 0) prochainSec = 0;
            MonEcran.setFont(u8g2_font_6x12_tr);
            MonEcran.drawStr(0, 35, "Prochain diamant:");
            sprintf(buffer, "%ld sec", prochainSec);
            MonEcran.setFont(u8g2_font_7x14_tr);
            MonEcran.drawStr(25, 56, buffer);
          }
        }
        break;
      }
    break;

    // ────────────────────────────────────────────────────
    case FIN_DE_JEU:

      switch (ModeJeu) {

        case BOMBE:
          if (BombeExplosee) {
            // --- Bombe explosée : vraie fin de partie ---
            MonEcran.setFont(u8g2_font_6x12_tr);
            MonEcran.drawStr(20, 12, "FIN DE PARTIE");
            MonEcran.drawHLine(0, 15, 128);
            MonEcran.setFont(u8g2_font_logisoso32_tr);
            MonEcran.drawStr(15, 48, "BOOM");
            MonEcran.setFont(u8g2_font_6x12_tr);
            MonEcran.drawStr(20, 62, "Bombe explosee !");
          } else if (Reamorcable) {
            // --- Désamorcée + réamorçable : prête à repartir ---
            MonEcran.setFont(u8g2_font_6x12_tr);
            MonEcran.drawStr(15, 12, "BOMBE DESAMORCEE");
            MonEcran.drawHLine(0, 15, 128);

            long minR = statTempsRestant / 60000;
            long secR = (statTempsRestant % 60000) / 1000;
            MonEcran.drawStr(5, 30, "Temps restant:");
            MonEcran.setFont(u8g2_font_7x14_tr);
            sprintf(buffer, "%02ld:%02ld", minR, secR);
            MonEcran.drawStr(25, 46, buffer);

            MonEcran.setFont(u8g2_font_6x10_tr);
            MonEcran.drawStr(8, 60, "OK: reamorcer");
          } else {
            // --- Désamorcée non réamorçable : fin de partie ---
            MonEcran.setFont(u8g2_font_6x12_tr);
            MonEcran.drawStr(20, 12, "FIN DE PARTIE");
            MonEcran.drawHLine(0, 15, 128);

            long minR = statTempsRestant / 60000;
            long secR = (statTempsRestant % 60000) / 1000;
            MonEcran.drawStr(15, 32, "Desamorcee !");
            MonEcran.drawStr(5,  46, "Temps restant:");
            MonEcran.setFont(u8g2_font_7x14_tr);
            sprintf(buffer, "%02ld:%02ld", minR, secR);
            MonEcran.drawStr(25, 62, buffer);
          }
        break;

        case CTF: {
          int total = statCTFRouge + statCTFBleu;
          int largR = (total > 0) ? map(statCTFRouge, 0, total, 0, 128) : 64;
          MonEcran.setFont(u8g2_font_6x12_tr);
          MonEcran.drawStr(20, 12, "FIN DE PARTIE");
          MonEcran.drawHLine(0, 15, 128);
          MonEcran.drawBox(0, 17, largR, 8);
          MonEcran.drawFrame(largR, 17, 128 - largR, 8);
          MonEcran.setFont(u8g2_font_6x12_tr);
          sprintf(buffer, "RGE: %ds", statCTFRouge); MonEcran.drawStr(0, 38, buffer);
          sprintf(buffer, "BLU: %ds", statCTFBleu);  MonEcran.drawStr(0, 52, buffer);
          MonEcran.setFont(u8g2_font_7x14_tr);
          if      (statCTFRouge > statCTFBleu) MonEcran.drawStr(70, 38, ">> ROUGE");
          else if (statCTFBleu > statCTFRouge) MonEcran.drawStr(70, 52, ">> BLEU");
          else                                 MonEcran.drawStr(65, 45, "EGALITE");
        }
        break;

        case PATATE_CHAUDE:
          MonEcran.setFont(u8g2_font_6x12_tr);
          MonEcran.drawStr(20, 12, "FIN DE PARTIE");
          MonEcran.drawHLine(0, 15, 128);
          MonEcran.setFont(u8g2_font_logisoso16_tr);
          MonEcran.drawStr(25, 40, "BOOM !");
          MonEcran.setFont(u8g2_font_6x12_tr);
          MonEcran.drawStr(15, 58, "Fin d'explosion");
        break;

        case MINE: {
          MonEcran.setFont(u8g2_font_6x12_tr);
          MonEcran.drawStr(20, 12, "FIN DE PARTIE");
          MonEcran.drawHLine(0, 15, 128);
          int totalD = statDiamantsRouge + statDiamantsBleu;
          int largR  = (totalD > 0) ? map(statDiamantsRouge, 0, totalD, 0, 128) : 64;
          MonEcran.drawBox(0, 17, largR, 8);
          MonEcran.drawFrame(largR, 17, 128 - largR, 8);
          MonEcran.setFont(u8g2_font_6x12_tr);
          sprintf(buffer, "RGE: %d", statDiamantsRouge); MonEcran.drawStr(0, 38, buffer);
          sprintf(buffer, "BLU: %d", statDiamantsBleu);  MonEcran.drawStr(0, 52, buffer);
          MonEcran.setFont(u8g2_font_7x14_tr);
          if      (statDiamantsRouge > statDiamantsBleu) MonEcran.drawStr(65, 38, ">> ROUGE");
          else if (statDiamantsBleu > statDiamantsRouge) MonEcran.drawStr(65, 52, ">> BLEU");
          else                                           MonEcran.drawStr(65, 45, "EGALITE");
        }
        break;
      }
    break;
  }

  MonEcran.sendBuffer();
}

// ======================================================
// COMMANDE DU BANDEAU LED
// ======================================================
// Vérifie si un événement de jeu nécessite de changer les couleurs
void DetecterChangementsLeds() {
  bool aChange = false;

  if (EtatActuel != memoEtatActuel) { aChange = true; memoEtatActuel = EtatActuel; }
  if (ModeJeu != memoModeJeu) { aChange = true; memoModeJeu = ModeJeu; }
  if (Explosion != memoExplosion) { aChange = true; memoExplosion = Explosion; }
  if (EnCaptureGauche != memoEnCaptureGauche) { aChange = true; memoEnCaptureGauche = EnCaptureGauche; }
  if (EnCaptureDroite != memoEnCaptureDroite) { aChange = true; memoEnCaptureDroite = EnCaptureDroite; }
  if (EquipeEnControle != memoEquipeEnControle) { aChange = true; memoEquipeEnControle = EquipeEnControle; }
  if (DiamantActif != memoDiamantActif) { aChange = true; memoDiamantActif = DiamantActif; }
  if (EnCaptureMineGauche != memoEnCaptureMineGauche) { aChange = true; memoEnCaptureMineGauche = EnCaptureMineGauche; }
  if (EnCaptureMineDroite != memoEnCaptureMineDroite) { aChange = true; memoEnCaptureMineDroite = EnCaptureMineDroite; }
  if (CompteurDiamantsRouge != memoCompteurDiamantsRouge) { aChange = true; memoCompteurDiamantsRouge = CompteurDiamantsRouge; }
  if (CompteurDiamantsBleu != memoCompteurDiamantsBleu) { aChange = true; memoCompteurDiamantsBleu = CompteurDiamantsBleu; }

  if (aChange) {
    majLeds = true;
  }
}

// S'occupe de faire clignoter les LEDs sans bloquer l'affichage
void GererClignotementsLeds() {
  if (EtatActuel == EN_JEU) {
    // Clignotement BOMBE
    if (ModeJeu == BOMBE && Explosion) {
      if (TempsActuel - DernierClignotement >= 200) {
        DernierClignotement = TempsActuel;
        EtatClignotement = !EtatClignotement;
        majLeds = true; // Déclenche un rafraîchissement
      }
    }
    // Clignotement CTF (Capture en cours)
    else if (ModeJeu == CTF && (EnCaptureGauche || EnCaptureDroite)) {
      if (TempsActuel - DernierClignotement >= 250) {
        DernierClignotement = TempsActuel;
        EtatClignotement = !EtatClignotement;
        majLeds = true;
      }
    }
    // Clignotement PATATE_CHAUDE
    else if (ModeJeu == PATATE_CHAUDE && Explosion) {
      if (TempsActuel - DernierClignotement >= 200) {
        DernierClignotement = TempsActuel;
        EtatClignotement = !EtatClignotement;
        majLeds = true;
      }
    }
    // Clignotement MINE (Diamant dispo, personne ne capture)
    else if (ModeJeu == MINE && DiamantActif && !EnCaptureMineGauche && !EnCaptureMineDroite) {
      if (TempsActuel - DernierClignotement >= 300) {
        DernierClignotement = TempsActuel;
        EtatClignotement = !EtatClignotement;
        majLeds = true;
      }
    }
  }
}

void Leds() {
  switch (EtatActuel) {

    case MENU_JEUX:
    case MENU_SETTINGS:
      BandeLed.clear();
      BandeLed.show();
    break;

    case EN_JEU:
      switch (ModeJeu) {

        case BOMBE:
          if (Explosion) {
            for (int i = 0; i < NUM_PIXELS; i++)
              BandeLed.setPixelColor(i, EtatClignotement ? BandeLed.Color(50,0,0) : BandeLed.Color(0,0,0));
          } else {
            BandeLed.clear();
          }
          BandeLed.show();
        break;

        case CTF:
          if (EnCaptureGauche || EnCaptureDroite) {
            // 1. Définition de la couleur de BASE (Contrôle actuel)
            uint32_t couleurBase;
            if      (EquipeEnControle == ROUGE) couleurBase = BandeLed.Color(50, 0, 0);
            else if (EquipeEnControle == BLEU)  couleurBase = BandeLed.Color(0, 0, 50);
            else                                couleurBase = BandeLed.Color(50, 50, 50); // Blanc (Neutre)

            // 2. Définition de la couleur de CAPTURE
            uint32_t couleurCapture;
            if (EnCaptureGauche) couleurCapture = BandeLed.Color(50, 0, 0);
            else                 couleurCapture = BandeLed.Color(0, 0, 50);

            // 3. Logique d'affichage
            // On ne clignote QUE si la couleur de capture est différente de la couleur de base
            if (couleurCapture != couleurBase) {
                for (int i = 0; i < NUM_PIXELS; i++) {
                    BandeLed.setPixelColor(i, EtatClignotement ? couleurCapture : couleurBase);
                }
            } else {
                // Même équipe : on reste fixe sur la couleur de base
                for (int i = 0; i < NUM_PIXELS; i++) {
                    BandeLed.setPixelColor(i, couleurBase);
                }
            }
          } 
          else {
            // --- Personne ne touche aux boutons ---
            if      (EquipeEnControle == ROUGE)  for(int i=0;i<NUM_PIXELS;i++) BandeLed.setPixelColor(i, BandeLed.Color(50,0,0));
            else if (EquipeEnControle == BLEU)   for(int i=0;i<NUM_PIXELS;i++) BandeLed.setPixelColor(i, BandeLed.Color(0,0,50));
            else                                 for(int i=0;i<NUM_PIXELS;i++) BandeLed.setPixelColor(i, BandeLed.Color(50,50,50));
          }
          BandeLed.show();
        break;

        case PATATE_CHAUDE:
          if (Explosion) {
            for (int i = 0; i < NUM_PIXELS; i++)
              BandeLed.setPixelColor(i, EtatClignotement ? BandeLed.Color(50,50,0) : BandeLed.Color(0,0,0));
          } else {
            BandeLed.clear();
          }
          BandeLed.show();
        break;

        case MINE:
          if (DiamantActif) {
            if (EnCaptureMineGauche) {
              for (int i = 0; i < NUM_PIXELS; i++) BandeLed.setPixelColor(i, BandeLed.Color(50,0,0));
            } else if (EnCaptureMineDroite) {
              for (int i = 0; i < NUM_PIXELS; i++) BandeLed.setPixelColor(i, BandeLed.Color(0,0,50));
            } else {
              for (int i = 0; i < NUM_PIXELS; i++)
                BandeLed.setPixelColor(i, EtatClignotement ? BandeLed.Color(50,50,50) : BandeLed.Color(0,0,0));
            }
          } else {
            int totalD = CompteurDiamantsRouge + CompteurDiamantsBleu;
            if (totalD > 0) {
              int ledsR = map(CompteurDiamantsRouge, 0, totalD, 0, NUM_PIXELS);
              for (int i = 0; i < NUM_PIXELS; i++)
                BandeLed.setPixelColor(i, (i < ledsR) ? BandeLed.Color(50,0,0) : BandeLed.Color(0,0,50));
            } else {
              BandeLed.clear();
            }
          }
          BandeLed.show();
        break;
      }
    break;

    case FIN_DE_JEU:
      BandeLed.clear();
      BandeLed.show();
    break;
  }
}
