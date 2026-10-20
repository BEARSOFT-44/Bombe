#pragma once
#include <cstdint>
#define NEO_GRB 0
#define NEO_KHZ800 0
struct Adafruit_NeoPixel{ Adafruit_NeoPixel(int,int,int){} void begin(){} void show(){} void clear(){} void setPixelColor(int,uint32_t){} static uint32_t Color(uint8_t r,uint8_t g,uint8_t b){return (r<<16)|(g<<8)|b;} };
