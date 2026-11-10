// Test des micro-coupures de boutons : on maintient un bouton en simulant de très courtes coupures du contact
// (faux contact, soudure fragile) et on vérifie que la capture va quand même au bout.
// La vraie bibliothèque EasyButton 2.0.3 est utilisée (simulateur/vendor/EasyButton), pas une imitation.
#include <string>
#include <cstdio>
unsigned long g_ms=0; std::string g_screen; int g_pins[64];
#include SRC

const int PERIODE = 25; // durée estimée d'un tour de loop() sur la bombe (écran en SPI logiciel), en ms
const int GAUCHE = 22, DROITE = 26, OK = 30; // broches des boutons ; bouton appuyé = broche à LOW
int erreurs = 0;

void tour(){ loop(); g_ms += PERIODE; }
void attendre(unsigned long ms){ unsigned long fin = g_ms + ms; while (g_ms < fin) tour(); }
void appuiCourt(int pin){ g_pins[pin] = LOW; attendre(100); g_pins[pin] = HIGH; attendre(100); }
void appuiLong(int pin){ g_pins[pin] = LOW; attendre(2500); g_pins[pin] = HIGH; attendre(100); }
// Maintient le bouton 'ms' millisecondes, avec une coupure de 'coupure' ms toutes les 'tous' ms (0 = pas de coupure)
void maintenir(int pin, unsigned long ms, unsigned long coupure = 0, unsigned long tous = 0){
  unsigned long debut = g_ms;
  while (g_ms < debut + ms) {
    unsigned long t = g_ms - debut;
    g_pins[pin] = (tous && t > 0 && t % tous < coupure) ? HIGH : LOW;
    tour();
  }
  g_pins[pin] = HIGH;
}
void verifier(const char* scenario, long obtenu, long attendu){
  bool bon = (obtenu == attendu); if (!bon) erreurs++;
  printf("%s %s (obtenu %ld, attendu %ld)\n", bon ? "OK    " : "ECHEC ", scenario, obtenu, attendu);
}
const char* zone(){ return EquipeEnControle == ROUGE ? "ROUGE" : EquipeEnControle == BLEU ? "BLEU" : "NEUTRE"; }

int main(){
  for (int i = 0; i < 64; i++) g_pins[i] = HIGH;
  setup();

  // ── CTF : capture 20 s ──
  ModeJeu = CTF; TimerCTF = 10; TimerCapture = 20; DelaiDebutCTF = 0; appuiCourt(OK);
  maintenir(GAUCHE, 21000, 50, 4000);
  verifier("CTF : ROUGE maintient 21 s avec une coupure de 50 ms toutes les 4 s -> zone ROUGE", EquipeEnControle, ROUGE);
  attendre(1000);
  maintenir(DROITE, 10000); attendre(1000); maintenir(DROITE, 10000); attendre(1000);
  verifier("CTF : BLEU maintient 10 s, lache vraiment 1 s, puis 10 s -> zone reste ROUGE", EquipeEnControle, ROUGE);
  maintenir(DROITE, 21000);
  verifier("CTF : BLEU maintient 21 s d'affilee -> zone NEUTRE", EquipeEnControle, NEUTRE);
  appuiLong(OK); appuiLong(OK);

  // ── MINE : capture 15 s ──
  ModeJeu = MINE; TimerMine = 10; IntervalleApparitionDiamant = 1; TimerCaptureMine = 15; DelaiDebutMine = 0; appuiCourt(OK);
  attendre(61000);
  maintenir(GAUCHE, 7000); attendre(1000); maintenir(GAUCHE, 9000); attendre(1000);
  verifier("MINE : ROUGE maintient 7 s, lache vraiment 1 s, puis 9 s -> pas de diamant", CompteurDiamantsRouge, 0);
  maintenir(GAUCHE, 16000, 40, 3000);
  verifier("MINE : ROUGE maintient 16 s avec une coupure de 40 ms toutes les 3 s -> 1 diamant", CompteurDiamantsRouge, 1);
  appuiLong(OK); appuiLong(OK);

  // ── BOMBE sans code : désamorçage 30 s ──
  ModeJeu = BOMBE; BesoinCode = false; TimerBombe = 10; TimerDesamorcage = 30; appuiCourt(OK); appuiCourt(OK);
  attendre(32000);
  verifier("BOMBE : personne ne touche pendant 32 s -> toujours en jeu", EtatActuel, EN_JEU);
  maintenir(OK, 27000); attendre(1000); maintenir(OK, 5000); attendre(1000);
  verifier("BOMBE : OK maintenu 27 s, vrai lacher 1 s, puis 5 s -> toujours en jeu", EtatActuel, EN_JEU);
  maintenir(OK, 30100, 40, 3000); attendre(300);
  verifier("BOMBE : OK maintenu 30 s avec une coupure de 40 ms toutes les 3 s -> desamorcee", EtatActuel, FIN_DE_JEU);

  printf(erreurs ? "\n%d scenario(s) en echec\n" : "\nTous les scenarios sont OK\n", erreurs);
  return erreurs ? 1 : 0;
}
