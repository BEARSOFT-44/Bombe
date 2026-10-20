// Pont entre le sketch Arduino et la page web : horloge, broches, ecran (memoire du SSD1306), LEDs.
#include "Arduino.h"
#include "Adafruit_NeoPixel.h"
#include "u8x8.h"
#define EXPORT(nom) extern "C" __attribute__((export_name(#nom)))

extern "C" {
volatile uint32_t sim_ms = 0;
uint8_t sim_pins[80], sim_inputs[80], sim_pinmode[80];
uint32_t sim_leds[SIM_MAX_PIXELS]; uint16_t sim_nb_leds = 0; uint32_t sim_leds_shows = 0;
}
void setup();
void loop();

// ── Ecran : on remplace le pilote SSD1306 par un pilote qui ecrit dans une copie de la RAM de l'ecran ──
static uint8_t ecran_ram[1024];       // 8 pages de 128 octets, bit 0 = ligne du haut (format SSD1306)
static uint8_t ecran_eteint = 0, ecran_contraste = 255;
static uint32_t ecran_images = 0;
extern "C" uint8_t u8x8_d_ssd1306_128x64_noname_reel(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
extern "C" uint8_t u8x8_d_ssd1306_128x64_noname(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr) {
  switch (msg) {
    case U8X8_MSG_DISPLAY_DRAW_TILE: {
      u8x8_tile_t *t = (u8x8_tile_t *)arg_ptr;
      uint8_t x = t->x_pos;
      for (uint8_t r = 0; r < arg_int; r++)
        for (uint8_t c = 0; c < t->cnt; c++, x++)
          if (x < 16 && t->y_pos < 8) memcpy(&ecran_ram[t->y_pos * 128 + x * 8], t->tile_ptr + c * 8, 8);
      ecran_images++;
      return 1;
    }
    case U8X8_MSG_DISPLAY_SET_POWER_SAVE: ecran_eteint = arg_int; break;
    case U8X8_MSG_DISPLAY_SET_CONTRAST: ecran_contraste = arg_int; break;
  }
  return u8x8_d_ssd1306_128x64_noname_reel(u8x8, msg, arg_int, arg_ptr);
}
// Bus SPI/I2C : rien a transmettre, l'image est captee ci-dessus
extern "C" uint8_t u8x8_bus_vide(u8x8_t *, uint8_t, uint8_t, void *) { return 1; }
#define BUS(nom) extern "C" uint8_t nom(u8x8_t *a, uint8_t b, uint8_t c, void *d) { return u8x8_bus_vide(a, b, c, d); }
BUS(u8x8_byte_arduino_8bit_8080mode) BUS(u8x8_byte_arduino_4wire_sw_spi) BUS(u8x8_byte_arduino_3wire_sw_spi)
BUS(u8x8_byte_arduino_3wire_hw_spi) BUS(u8x8_byte_arduino_hw_spi) BUS(u8x8_byte_arduino_2nd_hw_spi)
BUS(u8x8_byte_arduino_sw_i2c) BUS(u8x8_byte_arduino_hw_i2c) BUS(u8x8_byte_arduino_2nd_hw_i2c) BUS(u8x8_byte_arduino_ks0108)
extern "C" uint8_t u8x8_gpio_and_delay_arduino(u8x8_t *, uint8_t msg, uint8_t arg_int, void *) {
  if (msg == U8X8_MSG_DELAY_MILLI) delay(arg_int);   // pauses d'initialisation de l'ecran, comme sur la carte
  return 1;
}

// ── Support C++ minimal ──
extern "C" void __cxa_pure_virtual() { __builtin_trap(); }
extern "C" int __cxa_atexit(void (*)(void *), void *, void *) { return 0; }
void *__dso_handle = 0;
extern unsigned char __heap_base;
static unsigned char *tas = &__heap_base;
void *operator new(size_t n) { unsigned char *p = tas; tas += (n + 7) & ~7u; size_t pages = __builtin_wasm_memory_size(0) * 65536u; if ((size_t)tas > pages) __builtin_wasm_memory_grow(0, ((size_t)tas - pages) / 65536 + 1); return p; }
void *operator new[](size_t n) { return operator new(n); }
void operator delete(void *) noexcept {}
void operator delete(void *, size_t) noexcept {}
void operator delete[](void *) noexcept {}
extern "C" void *malloc(size_t n) { return operator new(n); }
extern "C" void free(void *) {}

// ── Fonctions appelees par la page ──
extern "C" void __wasm_call_ctors();
EXPORT(sim_init) void sim_init(uint32_t graine) { for (int i = 0; i < 80; i++) sim_inputs[i] = 1; sim_ms = 0; __wasm_call_ctors(); sim_srand(graine); }
EXPORT(sim_setup) void sim_setup() { setup(); }
EXPORT(sim_loop) void sim_loop() { loop(); }
EXPORT(sim_temps) uint32_t sim_temps() { return sim_ms; }
EXPORT(sim_regler_temps) void sim_regler_temps(uint32_t ms) { sim_ms = ms; }
EXPORT(sim_entree) void sim_entree(int pin, int niveau) { if (pin >= 0 && pin < 80) sim_inputs[pin] = niveau; }
EXPORT(sim_adr_broches) uint8_t *sim_adr_broches() { return sim_pins; }
EXPORT(sim_adr_modes) uint8_t *sim_adr_modes() { return sim_pinmode; }
EXPORT(sim_adr_leds) uint32_t *sim_adr_leds() { return sim_leds; }
EXPORT(sim_nombre_leds) int sim_nombre_leds() { return sim_nb_leds; }
EXPORT(sim_adr_ecran) uint8_t *sim_adr_ecran() { return ecran_ram; }
EXPORT(sim_ecran_eteint) int sim_ecran_eteint() { return ecran_eteint; }
EXPORT(sim_ecran_images) uint32_t sim_ecran_images() { return ecran_images; }
