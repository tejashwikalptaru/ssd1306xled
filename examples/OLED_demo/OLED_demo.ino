#include <Arduino.h>
#include <font6x8.h>
#include <font8x16.h>
#include <ssd1306xled.h>
#include "image.h"
#include "image_two.h"

// // default I2C address used is 0X3C, change as per your need
// #ifdef SSD1306_SA
//   #undef SSD1306_SA
//   #define SSD1306_SA 0X3C
// #else
//   #define SSD1306_SA 0X3C
// #endif

// // default PIN configuration is as below
// // VCC ---- vcc
// // GND ---- gnd
// // SCL ---- pb2
// // SDA ---- pb0

// // change them if you need
// #ifdef SSD1306_SCL
//   #undef SSD1306_SCL
//   #define SSD1306_SCL PB2
// #else
//   #define SSD1306_SCL PB2
// #endif

// #ifdef SSD1306_SDA
//   #undef SSD1306_SDA
//   #define SSD1306_SDA PB0
// #else
//   #define SSD1306_SDA PB2
// #endif


void setup() {
   _delay_ms(40);
  SSD1306.ssd1306_init();
}

void loop() {
  uint8_t p = 0xff;
  for (uint8_t i = 0; i < 4; i++){
    p = (p >> i);
    SSD1306.ssd1306_fillscreen(~p);
    _delay_ms(100);
  }
  SSD1306.ssd1306_fillscreen(0);
  SSD1306.ssd1306_setpos(0, 1);
  SSD1306.ssd1306_string_font6x8("That's the");
  SSD1306.ssd1306_setpos(43, 3);
  SSD1306.ssd1306_string_font6x8("project");
  SSD1306.ssd1306_setpos(0, 3);
  SSD1306.ssd1306_string_font6x8("The platform that gives you everything you need for your first microcontroller project");
  SSD1306.ssd1306_setpos(12, 7);
  SSD1306.ssd1306_string_font6x8("http://tinusaur.org");
  _delay_ms(6000);
  SSD1306.ssd1306_fillscreen(0);
  SSD1306.ssd1306_draw_bmp(0, 0, 128, 8, logo);
  _delay_ms(4000);
  SSD1306.ssd1306_fillscreen(0);
  SSD1306.ssd1306_draw_bmp(0, 0, 128, 8, logo_two);
  _delay_ms(4000);

  // Compositing demo: two 8x8 sprites on adjacent rows sharing a page.
  // Sprite A at Y=20 touches pages 2-3, sprite B at Y=28 touches pages 3-4.
  // Without compositing, drawing one would clobber the other on page 3.
  // With compositing, both are merged into a buffer before a single write.
  static const uint8_t spriteA[] PROGMEM = {
    0xFF, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0xFF  // 8x8 hollow rectangle
  };
  static const uint8_t spriteB[] PROGMEM = {
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF  // 8x8 filled rectangle
  };

  SSD1306.ssd1306_fillscreen(0);
  SSD1306.ssd1306_setpos(10, 0);
  SSD1306.ssd1306_string_font6x8("Compositing");

  // Draw non-shared pages normally
  SSD1306.ssd1306_draw_bmp_px(20, 20, 8, 1, spriteA);  // page 2 portion
  SSD1306.ssd1306_draw_bmp_px(20, 28, 8, 1, spriteB);  // page 4 portion

  // Composite the shared page (page 3)
  uint8_t buf[8];
  memset(buf, 0, 8);
  SSD1306.ssd1306_compose_bmp_px(buf, 20, 8, 20, 20, 8, 1, spriteA, 3);
  SSD1306.ssd1306_compose_bmp_px(buf, 20, 8, 20, 28, 8, 1, spriteB, 3);
  SSD1306.ssd1306_send_buf(20, 3, buf, 8);

  _delay_ms(4000);
}