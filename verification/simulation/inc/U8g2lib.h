#pragma once
#include "Arduino.h"
#include <string>
extern std::string g_screen;
struct U8G2_SSD1306_128X64_NONAME_F_4W_SW_SPI{ U8G2_SSD1306_128X64_NONAME_F_4W_SW_SPI(int,int,int,int,int,int){}
 void begin(){} void clearBuffer(){g_screen.clear();} void setFont(int){} int getStrWidth(const char*s){return 6*(int)std::string(s).size();}
 void drawStr(int x,int y,const char*s){g_screen+="["+std::to_string(x)+","+std::to_string(y)+"]"+s+" ";} void drawHLine(int,int,int){} void drawBox(int,int,int,int){} void drawFrame(int,int,int,int){} void sendBuffer(){} };
#define U8G2_R0 0
enum { u8g2_font_logisoso20_tr, u8g2_font_pressstart2p_8f, u8g2_font_logisoso32_tr, u8g2_font_6x12_tr, u8g2_font_logisoso16_tr, u8g2_font_6x10_tr, u8g2_font_7x14_tr };
