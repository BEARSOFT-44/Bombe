// Mini bibliotheque C pour WebAssembly (pas de libc disponible). sprintf suit le comportement avr-libc.
#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>
void *memset(void *d, int c, size_t n) { unsigned char *p = d; while (n--) *p++ = (unsigned char)c; return d; }
void *memcpy(void *d, const void *s, size_t n) { unsigned char *p = d; const unsigned char *q = s; while (n--) *p++ = *q++; return d; }
void *memmove(void *d, const void *s, size_t n) { unsigned char *p = d; const unsigned char *q = s; if (p < q) while (n--) *p++ = *q++; else { p += n; q += n; while (n--) *--p = *--q; } return d; }
int memcmp(const void *a, const void *b, size_t n) { const unsigned char *p = a, *q = b; for (; n; n--, p++, q++) if (*p != *q) return *p - *q; return 0; }
size_t strlen(const char *s) { size_t n = 0; while (s[n]) n++; return n; }
char *strcpy(char *d, const char *s) { char *r = d; while ((*d++ = *s++)); return r; }
char *strncpy(char *d, const char *s, size_t n) { size_t i = 0; for (; i < n && s[i]; i++) d[i] = s[i]; for (; i < n; i++) d[i] = 0; return d; }
char *strcat(char *d, const char *s) { strcpy(d + strlen(d), s); return d; }
int strcmp(const char *a, const char *b) { while (*a && *a == *b) a++, b++; return (unsigned char)*a - (unsigned char)*b; }
int strncmp(const char *a, const char *b, size_t n) { for (; n; n--, a++, b++) { if (*a != *b) return (unsigned char)*a - (unsigned char)*b; if (!*a) return 0; } return 0; }
char *strchr(const char *s, int c) { for (;; s++) { if (*s == (char)c) return (char *)s; if (!*s) return 0; } }
int atoi(const char *s) { int n = 0, neg = 0; while (*s == ' ') s++; if (*s == '-') neg = 1, s++; else if (*s == '+') s++; while (*s >= '0' && *s <= '9') n = n * 10 + (*s++ - '0'); return neg ? -n : n; }
long atol(const char *s) { return atoi(s); }

typedef struct { char *buf; size_t n, len; } out_t;
static void put(out_t *o, char c) { if (o->len + 1 < o->n) o->buf[o->len] = c; o->len++; }
static void emit(out_t *o, const char *s, int slen, int width, int left, char pad, int neg) {
  int total = slen + neg, i;
  if (!left && pad == '0') { if (neg) put(o, '-'); for (i = total; i < width; i++) put(o, '0'); }
  else if (!left) { for (i = total; i < width; i++) put(o, ' '); if (neg) put(o, '-'); }
  else if (neg) put(o, '-');
  for (i = 0; i < slen; i++) put(o, s[i]);
  if (left) for (i = total; i < width; i++) put(o, ' ');
}
static int utoa_(char *t, unsigned long v, int base, int upper) {
  char tmp[34]; int n = 0; const char *dg = upper ? "0123456789ABCDEF" : "0123456789abcdef";
  do { tmp[n++] = dg[v % base]; v /= base; } while (v);
  for (int i = 0; i < n; i++) t[i] = tmp[n - 1 - i];
  return n;
}
int vsnprintf(char *buf, size_t n, const char *f, va_list ap) {
  out_t o = { buf, n, 0 };
  for (; *f; f++) {
    if (*f != '%') { put(&o, *f); continue; }
    f++;
    int left = 0, width = 0, prec = -1, lng = 0; char pad = ' ';
    for (;; f++) { if (*f == '-') left = 1; else if (*f == '0') pad = '0'; else if (*f == '+' || *f == ' ' || *f == '#') {} else break; }
    if (*f == '*') { width = va_arg(ap, int); f++; } else while (*f >= '0' && *f <= '9') width = width * 10 + (*f++ - '0');
    if (*f == '.') { f++; prec = 0; while (*f >= '0' && *f <= '9') prec = prec * 10 + (*f++ - '0'); }
    while (*f == 'l' || *f == 'h') { if (*f == 'l') lng++; f++; }
    char t[40]; int len;
    switch (*f) {
      case 'd': case 'i': {
        long v = lng ? va_arg(ap, long) : (long)va_arg(ap, int);
        unsigned long u = v < 0 ? -(unsigned long)v : (unsigned long)v;
        len = utoa_(t, u, 10, 0); emit(&o, t, len, width, left, pad, v < 0); break; }
      case 'u': case 'x': case 'X': case 'o': {
        unsigned long u = lng ? va_arg(ap, unsigned long) : (unsigned long)va_arg(ap, unsigned int);
        len = utoa_(t, u, *f == 'u' ? 10 : (*f == 'o' ? 8 : 16), *f == 'X'); emit(&o, t, len, width, left, pad, 0); break; }
      case 'c': t[0] = (char)va_arg(ap, int); emit(&o, t, 1, width, left, ' ', 0); break;
      case 's': { const char *s = va_arg(ap, const char *); if (!s) s = "(null)"; len = (int)strlen(s); if (prec >= 0 && prec < len) len = prec; emit(&o, s, len, width, left, ' ', 0); break; }
      case 'f': (void)va_arg(ap, double); emit(&o, "?", 1, width, left, ' ', 0); break; // avr-libc : %f non supporte
      case '%': put(&o, '%'); break;
      default: put(&o, '%'); put(&o, *f); if (!*f) f--; break;
    }
  }
  if (n) buf[o.len < n ? o.len : n - 1] = 0;
  return (int)o.len;
}
int snprintf(char *buf, size_t n, const char *f, ...) { va_list ap; va_start(ap, f); int r = vsnprintf(buf, n, f, ap); va_end(ap); return r; }
int sprintf(char *buf, const char *f, ...) { va_list ap; va_start(ap, f); int r = vsnprintf(buf, (size_t)-1 >> 1, f, ap); va_end(ap); return r; }
char *ltoa(long v, char *buf, int base) { char *p = buf; unsigned long u = v; if (v < 0 && base == 10) { *p++ = '-'; u = -(unsigned long)v; } p[utoa_(p, u, base, 0)] = 0; return buf; }
char *itoa(int v, char *buf, int base) { return ltoa(v, buf, base); }
char *dtostrf(double v, signed char width, unsigned char prec, char *buf) {
  long m = 1; for (int i = 0; i < prec; i++) m *= 10;
  int neg = v < 0; if (neg) v = -v;
  unsigned long r = (unsigned long)(v * m + 0.5);
  char t[40]; int n = 0; if (neg) t[n++] = '-';
  n += utoa_(t + n, r / m, 10, 0);
  if (prec) { t[n++] = '.'; char f[20]; int k = utoa_(f, r % m, 10, 0); for (int i = k; i < prec; i++) t[n++] = '0'; for (int i = 0; i < k; i++) t[n++] = f[i]; }
  t[n] = 0; snprintf(buf, 40, "%*s", width, t); return buf;
}
// Generateur pseudo-aleatoire (algorithme de random() d'avr-libc, Park-Miller)
static unsigned long seed = 1;
long sim_rand(void) {
  long hi, lo, x = (long)seed;
  if (x == 0) x = 123459876L;
  hi = x / 127773L; lo = x % 127773L; x = 16807L * lo - 2836L * hi;
  if (x < 0) x += 0x7fffffffL;
  seed = (unsigned long)x;
  return x % ((unsigned long)0x7fffffff + 1);
}
void sim_srand(unsigned long s) { seed = s; }
