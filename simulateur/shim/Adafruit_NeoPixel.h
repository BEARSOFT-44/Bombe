// Faux bandeau NeoPixel : les couleurs envoyees par show() sont lues par la page web.
#pragma once
#include "Arduino.h"
#define NEO_GRB 0x52
#define NEO_RGB 0x06
#define NEO_RGBW 0x1B
#define NEO_GRBW 0xD2
#define NEO_KHZ800 0x0000
#define NEO_KHZ400 0x0100
#define SIM_MAX_PIXELS 64
extern "C" { extern uint32_t sim_leds[SIM_MAX_PIXELS]; extern uint16_t sim_nb_leds; extern uint32_t sim_leds_shows; }
class Adafruit_NeoPixel {
  uint16_t n; uint8_t bright = 0; uint32_t px[SIM_MAX_PIXELS] = {0};
public:
  Adafruit_NeoPixel(uint16_t num = 0, int16_t = 6, uint16_t = NEO_GRB + NEO_KHZ800) : n(num > SIM_MAX_PIXELS ? SIM_MAX_PIXELS : num) {}
  void begin() { sim_nb_leds = n; }
  void show() {
    sim_nb_leds = n; sim_leds_shows++;
    for (int i = 0; i < n; i++) {
      uint32_t c = px[i];
      if (bright) { uint8_t r = (c >> 16) & 255, g = (c >> 8) & 255, b = c & 255; r = (r * bright) >> 8; g = (g * bright) >> 8; b = (b * bright) >> 8; c = ((uint32_t)r << 16) | ((uint32_t)g << 8) | b; }
      sim_leds[i] = c;
    }
  }
  void clear() { for (int i = 0; i < n; i++) px[i] = 0; }
  void setPixelColor(uint16_t i, uint32_t c) { if (i < n) px[i] = c & 0xFFFFFF; }
  void setPixelColor(uint16_t i, uint8_t r, uint8_t g, uint8_t b) { setPixelColor(i, Color(r, g, b)); }
  void setPixelColor(uint16_t i, uint8_t r, uint8_t g, uint8_t b, uint8_t) { setPixelColor(i, Color(r, g, b)); }
  void fill(uint32_t c = 0, uint16_t first = 0, uint16_t count = 0) { uint16_t end = count ? first + count : n; for (uint16_t i = first; i < end && i < n; i++) px[i] = c; }
  uint32_t getPixelColor(uint16_t i) const { return i < n ? px[i] : 0; }
  void setBrightness(uint8_t b) { bright = b + 1; }
  uint8_t getBrightness() const { return bright - 1; }
  uint16_t numPixels() const { return n; }
  void updateLength(uint16_t num) { n = num > SIM_MAX_PIXELS ? SIM_MAX_PIXELS : num; }
  bool canShow() { return true; }
  static uint32_t Color(uint8_t r, uint8_t g, uint8_t b) { return ((uint32_t)r << 16) | ((uint32_t)g << 8) | b; }
  static uint32_t Color(uint8_t r, uint8_t g, uint8_t b, uint8_t) { return Color(r, g, b); }
  static uint32_t ColorHSV(uint16_t hue, uint8_t sat = 255, uint8_t val = 255) {
    hue = (hue * 1530L + 32768) / 65536; uint8_t r, g, b;
    if (hue < 510) { b = 0; if (hue < 255) { r = 255; g = hue; } else { r = 510 - hue; g = 255; } }
    else if (hue < 1020) { r = 0; if (hue < 765) { g = 255; b = hue - 510; } else { g = 1020 - hue; b = 255; } }
    else if (hue < 1530) { g = 0; if (hue < 1275) { r = hue - 1020; b = 255; } else { r = 255; b = 1530 - hue; } }
    else { r = 255; g = b = 0; }
    uint32_t v1 = 1 + val; uint16_t s1 = 1 + sat; uint8_t s2 = 255 - sat;
    return ((((((r * s1) >> 8) + s2) * v1) & 0xff00) << 8) | (((((g * s1) >> 8) + s2) * v1) & 0xff00) | (((((b * s1) >> 8) + s2) * v1) >> 8);
  }
};
