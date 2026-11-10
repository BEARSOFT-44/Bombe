#pragma once
// Fausse bibliothèque Arduino pour PC : le temps (g_ms) et les broches (g_pins) sont simulés par le test.
#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <cstddef>
typedef bool boolean;
extern unsigned long g_ms;
inline unsigned long millis(){return g_ms;}
inline void delay(unsigned long){}
#define OUTPUT 1
#define INPUT 0
#define INPUT_PULLUP 2
#define HIGH 1
#define LOW 0
#define CHANGE 1
#define NOT_AN_INTERRUPT -1
inline int digitalPinToInterrupt(int){return -1;}
inline void attachInterrupt(int, void(*)(), int){}
inline void detachInterrupt(int){}
inline void pinMode(int,int){}
extern int g_pins[64];
inline void digitalWrite(int p,int v){g_pins[p]=v;}
inline int digitalRead(int p){return g_pins[p];}
#define A0 54
inline int analogRead(int){return 123;}
inline void randomSeed(unsigned long s){std::srand(s);}
inline long random(long a,long b){ if(a>=b) return a; return a + (std::rand() % (b-a)); }
template<class A,class B> auto min(A a,B b)->decltype(a+b){return a<b?a:b;}
template<class A,class B> auto max(A a,B b)->decltype(a+b){return a>b?a:b;}
inline long map(long x,long a,long b,long c,long d){return (x-a)*(d-c)/(b-a)+c;}
struct SerialC{void begin(long){} } ;
static SerialC Serial;
