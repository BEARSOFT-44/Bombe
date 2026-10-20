#pragma once
#include "Arduino.h"
// mock fidèle au comportement EasyButton 2.0.3 : onPressed au relâchement, sauf si maintenu >= seuil onPressedFor
struct EasyButton{ typedef void(*cb)(); int pin; bool sim=false, cur=false, held=false, heldCalled=false; unsigned long since=0, thr=0; cb p=nullptr, l=nullptr;
 EasyButton(int pi):pin(pi){} void begin(){} void onPressed(cb c){p=c;} void onPressedFor(unsigned long d,cb c){thr=d;l=c;}
 bool isPressed(){return cur;}
 bool read(){ bool prev=cur; cur=sim; if(cur&&!prev) since=millis();
   if(!cur&&prev){ if(!held){ if(p)p(); } else held=false; heldCalled=false; }
   else if(cur && l && millis()-since>=thr){ held=true; if(!heldCalled){heldCalled=true; l();} }
   return cur; } };
