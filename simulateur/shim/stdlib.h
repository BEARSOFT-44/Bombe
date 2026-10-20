#pragma once
#include <stddef.h>
#ifdef __cplusplus
extern "C" {
#endif
int atoi(const char *s);
long atol(const char *s);
char *itoa(int v, char *buf, int base);
char *ltoa(long v, char *buf, int base);
char *dtostrf(double v, signed char width, unsigned char prec, char *buf);
#ifdef __cplusplus
}
#endif
#ifndef __cplusplus
#define abs(x) ((x)>0?(x):-(x))
#else
inline int abs(int x) { return x < 0 ? -x : x; }
inline long abs(long x) { return x < 0 ? -x : x; }
#endif
