#include <string>
#include <cstdio>
#include <vector>
unsigned long g_ms=0; std::string g_screen; int g_pins[64];
#include SRC
int lastBz=0; unsigned long bzStart=0; std::vector<std::string> bips;
void tick(){ loop(); int b=g_pins[32]; if(b&&!lastBz) bzStart=g_ms; if(!b&&lastBz){ char t[64]; sprintf(t,"bip@%.1fs(%lums)", bzStart/1000.0, g_ms-bzStart); bips.push_back(t);} lastBz=b; g_ms+=10; }
void run(unsigned long until){ while(g_ms<until) tick(); }
void ok(){ BPOK.sim=true; run(g_ms+100); BPOK.sim=false; run(g_ms+100); }
void okLong(){ BPOK.sim=true; run(g_ms+2500); BPOK.sim=false; run(g_ms+100); }
void dump(const char*t){ printf("  %s bips: ",t); for(auto&b:bips) printf("%s ",b.c_str()); printf("\n"); bips.clear(); }
int main(){
  setup(); unsigned long t0;
  // CTF avec délai 1 min, partie 5 min
  ModeJeu=CTF; TimerCTF=5; TimerCapture=20; DelaiDebutCTF=1; ok(); t0=g_ms;
  printf("t0=%.1fs\n",t0/1000.0);
  run(t0+30000); tick(); printf("CTF attente écran: %s\n", g_screen.c_str());
  run(t0+60000+1000); t0=g_ms;
  BPGauche.sim=true; run(t0+21000); BPGauche.sim=false;  // R @20
  run(t0+100000); BPDroite.sim=true; run(t0+121000); BPDroite.sim=false; // N @120
  BPGauche.sim=true; run(t0+142000); BPGauche.sim=false; // R @141
  run(t0+250000); BPDroite.sim=true; run(t0+271000); BPDroite.sim=false; // N @270
  run(t0+305000);
  printf("CTF: ROUGE=%ds (attendu ~228) BLEU=%ds etat=%d\n", statCTFRouge, statCTFBleu, EtatActuel); dump("CTF (attendu départ ~61s 1000ms, fin 2000ms)");
  okLong();
  // CTF sans délai : bips de départ et de fin toujours actifs
  DelaiDebutCTF=0; ok(); run(g_ms+5*60000+2000); dump("CTF sans délai (attendu départ 1000 + fin 2000)"); okLong();
  // MINE délai 1 min, partie 10 min, diamant toutes les 1 min
  ModeJeu=MINE; TimerMine=10; IntervalleApparitionDiamant=1; TimerCaptureMine=15; DelaiDebutMine=1; ok(); t0=g_ms;
  run(t0+30000); tick(); printf("MINE attente: MineEnAttente=%d écran=%s\n", MineEnAttente, g_screen.c_str());
  run(t0+60000+500); t0=g_ms; printf("MINE démarrée: MineEnAttente=%d DiamantActif=%d\n", MineEnAttente, DiamantActif);
  run(t0+61000); printf("diamant à +61s: %d\n", DiamantActif);
  BPGauche.sim=true; run(g_ms+1000); BPGauche.sim=false; run(g_ms+20000);
  printf("appui 1 s: rouge=%d (attendu 0) diamant actif=%d\n", CompteurDiamantsRouge, DiamantActif);
  BPDroite.sim=true; run(g_ms+16000); BPDroite.sim=false;
  printf("appui 16 s bleu: bleu=%d (attendu 1)\n", CompteurDiamantsBleu);
  run(t0+10*60000UL+2000);
  printf("MINE fin: etat=%d rouge=%d bleu=%d\n", EtatActuel, statDiamantsRouge, statDiamantsBleu); dump("MINE (départ 1000, diamants 300, fin 2000)");
  okLong();
  // BOMBE sans code
  ModeJeu=BOMBE; BesoinCode=false; TimerBombe=5; BipBomb=true; ok(); ok(); t0=g_ms;
  run(t0+20000); dump("BOMBE 20 premières s (armement 1000 puis bips 500 /5s)");
  run(t0+5*60000UL+6000); tick();
  printf("BOMBE 6 s après BOOM: etat=%d écran=%s\n", EtatActuel, g_screen.c_str()); bips.clear();
  ok(); printf("Réamorçable=NON, OK en fin: etat=%d\n", EtatActuel); okLong(); printf("appui long: etat=%d (0=menu)\n", EtatActuel);
  // BOMBE avec code
  BesoinCode=true; BipBomb=false; ok(); for(int i=0;i<4;i++) ok(); t0=g_ms; run(t0+3000);
  printf("BOMBE code: armée=%d\n", BombeAmorcee); dump("BOMBE code BipBomb=NON (attendu aucun)");
  EtatActuel=MENU_JEUX; BombeAmorcee=false;
  // PATATE
  ModeJeu=PATATE_CHAUDE; TimerPatate=5; LatitudeRandom=15;
  for(int i=0;i<8;i++){ ok(); printf("%lu ", DureeCiblePatate/60000); EtatActuel=MENU_JEUX; } printf(" <- PATATE 5±15 durées (min), attendu 1..20\n");
  bips.clear(); ModeJeu=MINE; BipMine=false; IntervalleApparitionDiamant=1; DelaiDebutMine=0; TimerMine=5; ok(); run(g_ms+5*60000+3000); dump("MINE BipMine=NON (attendu aucun)"); okLong(); BipMine=true;
  ModeJeu=CTF; EtatActuel=MENU_SETTINGS; PositionMenu=3; tick(); printf("Ecran settings CTF 3 : %s\n", g_screen.c_str());
  ModeJeu=MINE; EtatActuel=MENU_SETTINGS; PositionMenu=5; tick(); printf("Ecran settings MINE 5 : %s\n", g_screen.c_str());
  ModeJeu=MINE; EtatActuel=MENU_SETTINGS; PositionMenu=1; tick(); printf("Ecran settings MINE : %s\n", g_screen.c_str());
  ModeJeu=BOMBE; tick(); printf("Ecran settings BOMBE : %s\n", g_screen.c_str());
}
