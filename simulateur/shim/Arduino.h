// Faux Arduino (Mega 2560) pour la simulation WebAssembly. Le temps et les broches sont pilotes par la page web.
#pragma once
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#ifdef __cplusplus
extern "C" {
#endif
extern volatile uint32_t sim_ms;          // millis() simule
extern uint8_t sim_pins[80];              // niveau de sortie des broches (digitalWrite)
extern uint8_t sim_inputs[80];            // niveau lu par digitalRead (1 = HIGH)
extern uint8_t sim_pinmode[80];
#ifdef __cplusplus
}
#endif
typedef bool boolean;
typedef uint8_t byte;
typedef unsigned int word;
#define HIGH 1
#define LOW 0
#define INPUT 0
#define OUTPUT 1
#define INPUT_PULLUP 2
#define CHANGE 1
#define FALLING 2
#define RISING 3
#define NOT_AN_INTERRUPT -1
#define A0 54
#define A1 55
#define A2 56
#define A3 57
#define LED_BUILTIN 13
#define PROGMEM
#define F(s) (s)
#define pgm_read_byte(p) (*(const uint8_t*)(p))
#define digitalPinToInterrupt(p) (NOT_AN_INTERRUPT)
#define min(a,b) ((a)<(b)?(a):(b))
#define max(a,b) ((a)>(b)?(a):(b))
#define constrain(amt,low,high) ((amt)<(low)?(low):((amt)>(high)?(high):(amt)))
#define sq(x) ((x)*(x))
inline unsigned long millis() { return sim_ms; }
inline unsigned long micros() { return sim_ms * 1000UL; }
inline void delay(unsigned long ms) { sim_ms += ms; }   // la page affiche l'ecran fige pendant ce temps
inline void delayMicroseconds(unsigned int) {}
inline void pinMode(uint8_t p, uint8_t m) { if (p < 80) { sim_pinmode[p] = m; } }
inline void digitalWrite(uint8_t p, uint8_t v) { if (p < 80) sim_pins[p] = v ? 1 : 0; }
inline int digitalRead(uint8_t p) { return p < 80 ? sim_inputs[p] : 0; }
inline int analogRead(uint8_t) { return 0; }
inline void analogWrite(uint8_t p, int v) { if (p < 80) sim_pins[p] = v > 0; }
inline void attachInterrupt(int, void (*)(), int) {}
inline void detachInterrupt(int) {}
inline void tone(uint8_t p, unsigned int, unsigned long = 0) { digitalWrite(p, 1); }
inline void noTone(uint8_t p) { digitalWrite(p, 0); }
// random() identique au coeur AVR : random(max) = random() % max, random(a,b) = a + random(b-a)
extern "C" long sim_rand();
extern "C" void sim_srand(unsigned long);
inline void randomSeed(unsigned long s) { if (s != 0) sim_srand(s); }
inline long random(long howbig) { if (howbig == 0) return 0; return sim_rand() % howbig; }
inline long random(long howsmall, long howbig) { if (howsmall >= howbig) return howsmall; return random(howbig - howsmall) + howsmall; }
inline long map(long x, long in_min, long in_max, long out_min, long out_max) { return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min; }
struct SimSerial {
  void begin(long) {} void end() {} int available() { return 0; } int read() { return -1; } void flush() {}
  operator bool() { return true; }
  template<class T> size_t print(T) { return 0; }
  template<class T, class U> size_t print(T, U) { return 0; }
  size_t println() { return 0; }
  template<class T> size_t println(T) { return 0; }
  template<class T, class U> size_t println(T, U) { return 0; }
  size_t write(uint8_t) { return 0; }
};
static SimSerial Serial;
