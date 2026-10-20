#pragma once
#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <algorithm>
typedef bool boolean;
extern unsigned long g_ms;
inline unsigned long millis(){return g_ms;}
inline void delay(unsigned long){}
#define OUTPUT 1
#define INPUT 0
#define HIGH 1
#define LOW 0
inline void pinMode(int,int){}
extern int g_pins[64];
inline void digitalWrite(int p,int v){g_pins[p]=v;}
#define A0 54
inline int analogRead(int){return 123;}
inline void randomSeed(unsigned long s){std::srand(s);}
inline long random(long a,long b){ if(a>=b) return a; return a + (std::rand() % (b-a)); }
template<class A,class B> auto min(A a,B b)->decltype(a+b){return a<b?a:b;}
template<class A,class B> auto max(A a,B b)->decltype(a+b){return a>b?a:b;}
inline long map(long x,long a,long b,long c,long d){return (x-a)*(d-c)/(b-a)+c;}
struct SerialC{void begin(long){} } ;
static SerialC Serial;
