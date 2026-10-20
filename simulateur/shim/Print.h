// Classe Print d'Arduino (version simplifiee mais au comportement identique pour l'affichage).
#pragma once
#include "Arduino.h"
#define DEC 10
#define HEX 16
#define OCT 8
#define BIN 2
class __FlashStringHelper;
class Print {
  size_t printNumber(unsigned long n, uint8_t base) {
    char buf[8 * sizeof(long) + 1]; char *s = &buf[sizeof(buf) - 1]; *s = 0;
    if (base < 2) base = 10;
    do { char c = n % base; n /= base; *--s = c < 10 ? c + '0' : c + 'A' - 10; } while (n);
    return write(s);
  }
public:
  virtual ~Print() {}
  virtual size_t write(uint8_t) = 0;
  size_t write(const char *s) { return s ? write((const uint8_t *)s, strlen(s)) : 0; }
  virtual size_t write(const uint8_t *b, size_t n) { size_t k = 0; while (n--) k += write(*b++); return k; }
  size_t write(const char *b, size_t n) { return write((const uint8_t *)b, n); }
  size_t print(const char s[]) { return write(s); }
  size_t print(const __FlashStringHelper *s) { return write((const char *)s); }
  size_t print(char c) { return write((uint8_t)c); }
  size_t print(unsigned char b, int base = DEC) { return print((unsigned long)b, base); }
  size_t print(int n, int base = DEC) { return print((long)n, base); }
  size_t print(unsigned int n, int base = DEC) { return print((unsigned long)n, base); }
  size_t print(long n, int base = DEC) {
    if (base == 0) return write((uint8_t)n);
    if (base == 10 && n < 0) { size_t t = print('-'); return printNumber(-(unsigned long)n, 10) + t; }
    return printNumber(n, base);
  }
  size_t print(unsigned long n, int base = DEC) { return base == 0 ? write((uint8_t)n) : printNumber(n, base); }
  size_t print(double x, int digits = 2) { char b[40]; dtostrf(x, 0, digits, b); return write(b); }
  size_t println() { return write("\r\n"); }
  template <class T> size_t println(T v) { size_t n = print(v); return n + println(); }
  template <class T> size_t println(T v, int b) { size_t n = print(v, b); return n + println(); }
};
